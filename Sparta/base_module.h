#pragma once

#include "intel.h"
#include "vcpu.h"
#include "memory.h"

namespace sparta
{
#pragma pack(push, 1)
	struct VmExitGuestState
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
		unsigned long long cr2;
		unsigned long long r15;
	};
#pragma pack(pop)

	class BaseModule
	{
	public:
		virtual ~BaseModule() {}

	public:
		virtual void initialize(VcpuContext* vcpu_context) volatile;

	public:
		void handle_vmexit(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip) volatile;

	protected:
		virtual void handle_exception_or_nmi(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_external_interrupt(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_triple_fault(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_init_signal(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_sipi(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_io_smi(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_other_smi(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_interrupt_window(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_nmi_window(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_task_switch(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_cpuid(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_getsec(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_hlt(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_invd(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_invlpg(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdpmc(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdtmc(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rsm(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmcall(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmclear(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmlaunch(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmptrld(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmptrst(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmread(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmresume(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmwrite(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmxoff(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmxon(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_cr_access(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_mov_dr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_io_instruction(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vm_entry_failure_guest_state(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vm_entry_failure_msr_loading(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_mwait(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_mtf(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_monitor(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_pause(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vm_entry_failure_machine_check_event(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_tpr_below_threshold(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_apic_access(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_virtualized_eoi(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_access_to_gdtr_idtr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_acess_to_ldtr_tr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_ept_violation(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_ept_misconfiguration(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_invept(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdtscp(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmx_preemption_time_expired(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_invvpid(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_wbinvd(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_xsetbv(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_apic_write(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdrand(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_invpcid(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_vmfunc(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_encls(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_rdseed(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_pml_full(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_xsaves(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_xrstors(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_spp_related_event(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_umwait(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_tpause(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
		virtual void handle_max(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile;
	};
}
