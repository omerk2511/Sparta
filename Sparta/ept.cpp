#include "ept.h"

#include "vmx.h"
#include "asm_helpers.h"

void ept::setup(VcpuContext* vcpu_context)
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

static void* hooked_address = nullptr;
static bool (*hook_handler)(VcpuContext*) = nullptr;

void ept::hook_write(void* address, bool (*handler)(VcpuContext*), VcpuContext* vcpu_context)
{
	hooked_address = address;
	hook_handler = handler;

	auto lpage_address = reinterpret_cast<unsigned long long>(address) >> 21;
	auto& pde = vcpu_context->pd[lpage_address / intel::EPT_ENTRY_COUNT][lpage_address % intel::EPT_ENTRY_COUNT];

	pde.write = false;
	asm_helpers::invept();
}

void ept::unhook(VcpuContext* vcpu_context)
{
	auto lpage_address = reinterpret_cast<unsigned long long>(hooked_address) >> 21;
	auto& pde = vcpu_context->pd[lpage_address / intel::EPT_ENTRY_COUNT][lpage_address % intel::EPT_ENTRY_COUNT];

	pde.write = true;
	asm_helpers::invept();

	hooked_address = nullptr;
	hook_handler = nullptr;
}

bool ept::handle_violation(VcpuContext* vcpu_context)
{
	auto [success, address] = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PHYSICAL_ADDRESS);
	if (reinterpret_cast<void*>(address) == hooked_address)
	{
		return hook_handler(vcpu_context);
	}

	auto lpage_address = address >> 21;
	auto& pde = vcpu_context->pd[lpage_address / intel::EPT_ENTRY_COUNT][lpage_address % intel::EPT_ENTRY_COUNT];

	pde.write = true;
	asm_helpers::invept();

	intel::PrimaryProcessorBasedVmxControls primary_processor_based_vmx_controls = { vmx::vmread<unsigned long>(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS).value };
	primary_processor_based_vmx_controls.monitor_trap_flag = true;
	vmx::vmwrite(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, primary_processor_based_vmx_controls.raw);

	return false;
}

void ept::handle_trap(VcpuContext* vcpu_context)
{
	hook_write(hooked_address, hook_handler, vcpu_context);

	intel::PrimaryProcessorBasedVmxControls primary_processor_based_vmx_controls = { vmx::vmread<unsigned long>(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS).value };
	primary_processor_based_vmx_controls.monitor_trap_flag = false;
	vmx::vmwrite(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, primary_processor_based_vmx_controls.raw);
}
