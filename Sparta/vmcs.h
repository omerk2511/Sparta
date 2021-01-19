#pragma once

#include "vcpu.h"
#include "loader.h"

namespace vmcs
{
	void setup(VcpuContext* vcpu_context, unsigned long long host_cr3);
}
