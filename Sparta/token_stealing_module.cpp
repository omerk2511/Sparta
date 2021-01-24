#include "token_stealing_module.h"
#include "vmx.h"

TokenStealingModule::TokenStealingModule() :
	sparta::BaseModule()
{
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::CR_ACCESS)] = handle_cr_access;
	// initialize SYSTEM tokens
}

void TokenStealingModule::initialize() volatile
{
	intel::PrimaryProcessorBasedVmxControls primary_processor_based_vmx_controls = { vmx::vmread<unsigned long>(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS).value };
	primary_processor_based_vmx_controls.cr3_load_exiting = true;
	vmx::vmwrite(intel::VmcsField::VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, primary_processor_based_vmx_controls.raw);
}

void TokenStealingModule::handle_cr_access(VcpuContext*, sparta::VmExitGuestState*, bool&)
{
}
