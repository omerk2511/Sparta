#pragma once

#include "base_module.h"

class TokenStealingModule final : public sparta::BaseModule
{
public:
	explicit TokenStealingModule();

private:
	static void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip);

private:
	static bool handle_nmi(void* context, bool handled);

private:
	static PACCESS_TOKEN _system_token;
	static bool _should_bsod;
};
