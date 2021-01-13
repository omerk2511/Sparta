#include "loader.h"
#include "multiprocessing.h"
#include "vmx.h"
#include "asm_helpers.h"
#include "logging.h"

static auto allocate_vcpu_context() -> loader::VcpuContext*;

static void setup_ept(loader::VcpuContext* vcpu_context);
static void setup_vmcs(loader::VcpuContext* vcpu_context, unsigned long long host_cr3);

static auto is_hypervisor_present() -> bool
{
	intel::GeneralCpuidInfo cpuid_info = { 0 };
	::__cpuid(reinterpret_cast<int*>(&cpuid_info), 0);
	return (cpuid_info.ebx == 0x72617053);
}

auto loader::load_sparta(SpartaContext* sparta_context) -> bool
{
	auto processor_index = static_cast<unsigned long>(multiprocessing::get_current_processor_id());
	KdPrint(("[*] loading sparta on processor %ul\n", processor_index));

	auto vcpu_context = allocate_vcpu_context();

	if (vcpu_context == nullptr)
	{
		KdPrint(("[-] could not allocate a vcpu context in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully allocated a vcpu context\n"));

	vcpu_context->processor_index = processor_index;

	vmx::enable_vmx();
	KdPrint(("[+] enabled vmx operation successfully\n"));

	vcpu_context->vmxon_region = vmx::vmxon();

	if (!vcpu_context->vmxon_region)
	{
		KdPrint(("[-] failed initializing vmx in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] entered vmx root mode successfully in processor %ul\n", processor_index));

	setup_ept(vcpu_context);
	setup_vmcs(vcpu_context, sparta_context->host_cr3);

	if (!vcpu_context->vmcs_region)
	{
		KdPrint(("[-] failed initializing the vmcs in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized the vmcs in processor %ul\n", processor_index));

	auto success = true;
	::RtlCaptureContext(&vcpu_context->guest_context);

	KIRQL old_irql;
	KeRaiseIrql(2, &old_irql);
	if (!is_hypervisor_present())
	{
		KdPrint(("[*] vmlaunch in host\n"));
		success = vmx::vmlaunch();
	}
	else
	{
		KdPrint(("[*] in guest :)\n"));
	}
	::KeLowerIrql(old_irql);

	if (!success)
	{
		KdPrint(("[-] failed initializing vmx in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized vmx in processor %ul\n", processor_index));

	// log some MSRs to check syscall stuff
	logging::dump_syscall_check();

	return true;
}

constexpr unsigned short ZERO_RPL_MASK = 0xfc;
constexpr unsigned long long VMCS_LINK_POINTER_NOT_USED = ~0ULL;

auto allocate_vcpu_context() -> loader::VcpuContext*
{
	auto vcpu_context = new (NonPagedPool) loader::VcpuContext;
	if (!vcpu_context)
	{
		return nullptr;
	}

	::RtlSecureZeroMemory(vcpu_context, sizeof(loader::VcpuContext));

	return vcpu_context;
}

void setup_ept(loader::VcpuContext* vcpu_context)
{
	vcpu_context->pml4[0].read = true;
	vcpu_context->pml4[0].write = true;
	vcpu_context->pml4[0].supervisor_mode_execute = true;
	vcpu_context->pml4[0].user_mode_execute = true;
	vcpu_context->pml4[0].pfn = ::MmGetPhysicalAddress(&vcpu_context->pdpt).QuadPart >> 12;

	for (auto i = 0; i < intel::EPT_ENTRY_COUNT; i++)
	{
		vcpu_context->pdpt[i].read = true;
		vcpu_context->pdpt[i].write = true;
		vcpu_context->pdpt[i].supervisor_mode_execute = true;
		vcpu_context->pdpt[i].user_mode_execute = true;
		vcpu_context->pdpt[i].pfn = ::MmGetPhysicalAddress(&vcpu_context->pd[i]).QuadPart >> 12;
	}

	for (auto i = 0; i < intel::EPT_ENTRY_COUNT; i++)
	{
		for (auto j = 0; j < intel::EPT_ENTRY_COUNT; j++)
		{
			vcpu_context->pd[i][j].read = true;
			vcpu_context->pd[i][j].write = true;
			vcpu_context->pd[i][j].supervisor_mode_execute = true;
			vcpu_context->pd[i][j].user_mode_execute = true;
			vcpu_context->pd[i][j].must_be_1 = 1;
			vcpu_context->pd[i][j].type = static_cast<unsigned long long>(intel::MtrrType::WB); // fix - poll mtrr msrs
			vcpu_context->pd[i][j].pfn = i * intel::EPT_ENTRY_COUNT + j;
		}
	}
}

static void setup_vmcs(loader::VcpuContext* vcpu_context, unsigned long long host_cr3)
{
	auto virtual_vmcs_region = new (NonPagedPool) intel::Vmcs;

	if (!virtual_vmcs_region)
	{
		KdPrint(("[-] could not allocate a vmcs region\n"));
		return;
	}

	::RtlSecureZeroMemory(virtual_vmcs_region, intel::VMXON_REGION_SIZE);

	intel::Ia32VmxBasic ia32_vmx_basic = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_BASIC)) };
	virtual_vmcs_region->revision_identifier = static_cast<unsigned long>(ia32_vmx_basic.revision_identifier);

	auto physical_vmcs_region = ::MmGetPhysicalAddress(virtual_vmcs_region).QuadPart;
	KdPrint(("[+] vmcs region allocated @0x%p (virtual) -> @0x%p (physical)\n", virtual_vmcs_region, physical_vmcs_region));

	auto success = vmx::vmclear(reinterpret_cast<unsigned long long*>(&physical_vmcs_region));

	if (!success)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmclear failed\n"));

		return;
	}

	success = vmx::vmptrld(reinterpret_cast<unsigned long long*>(&physical_vmcs_region));

	if (!success)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmptrld failed\n"));

		return;
	}

	auto segment_selectors = asm_helpers::get_segment_selectors();

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CS_SELECTOR, segment_selectors.cs);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_SS_SELECTOR, segment_selectors.ss);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DS_SELECTOR, segment_selectors.ds);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_FS_SELECTOR, segment_selectors.fs);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GS_SELECTOR, segment_selectors.gs);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_SELECTOR, segment_selectors.ldtr);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_ES_SELECTOR, segment_selectors.es);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_TR_SELECTOR, segment_selectors.tr);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_ES_SELECTOR, static_cast<unsigned short>(segment_selectors.es & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_CS_SELECTOR, static_cast<unsigned short>(segment_selectors.cs & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_SS_SELECTOR, static_cast<unsigned short>(segment_selectors.ss & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_DS_SELECTOR, static_cast<unsigned short>(segment_selectors.ds & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_FS_SELECTOR, static_cast<unsigned short>(segment_selectors.fs & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_GS_SELECTOR, static_cast<unsigned short>(segment_selectors.gs & ZERO_RPL_MASK));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_TR_SELECTOR, static_cast<unsigned short>(segment_selectors.tr & ZERO_RPL_MASK));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1ull); // use a real VPID?

	intel::VmxEptp ept_pointer = { 0 };

	ept_pointer.type = static_cast<unsigned long long>(intel::MtrrType::WB);
	ept_pointer.page_walk_length = 3;
	ept_pointer.pfn = ::MmGetPhysicalAddress(&vcpu_context->pml4).QuadPart >> 12;

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_EPT_POINTER, ept_pointer.raw);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_VMCS_LINK_POINTER, VMCS_LINK_POINTER_NOT_USED);

	intel::PinBasedVmxControls pin_based_vmx_controls = { 0 };

	vmx::adjust_vmx_controls(
		pin_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint{
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_PINBASED_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PINBASED_CTLS))
		}
	);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pin_based_vmx_controls.raw);

	intel::PrimaryProcessorBasedVmxControls primary_processor_based_vmx_controls = { 0 };

	primary_processor_based_vmx_controls.cr3_load_exiting = true;
	primary_processor_based_vmx_controls.activate_secondary_controls = true;

	vmx::adjust_vmx_controls(
		primary_processor_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint{
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_PROCBASED_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PROCBASED_CTLS))
		}
	);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, primary_processor_based_vmx_controls.raw);

	intel::VmExitControls vm_exit_controls = { 0 };

	vm_exit_controls.host_address_space_size = true;

	vmx::adjust_vmx_controls(
		vm_exit_controls.raw,
		intel::Ia32VmxControlsHint{
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_EXIT_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_EXIT_CTLS))
		}
	);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_VMEXIT_CONTROLS, vm_exit_controls.raw);

	intel::VmEntryControls vm_entry_controls = { 0 };

	vm_entry_controls.ia32e_mode_guest = true;

	vmx::adjust_vmx_controls(
		vm_entry_controls.raw,
		intel::Ia32VmxControlsHint{
			ia32_vmx_basic.vmx_capability_hint
			? ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_TRUE_ENTRY_CTLS))
			: ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_ENTRY_CTLS))
		}
	);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_VMENTRY_CONTROLS, vm_entry_controls.raw);

	intel::SecondaryProcessorBasedVmxControls secondary_processor_based_vmx_controls = { 0 };

	secondary_processor_based_vmx_controls.enable_ept = true;
	secondary_processor_based_vmx_controls.enable_vpid = true;
	secondary_processor_based_vmx_controls.mode_based_execute_control_for_ept = true;
	secondary_processor_based_vmx_controls.enable_rdtscp = true;
	secondary_processor_based_vmx_controls.enable_invpcid = true;
	secondary_processor_based_vmx_controls.enable_xsaves_xrstors = true;

	vmx::adjust_vmx_controls(
		secondary_processor_based_vmx_controls.raw,
		intel::Ia32VmxControlsHint{ ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_PROCBASED_CTLS2)) }
	);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, secondary_processor_based_vmx_controls.raw);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_ES_LIMIT, ::__segmentlimit(segment_selectors.es));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CS_LIMIT, ::__segmentlimit(segment_selectors.cs));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_SS_LIMIT, ::__segmentlimit(segment_selectors.ss));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DS_LIMIT, ::__segmentlimit(segment_selectors.ds));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_FS_LIMIT, ::__segmentlimit(segment_selectors.fs));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GS_LIMIT, ::__segmentlimit(segment_selectors.gs));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_LIMIT, ::__segmentlimit(segment_selectors.ldtr));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_TR_LIMIT, ::__segmentlimit(segment_selectors.tr));

	intel::Gdtr gdtr = { 0 };
	::_sgdt(&gdtr);

	intel::Idtr idtr = { 0 };
	::__sidt(&idtr);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GDTR_LIMIT, static_cast<unsigned long>(gdtr.limit));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_IDTR_LIMIT, static_cast<unsigned long>(idtr.limit));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_ES_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.es).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CS_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.cs).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_SS_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.ss).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DS_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.ds).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_FS_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.fs).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GS_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.gs).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.ldtr).raw);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_TR_ACCESS_RIGHTS, intel::get_segment_access_rights(segment_selectors.tr).raw);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CR0, ::__readcr0());
	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_CR0_READ_SHADOW, ::__readcr0());

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CR3, ::__readcr3());

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CR4, ::__readcr4());
	success &= vmx::vmwrite(intel::VmcsField::VMCS_CTRL_CR4_READ_SHADOW, ::__readcr4());

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_ES_BASE, 0ull);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_CS_BASE, 0ull);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_SS_BASE, 0ull);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DS_BASE, 0ull);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_FS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FS_BASE)));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE)));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_BASE, intel::get_system_segment_base(segment_selectors.ldtr, gdtr.base));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_TR_BASE, intel::get_system_segment_base(segment_selectors.tr, gdtr.base));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_GDTR_BASE, gdtr.base);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_IDTR_BASE, idtr.base);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DEBUGCTL, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_DEBUGCTL)));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_DR7, ::__readdr(7));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_CR0, ::__readcr0());
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_CR3, host_cr3);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_CR4, ::__readcr4());

	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_FS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FS_BASE)));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_GS_BASE, ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE)));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_TR_BASE, intel::get_system_segment_base(segment_selectors.tr, gdtr.base));

	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_GDTR_BASE, gdtr.base);
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_IDTR_BASE, idtr.base);

	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RSP, reinterpret_cast<unsigned long long>(vcpu_context->stack + loader::STACK_LIMIT - 8));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RIP, reinterpret_cast<unsigned long long>(_restore_guest));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_GUEST_RFLAGS, ::__readeflags());

	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_RSP, reinterpret_cast<unsigned long long>(vcpu_context->stack + sizeof(loader::VcpuContext) - 8));
	success &= vmx::vmwrite(intel::VmcsField::VMCS_HOST_RIP, reinterpret_cast<unsigned long long>(_vmexit_handler));

	if (success)
	{
		vcpu_context->vmcs_region = virtual_vmcs_region;
	}
}
