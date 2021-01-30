#include "vmx.h"

#include "memory.h"
#include "asm_helpers.h"
#include "multiprocessing.h"
#include "module_manager.h"

void vmx::enable_vmx()
{
	intel::Cr4 cr4 = { ::__readcr4() };
	cr4.vmxe = true;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully set the vmx enabled bit of cr4\n"));

	intel::Ia32FeatureControl feature_control = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FEATURE_CONTROL)) };

	if (!feature_control.lock)
	{
		feature_control.lock = true;
		feature_control.enable_smx = true;
		feature_control.enable_vmxon = true;

		::__writemsr(
			static_cast<unsigned long>(intel::Msr::IA32_FEATURE_CONTROL),
			feature_control.raw
		);

		KdPrint(("[+] successfully enabled vmxon in ia32_feature_control msr\n"));
	}
	else
	{
		KdPrint(("[+] vmxon is already enabled in ia32_feature_control msr\n"));
	}

	auto ia32_vmx_cr0_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR0_FIXED0)
	);

	auto ia32_vmx_cr0_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR0_FIXED1)
	);

	auto cr0 = ::__readcr0();
	cr0 |= ia32_vmx_cr0_fixed0;
	cr0 &= ia32_vmx_cr0_fixed1;
	::__writecr0(cr0);

	auto ia32_vmx_cr4_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR4_FIXED0)
	);

	auto ia32_vmx_cr4_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR4_FIXED1)
	);

	cr4.raw = ::__readcr4();
	cr4.raw |= ia32_vmx_cr4_fixed0;
	cr4.raw &= ia32_vmx_cr4_fixed1;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully updated cr0 and cr4 based on the required bits\n"));
}

auto vmx::vmxon() -> void*
{
	auto virtual_vmxon_region = reinterpret_cast<void*>(new (NonPagedPool) unsigned char[intel::VMXON_REGION_SIZE]);

	if (!virtual_vmxon_region)
	{
		KdPrint(("[-] could not allocate a vmxon region\n"));
		return nullptr;
	}

	::RtlSecureZeroMemory(virtual_vmxon_region, intel::VMXON_REGION_SIZE);

	auto physical_vmxon_region = ::MmGetPhysicalAddress(virtual_vmxon_region).QuadPart;

	KdPrint(("[+] vmxon region allocated @0x%p (virtual) -> @0x%p (physical)\n", virtual_vmxon_region, physical_vmxon_region));

	intel::Ia32VmxBasic ia32_vmx_basic = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_BASIC)) };
	*reinterpret_cast<unsigned long*>(virtual_vmxon_region) = static_cast<unsigned long>(ia32_vmx_basic.revision_identifier);

	auto ret = ::__vmx_on(reinterpret_cast<unsigned long long*>(&physical_vmxon_region));

	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmxon_region;
		KdPrint(("[-] vmxon failed\n"));

		return nullptr;
	}

	KdPrint(("[+] vmxon succeeded\n"));

	return virtual_vmxon_region;
}

auto vmx::vmlaunch() -> bool
{
	::__vmx_vmlaunch();
	return false; // not really
}

auto vmx::vmclear(unsigned long long* vmcs) -> bool
{
	auto ret = ::__vmx_vmclear(vmcs);
	return ret == STATUS_SUCCESS;
}

auto vmx::vmptrld(unsigned long long* vmcs) -> bool
{
	auto ret = ::__vmx_vmptrld(vmcs);
	return ret == STATUS_SUCCESS;
}

void vmx::adjust_vmx_controls(unsigned long& vmx_controls, intel::Ia32VmxControlsHint hint)
{
	vmx_controls |= hint.allowed_0_settings;
	vmx_controls &= hint.allowed_1_settings;
}

void vmx::inject_nmi()
{
	intel::VmEntryInterruptionInformation vm_entry_interruption_information = { 0 };

	vm_entry_interruption_information.vector = 2; // NMI_VECTOR;
	vm_entry_interruption_information.type = 2; // intel::VmxInterruptionType::NMI
	vm_entry_interruption_information.valid = true;

	vmx::vmwrite(intel::VmcsField::VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, vm_entry_interruption_information.raw);
}

extern "C" void vmexit_handler(sparta::VmExitGuestState* guest_state)
{
	auto vcpu_context = reinterpret_cast<VcpuContext*>(reinterpret_cast<unsigned long long>(guest_state) & 0xfffffffffffff000);
	auto exit_reason = static_cast<intel::VmExitReason>(vmx::vmread<unsigned long>(intel::VmcsField::VMCS_EXIT_REASON).value);

	bool increment_rip = true;

	sparta::call_modules(exit_reason, vcpu_context, guest_state, increment_rip);
	
	if (increment_rip)
	{
		auto guest_rip = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP).value;
		auto instruction_length = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_VMEXIT_INSTRUCTION_LENGTH).value;

		vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RIP, guest_rip + instruction_length);
	}
}
