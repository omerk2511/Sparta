#pragma warning(disable: 4100)

#include "kernel_integrity_module.h"
#include "nt_tricks.h"
#include "ept.h"
#include "vmx.h"
#include "logging.h"

static constexpr unsigned long long SMEP_BIT_MASK = 0x100000;

void KernelIntegrityModule::initialize(VcpuContext* vcpu_context) volatile
{
	// mark all pages as kernel NX
	/*for (auto i = 0; i < intel::EPT_ENTRY_COUNT; i++)
	{
		for (auto j = 0; j < intel::EPT_ENTRY_COUNT; j++)
		{
			vcpu_context->pd[i][j].supervisor_mode_execute = false;
		}
	}*/

	// loop over all drivers and mark as kernel X
	// inject #PFs to handle paged code...

	// register load driver notify routine
	// (later)

	// mark all sensitive pages as RO
	// (later)

	auto ssdt_base = nt_tricks::get_ssdt_base();
	auto ssdt_end = nt_tricks::get_ssdt_end();

	_ssdt_base_physical_address = ::MmGetPhysicalAddress(ssdt_base).QuadPart;
	_ssdt_end_physical_address = ::MmGetPhysicalAddress(ssdt_end).QuadPart;

	auto ssdt_base_pt = ept::split_large_page(&vcpu_context->pd[(_ssdt_base_physical_address >> 21) / intel::EPT_ENTRY_COUNT][(_ssdt_base_physical_address >> 21) % intel::EPT_ENTRY_COUNT]);
	auto ssdt_base_pte = &ssdt_base_pt[(_ssdt_base_physical_address >> 12) % intel::EPT_ENTRY_COUNT];

	ssdt_base_pte->write = false;

	auto ssdt_end_pt = ept::split_large_page(&vcpu_context->pd[(_ssdt_end_physical_address >> 21) / intel::EPT_ENTRY_COUNT][(_ssdt_end_physical_address >> 21) % intel::EPT_ENTRY_COUNT]);
	auto ssdt_end_pte = &ssdt_end_pt[(_ssdt_end_physical_address >> 12) % intel::EPT_ENTRY_COUNT];

	ssdt_end_pte->write = false;

	vmx::vmwrite(intel::VmcsField::VMCS_CTRL_CR4_GUEST_HOST_MASK, SMEP_BIT_MASK);
}

void KernelIntegrityModule::handle_ept_violation(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile
{
	auto guest_physical_address = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PHYSICAL_ADDRESS).value;
	if (guest_physical_address < _ssdt_base_physical_address || guest_physical_address >= _ssdt_end_physical_address)
	{
		return;
	}

	auto guest_rip = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP).value;
	logging::log_to_usermode(logging::ThreatType::KERNEL_STRUCTURE_CORRUPTION, guest_rip);

	increment_rip = true; // BSOD instead of just blocking write?
}

void KernelIntegrityModule::handle_cr_access(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile
{
	auto guest_rip = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP).value;
	logging::log_to_usermode(logging::ThreatType::SMEP_BYPASS, guest_rip);

	increment_rip = true;
}
