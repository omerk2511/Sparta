#pragma once

#include "base_module.h"

class TokenStealingModule final : public sparta::BaseModule
{
public:
	explicit TokenStealingModule();

protected:
	virtual void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;

private:
	static bool handle_nmi(void* context, bool handled);

private:
	PACCESS_TOKEN _system_token;
	bool _should_bsod;
};
