#pragma once

#include "base_module.h"

class KernelIntegrityModule final : public sparta::BaseModule
{
public:
	virtual void initialize(VcpuContext* vcpu_context) volatile override;

protected:
	virtual void handle_ept_violation(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_cr_access(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;

private:
	unsigned long long _ssdt_base_physical_address;
	unsigned long long _ssdt_end_physical_address;
};
