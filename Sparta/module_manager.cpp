#include "module_manager.h"

#include <ntddk.h>

static constexpr size_t MODULE_COUNT = 256;
volatile sparta::BaseModule* g_modules[MODULE_COUNT];

static constexpr auto NO_MODULE = 0ll;

bool sparta::register_module(BaseModule* module)
{
	for (auto module_index = 0; module_index < MODULE_COUNT; module_index++)
	{
		auto module_slot = &g_modules[module_index];
		if (::_InterlockedCompareExchange64(
			reinterpret_cast<volatile LONG64*>(module_slot),
			reinterpret_cast<LONG64>(module),
			NO_MODULE) == NO_MODULE)
		{
			return true;
		}
	}

	return false;
}

void sparta::unregister_module(BaseModule* module)
{
	for (auto module_index = 0; module_index < MODULE_COUNT; module_index++)
	{
		auto module_slot = &g_modules[module_index];
		if (::_InterlockedCompareExchange64(
			reinterpret_cast<volatile LONG64*>(module_slot),
			NO_MODULE,
			reinterpret_cast<LONG64>(module)) == reinterpret_cast<LONG64>(module))
		{
			return;
		}
	}
}

void sparta::initialize_modules()
{
	for (auto module : g_modules)
	{
		if (module)
		{
			module->initialize();
		}
	}
}

void sparta::call_modules(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip)
{
	for (auto module : g_modules)
	{
		if (module)
		{
			module->handle_vmexit(reason, vcpu_context, guest_state, increment_rip);
		}
	}
}
