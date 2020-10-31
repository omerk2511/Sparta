#include <ntddk.h>

#include "vmx.h"
#include "memory.h"
#include "processor_context.h"
#include "asm_helpers.h"

extern ProcessorContext* g_processors_context;

namespace vmx
{
	void enable_vmx_operation();
	void* vmxon();
	void* setup_vmcs(unsigned int processor_index, void* saved_rsp);
	void vmlaunch();

	static inline unsigned long long get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base);
	static inline void adjust_vmx_controls(unsigned long& vmx_controls, intel::Ia32VmxControlsHint hint);
	static inline intel::VmxSegmentAccessRights get_segment_access_rights(unsigned long selector);
}

bool vmx::initialize_vmx(unsigned int processor_index)
{
	auto saved_rsp = reinterpret_cast<void*>(reinterpret_cast<unsigned long long>(asm_helpers::get_rsp()) + 0x38);

	KdPrint(("[*] initializing vmx in processor %d\n", processor_index));

	enable_vmx_operation();
	KdPrint(("[+] enabled vmx operation successfully\n"));

	auto vmxon_region = vmxon();

	if (!vmxon_region)
	{
		KdPrint(("[-] failed initializing vmx in processor %d\n", processor_index));
		return false;
	}

	g_processors_context[processor_index].vmxon_region = vmxon_region;
	KdPrint(("[+] entered vmx root mode successfully\n", processor_index));

	g_processors_context[processor_index].vmcs_region = setup_vmcs(processor_index, saved_rsp);
	KdPrint(("[+] successfully initialized the vmcs\n"));

	vmlaunch();

	KdPrint(("[+] successfully initialized vmx in processor %d\n", processor_index));

	return true;
}

void vmx::enable_vmx_operation()
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

static constexpr unsigned short ZERO_TI_RPL_MASK = 0xf8;
static constexpr unsigned long long VMCS_LINK_POINTER_NOT_USED = MAXULONG64;

static inline unsigned long long vmx::get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base)
{
	if (!segment_selector)
		return 0;

	auto descriptor = reinterpret_cast<intel::SystemSegmentDescriptor*>(
		gdt_base + (segment_selector >> 3) * sizeof(intel::SegmentDescriptor));
	return descriptor->base_0 | (descriptor->base_0 << 24);
}

static inline void vmx::adjust_vmx_controls(unsigned long& vmx_controls, intel::Ia32VmxControlsHint hint)
{
	vmx_controls |= hint.allowed_0_settings;
	vmx_controls &= hint.allowed_1_settings;
}

