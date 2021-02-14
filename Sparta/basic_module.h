#pragma once

#include "base_module.h"

class BasicModule final : public sparta::BaseModule
{
protected:
	virtual void handle_triple_fault(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_cpuid(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_invd(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_rdmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_xsetbv(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
};
