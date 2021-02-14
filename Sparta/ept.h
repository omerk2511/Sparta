#pragma once

#include "vcpu.h"

namespace ept
{
	void setup(VcpuContext* vcpu_context);

	intel::EptPte* split_large_page(intel::EptLargePde* large_pde);
	
	void hook_write(void* address, bool (*handler)(VcpuContext*), VcpuContext* vcpu_context);
	void unhook(VcpuContext* vcpu_context);

	bool handle_violation(VcpuContext* vcpu_context);
	void handle_trap(VcpuContext* vcpu_context);
}