void* vmx::setup_vmcs(unsigned int processor_index, void* saved_rsp)
{
	auto virtual_vmcs_region = new (NonPagedPool) intel::Vmcs;

	if (!virtual_vmcs_region)
	{
		KdPrint(("[-] could not allocate a vmcs region\n"));
		return nullptr;
	}

	::RtlSecureZeroMemory(virtual_vmcs_region, intel::VMXON_REGION_SIZE);

	intel::Ia32VmxBasic ia32_vmx_basic = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_BASIC)) };
	virtual_vmcs_region->revision_identifier = static_cast<unsigned long>(ia32_vmx_basic.revision_identifier);

	auto physical_vmcs_region = ::MmGetPhysicalAddress(virtual_vmcs_region).QuadPart;
	KdPrint(("[+] vmcs region allocated @0x%p (virtual) -> @0x%p (physical)\n", virtual_vmcs_region, physical_vmcs_region));

	auto ret = ::__vmx_vmclear(reinterpret_cast<unsigned long long*>(&physical_vmcs_region));

	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmclear failed\n"));

		return nullptr;
	}

	ret = ::__vmx_vmptrld(reinterpret_cast<unsigned long long*>(&physical_vmcs_region));

	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmptrld failed\n"));

		return nullptr;
	}

	auto success = true;

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, static_cast<unsigned short>(processor_index + 1));
	
	auto segment_selectors = asm_helpers::get_segment_selectors();

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_ES_SELECTOR, segment_selectors.es);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CS_SELECTOR, segment_selectors.cs);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SS_SELECTOR, segment_selectors.ss);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_DS_SELECTOR, segment_selectors.ds);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_FS_SELECTOR, segment_selectors.fs);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GS_SELECTOR, segment_selectors.gs);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_SELECTOR, segment_selectors.ldtr);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_TR_SELECTOR, segment_selectors.tr);

	success &= vmwrite(intel::VmcsField::VMCS_HOST_ES_SELECTOR, static_cast<unsigned short>(segment_selectors.es & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_CS_SELECTOR, static_cast<unsigned short>(segment_selectors.cs & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_SS_SELECTOR, static_cast<unsigned short>(segment_selectors.ss & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_DS_SELECTOR, static_cast<unsigned short>(segment_selectors.ds & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_FS_SELECTOR, static_cast<unsigned short>(segment_selectors.fs & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_GS_SELECTOR, static_cast<unsigned short>(segment_selectors.gs & ZERO_TI_RPL_MASK));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_TR_SELECTOR, static_cast<unsigned short>(segment_selectors.tr & ZERO_TI_RPL_MASK));

	// success &= vmwrite(intel::VmcsField::VMCS_CTRL_EPT_POINTER, 0ull); // use a real ept

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_VMCS_LINK_POINTER, VMCS_LINK_POINTER_NOT_USED);

	intel::PinBasedVmxControls pin_based_vmx_controls = { 0 };

	adjust_vmx_controls(
		pin_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint {
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_PINBASED_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PINBASED_CTLS))
		}
	);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pin_based_vmx_controls.raw);

	intel::PrimaryProcessorBasedVmxControls primary_processor_based_vmx_controls = { 0 };

	primary_processor_based_vmx_controls.hlt_exiting = 1;
	primary_processor_based_vmx_controls.cr3_load_exiting = 1;
	primary_processor_based_vmx_controls.activate_secondary_controls = 1;

	adjust_vmx_controls(
		primary_processor_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint {
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_PROCBASED_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PROCBASED_CTLS))
		}
	);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, primary_processor_based_vmx_controls.raw);

	intel::VmExitControls vm_exit_controls = { 0 };

	vm_exit_controls.host_address_space_size = 1;

	adjust_vmx_controls(
		vm_exit_controls.raw,
		intel::Ia32VmxControlsHint {
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_EXIT_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_EXIT_CTLS))
		}
	);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_VMEXIT_CONTROLS, vm_exit_controls.raw);

	intel::VmEntryControls vm_entry_controls = { 0 };

	vm_entry_controls.ia32e_mode_guest = 1;

	adjust_vmx_controls(
		vm_entry_controls.raw,
		intel::Ia32VmxControlsHint {
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_ENTRY_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_ENTRY_CTLS))
		}
	);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_VMENTRY_CONTROLS, vm_entry_controls.raw);

	intel::SecondaryProcessorBasedVmxControls secondary_processor_based_vmx_controls = { 0 };

	secondary_processor_based_vmx_controls.enable_ept = 0;
	secondary_processor_based_vmx_controls.enable_vpid = 1;
	
	adjust_vmx_controls(
		secondary_processor_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PROCBASED_CTLS2)) }
	);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, secondary_processor_based_vmx_controls.raw);

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_ES_LIMIT, ::__segmentlimit(segment_selectors.es));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CS_LIMIT, ::__segmentlimit(segment_selectors.cs));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SS_LIMIT, ::__segmentlimit(segment_selectors.ss));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_DS_LIMIT, ::__segmentlimit(segment_selectors.ds));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_FS_LIMIT, ::__segmentlimit(segment_selectors.fs));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GS_LIMIT, ::__segmentlimit(segment_selectors.gs));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_LIMIT, ::__segmentlimit(segment_selectors.ldtr));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_TR_LIMIT, ::__segmentlimit(segment_selectors.tr));

	intel::Gdtr gdtr = { 0 };
	::_sgdt(&gdtr);

	intel::Idtr idtr = { 0 };
	::__sidt(&idtr);

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GDTR_LIMIT, static_cast<unsigned long>(gdtr.limit));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_IDTR_LIMIT, static_cast<unsigned long>(idtr.limit));

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_ES_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.es).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CS_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.cs).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SS_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.ss).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_DS_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.ds).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_FS_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.fs).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GS_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.gs).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.ldtr).raw);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_TR_ACCESS_RIGHTS, get_segment_access_rights(segment_selectors.tr).raw);
	
	success &= vmwrite(
		intel::VmcsField::VMCS_GUEST_SYSENTER_CS,
		static_cast<unsigned long>(::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_CS)))
	);

	success &= vmwrite(
		intel::VmcsField::VMCS_SYSENTER_CS,
		static_cast<unsigned long>(::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_CS)))
	);

	intel::Cr0 cr0_mask = { 0 };
	intel::Cr0 cr0_shadow = { ::__readcr0() };

	intel::Cr4 cr4_mask = { 0 };
	intel::Cr4 cr4_shadow = { ::__readcr4() };

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_CR0_GUEST_HOST_MASK, cr0_mask.raw);	
	success &= vmwrite(intel::VmcsField::VMCS_CTRL_CR0_READ_SHADOW, cr0_shadow.raw);

	success &= vmwrite(intel::VmcsField::VMCS_CTRL_CR4_GUEST_HOST_MASK, cr4_mask.raw);
	success &= vmwrite(intel::VmcsField::VMCS_CTRL_CR4_READ_SHADOW, cr4_shadow.raw);

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CR0, ::__readcr0());
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CR3, ::__readcr3());
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CR4, ::__readcr4());

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_ES_BASE, 0ull);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_CS_BASE, 0ull);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SS_BASE, 0ull);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_DS_BASE, 0ull);

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_FS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FS_BASE)));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE)));

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_BASE, get_system_segment_base(segment_selectors.ldtr, gdtr.base));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_TR_BASE, get_system_segment_base(segment_selectors.tr, gdtr.base));

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_GDTR_BASE, gdtr.base);
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_IDTR_BASE, idtr.base);

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_DR7, ::__readdr(7));

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_RSP, reinterpret_cast<unsigned long long>(saved_rsp));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_RIP, reinterpret_cast<unsigned long long>(_end_initialize_vmx));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_RFLAGS, ::__readeflags());

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SYSENTER_ESP, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_ESP)));
	success &= vmwrite(intel::VmcsField::VMCS_GUEST_SYSENTER_EIP, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_EIP)));

	success &= vmwrite(intel::VmcsField::VMCS_HOST_CR0, ::__readcr0());
	success &= vmwrite(intel::VmcsField::VMCS_HOST_CR3, ::__readcr3());
	success &= vmwrite(intel::VmcsField::VMCS_HOST_CR4, ::__readcr4());

	success &= vmwrite(intel::VmcsField::VMCS_HOST_FS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FS_BASE)));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_GS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE)));

	success &= vmwrite(intel::VmcsField::VMCS_GUEST_TR_BASE, get_system_segment_base(segment_selectors.tr, gdtr.base));

	success &= vmwrite(intel::VmcsField::VMCS_HOST_GDTR_BASE, gdtr.base);
	success &= vmwrite(intel::VmcsField::VMCS_HOST_IDTR_BASE, idtr.base);

	success &= vmwrite(intel::VmcsField::VMCS_HOST_SYSENTER_ESP, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_ESP)));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_SYSENTER_EIP, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_SYSENTER_EIP)));

	auto stack_buffer = new (NonPagedPool) void*[0x1000];
	::RtlSecureZeroMemory(stack_buffer, 0x8000);

	success &= vmwrite(intel::VmcsField::VMCS_HOST_RSP, reinterpret_cast<unsigned long long>(stack_buffer + 0x1000 - 1));
	success &= vmwrite(intel::VmcsField::VMCS_HOST_RIP, reinterpret_cast<unsigned long long>(_vmexit_handler));

	return virtual_vmcs_region;
}

