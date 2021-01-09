#include <ntddk.h>

#include "vmx.h"
#include "memory.h"
#include "asm_helpers.h"
#include "multiprocessing.h"
#include "logging.h"

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

void* vmx::vmxon()
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

bool vmx::vmlaunch()
{
	logging::dump_vmcs_guest_state_area();
	::__vmx_vmlaunch();
	return false; // not really
}

bool vmx::vmclear(unsigned long long* vmcs)
{
	auto ret = ::__vmx_vmclear(vmcs);
	return ret == STATUS_SUCCESS;
}

bool vmx::vmptrld(unsigned long long* vmcs)
{
	auto ret = ::__vmx_vmptrld(vmcs);
	return ret == STATUS_SUCCESS;
}

void vmx::adjust_vmx_controls(unsigned long& vmx_controls, intel::Ia32VmxControlsHint hint)
{
	vmx_controls |= hint.allowed_0_settings;
	vmx_controls &= hint.allowed_1_settings;
}

#pragma pack(push, 1)
struct GuestState
{
	unsigned long long rsp;
	unsigned long long rax;
	unsigned long long rcx;
	unsigned long long rdx;
	unsigned long long rbx;
	unsigned long long rbp;
	unsigned long long rsi;
	unsigned long long rdi;
	unsigned long long r8;
	unsigned long long r9;
	unsigned long long r10;
	unsigned long long r11;
	unsigned long long r12;
	unsigned long long r13;
	unsigned long long r14;
	unsigned long long r15;
};
#pragma pack(pop)

static unsigned long long select_register(GuestState* guest_state, unsigned long long register_number)
{
	switch (register_number)
	{
	case 0: return guest_state->rax;
	case 1: return guest_state->rcx;
	case 2: return guest_state->rdx;
	case 3: return guest_state->rbx;
	case 4: return guest_state->rsp;
	case 5: return guest_state->rbp;
	case 6: return guest_state->rsi;
	case 7: return guest_state->rdi;
	case 8: return guest_state->r8;
	case 9: return guest_state->r9;
	case 10: return guest_state->r10;
	case 11: return guest_state->r11;
	case 12: return guest_state->r12;
	case 13: return guest_state->r13;
	case 14: return guest_state->r14;
	case 15: return guest_state->r15;
	}

	return 0;
}

extern "C" void vmexit_handler(GuestState* guest_state)
{
	auto [success_1, exit_reason] = vmx::vmread<unsigned long>(intel::VmcsField::VMCS_EXIT_REASON);
	auto [success_2, exit_qualification] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_EXIT_QUALIFICATION);

	KdPrint(("[*] exit reason: %d\n", exit_reason & 0xffff));
	KdPrint(("[*] exit qualification: 0x%p\n", exit_qualification));

	auto [success_3, guest_rip] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP);
	auto [success_4, guest_rsp] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RSP);

	KdPrint(("[*] guest rip: 0x%p\n", guest_rip));
	KdPrint(("[*] guest rsp: 0x%p\n", guest_rsp));

	switch (exit_reason & 0xffff)
	{
	case 2: {
		::__debugbreak();
		break;
	}

	case 10: {
		intel::GeneralCpuidInfo cpuid_info = { 0 };

		::__cpuid(
			reinterpret_cast<int*>(&cpuid_info),
			static_cast<int>(guest_state->rax)
		);

		if ((guest_state->rax & 0xffffffff) == 0)
		{
			cpuid_info.ebx = 0x72617053;
			cpuid_info.edx = 0x70536174;
			cpuid_info.ecx = 0x61747261;
		}

		guest_state->rax = static_cast<unsigned long long>(cpuid_info.eax);
		guest_state->rbx = static_cast<unsigned long long>(cpuid_info.ebx);
		guest_state->rcx = static_cast<unsigned long long>(cpuid_info.ecx);
		guest_state->rdx = static_cast<unsigned long long>(cpuid_info.edx);

		break;
	}

	case 28: {
		intel::ControlRegisterAccessExitQualification cr_access_exit_qual = { exit_qualification };

		if (cr_access_exit_qual.cr_number == 3 && cr_access_exit_qual.access_type == 0)
		{
			vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CR3, select_register(guest_state, cr_access_exit_qual.mov_cr_gp_register));
			KdPrint(("[*] context switch!\n"));
		}

		break;
	}

	case 31: {
		auto msr = ::__readmsr(static_cast<unsigned long>(guest_state->rcx));

		guest_state->rax = msr & 0xffffffff;
		guest_state->rdx = msr >> 32;

		break;
	}

	case 32: {
		::__writemsr(
			static_cast<unsigned long>(guest_state->rcx),
			(guest_state->rax & 0xffffffff) | (guest_state->rdx << 32)
		);

		break;
	}

	case 33: {
		KdPrint(("[-] fuckkkk\n"));
		::KeBugCheck(0x0000013D);
	}
	}
	
	auto [success_5, instruction_length] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_VMEXIT_INSTRUCTION_LENGTH);

	vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RIP, guest_rip + instruction_length);
}
