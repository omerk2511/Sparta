#pragma once

#include "base_module.h"

class BasicModule final : public sparta::BaseModule
{
public:
	explicit BasicModule();

private:
	static void handle_triple_fault(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
	static void handle_cpuid(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
	static void handle_invd(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
	static void handle_rdmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
	static void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
	static void handle_xsetbv(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);
};
