#pragma once
#pragma warning(disable: 4201)

#include <ntifs.h>

#include "intel.h"

namespace loader
{
	struct SpartaContext
	{
		unsigned long long host_cr3;
	};

	constexpr size_t STACK_LIMIT = 0x8000;

	struct VcpuContext
	{
		union
		{
			CONTEXT guest_context;
			unsigned char stack[STACK_LIMIT];
		};

		intel::EptPml4e pml4[intel::EPT_ENTRY_COUNT];
		intel::EptPdpte pdpt[intel::EPT_ENTRY_COUNT];
		intel::EptLargePde pde[intel::EPT_ENTRY_COUNT][intel::EPT_ENTRY_COUNT];

		void* vmxon_region;
		void* vmcs_region;
		unsigned long processor_index;
	};

	auto load_sparta(SpartaContext* sparta_context) -> bool;
}
