#pragma once

#include "base_module.h"
#include "vcpu.h"

namespace sparta
{
	bool register_module(BaseModule* module);
	void unregister_module(BaseModule* module);

	void initialize_modules();
	void call_modules(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip);
}