void vmx::vmlaunch()
{
	::__vmx_vmlaunch();
}

static inline intel::VmxSegmentAccessRights vmx::get_segment_access_rights(unsigned long selector)
{
	intel::VmxSegmentAccessRights access_rights = { 0 };

	if (selector)
	{
		access_rights.raw = asm_helpers::get_segment_access_rights(selector).raw >> 8;
	}
	else
	{
		access_rights.segment_unusable = 1;
	}

	return access_rights;
}

#pragma pack(push, 1)
struct GuestState
{
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

extern "C" void vmexit_handler(GuestState* guest_state)
{
	auto [success_1, exit_reason] = vmx::vmread<unsigned long>(intel::VmcsField::VMCS_EXIT_REASON);
	auto [success_2, exit_qualification] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_EXIT_QUALIFICATION);

	KdPrint(("[*] exit reason: %d\n", exit_reason & 0xffff));
	KdPrint(("[*] exit qualification: 0x%p\n", exit_qualification));

	switch (exit_reason & 0xffff)
	{
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
	}

	auto [success_3, guest_rip] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP);
	auto [success_4, guest_rsp] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RSP);

	KdPrint(("[*] guest rip: 0x%p\n", guest_rip));
	KdPrint(("[*] guest rsp: 0x%p\n", guest_rsp));
	
	auto [success_5, instruction_length] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_VMEXIT_INSTRUCTION_LENGTH);

	vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RIP, guest_rip + instruction_length);
}
