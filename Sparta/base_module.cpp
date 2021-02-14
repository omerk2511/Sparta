#include "base_module.h"

void sparta::BaseModule::initialize(VcpuContext*) volatile
{
}

void sparta::BaseModule::handle_vmexit(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip) volatile
{
	switch (reason)
	{
	case intel::VmExitReason::EXCEPTION_OR_NMI: return handle_exception_or_nmi(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::EXTERNAL_INTERRUPT: return handle_external_interrupt(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::TRIPLE_FAULT: return handle_triple_fault(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INIT_SIGNAL: return handle_init_signal(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::SIPI: return handle_sipi(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::IO_SMI: return handle_io_smi(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::OTHER_SMI: return handle_other_smi(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INTERRUPT_WINDOW: return handle_interrupt_window(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::NMI_WINDOW: return handle_nmi_window(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::TASK_SWITCH: return handle_task_switch(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::CPUID: return handle_cpuid(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::GETSEC: return handle_getsec(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::HLT: return handle_hlt(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INVD: return handle_invd(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INVLPG: return handle_invlpg(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDPMC: return handle_rdpmc(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDTMC: return handle_rdtmc(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RSM: return handle_rsm(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMCALL: return handle_vmcall(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMCLEAR: return handle_vmclear(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMLAUNCH: return handle_vmlaunch(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMPTRLD: return handle_vmptrld(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMPTRST: return handle_vmptrst(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMREAD: return handle_vmread(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMRESUME: return handle_vmresume(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMWRITE: return handle_vmwrite(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMXOFF: return handle_vmxoff(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMXON: return handle_vmxon(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::CR_ACCESS: return handle_cr_access(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::MOV_DR: return handle_mov_dr(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::IO_INSTRUCTION: return handle_io_instruction(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDMSR: return handle_rdmsr(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::WRMSR: return handle_wrmsr(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VM_ENTRY_FAILURE_GUEST_STATE: return handle_vm_entry_failure_guest_state(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VM_ENTRY_FAILURE_MSR_LOADING: return handle_vm_entry_failure_msr_loading(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::MWAIT: return handle_mwait(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::MTF: return handle_mtf(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::MONITOR: return handle_monitor(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::PAUSE: return handle_pause(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VM_ENTRY_FAILURE_MACHINE_CHECK_EVENT: return handle_vm_entry_failure_machine_check_event(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::TPR_BELOW_THRESHOLD: return handle_tpr_below_threshold(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::APIC_ACCESS: return handle_apic_access(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VIRTUALIZED_EOI: return handle_virtualized_eoi(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::ACCESS_TO_GDTR_IDTR: return handle_access_to_gdtr_idtr(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::ACESS_TO_LDTR_TR: return handle_acess_to_ldtr_tr(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::EPT_VIOLATION: return handle_ept_violation(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::EPT_MISCONFIGURATION: return handle_ept_misconfiguration(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INVEPT: return handle_invept(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDTSCP: return handle_rdtscp(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMX_PREEMPTION_TIME_EXPIRED: return handle_vmx_preemption_time_expired(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INVVPID: return handle_invvpid(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::WBINVD: return handle_wbinvd(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::XSETBV: return handle_xsetbv(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::APIC_WRITE: return handle_apic_write(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDRAND: return handle_rdrand(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::INVPCID: return handle_invpcid(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::VMFUNC: return handle_vmfunc(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::ENCLS: return handle_encls(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::RDSEED: return handle_rdseed(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::PML_FULL: return handle_pml_full(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::XSAVES: return handle_xsaves(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::XRSTORS: return handle_xrstors(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::SPP_RELATED_EVENT: return handle_spp_related_event(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::UMWAIT: return handle_umwait(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::TPAUSE: return handle_tpause(vcpu_context, guest_state, increment_rip);
	case intel::VmExitReason::MAX: return handle_max(vcpu_context, guest_state, increment_rip);
	}
}

void sparta::BaseModule::handle_exception_or_nmi(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_external_interrupt(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_triple_fault(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_init_signal(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_sipi(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_io_smi(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_other_smi(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_interrupt_window(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_nmi_window(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_task_switch(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_cpuid(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_getsec(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_hlt(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_invd(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_invlpg(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdpmc(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdtmc(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rsm(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmcall(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmclear(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmlaunch(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmptrld(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmptrst(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmread(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmresume(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmwrite(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmxoff(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmxon(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_cr_access(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_mov_dr(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_io_instruction(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdmsr(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_wrmsr(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vm_entry_failure_guest_state(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vm_entry_failure_msr_loading(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_mwait(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_mtf(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_monitor(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_pause(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vm_entry_failure_machine_check_event(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_tpr_below_threshold(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_apic_access(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_virtualized_eoi(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_access_to_gdtr_idtr(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_acess_to_ldtr_tr(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_ept_violation(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_ept_misconfiguration(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_invept(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdtscp(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmx_preemption_time_expired(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_invvpid(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_wbinvd(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_xsetbv(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_apic_write(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdrand(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_invpcid(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_vmfunc(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_encls(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_rdseed(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_pml_full(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_xsaves(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_xrstors(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_spp_related_event(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_umwait(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_tpause(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}

void sparta::BaseModule::handle_max(VcpuContext*, sparta::VmExitGuestState*, bool&) volatile
{
}
