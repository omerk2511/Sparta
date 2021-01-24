#pragma once

#include "base_module.h"

class TokenStealingModule final : public sparta::BaseModule
{
public:
	explicit TokenStealingModule();

public:
	virtual void initialize() volatile override;

private:
	static void handle_cr_access(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);

private:
	// SYSTEM tokens
};
