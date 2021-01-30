#include "base_module.h"

void sparta::BaseModule::initialize() volatile
{
}

void sparta::BaseModule::handle_vmexit(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip) const volatile
{
	auto handler = _vmexit_handlers[static_cast<size_t>(reason)];
	if (handler)
	{
		handler(vcpu_context, guest_state, increment_rip);
	}
}
