#pragma once
#pragma warning(disable: 4201)

#include <ntifs.h>

#include "intel.h"

constexpr size_t STACK_LIMIT = 0x1000;

struct VcpuContext
{
	union
	{
		CONTEXT guest_context;
		unsigned char stack[STACK_LIMIT];
	};

	intel::EptPml4e pml4[intel::EPT_ENTRY_COUNT];
	intel::EptPdpte pdpt[intel::EPT_ENTRY_COUNT];
	intel::EptLargePde pd[intel::EPT_ENTRY_COUNT][intel::EPT_ENTRY_COUNT];

	void* vmxon_region;
	void* vmcs_region;
	unsigned long processor_index;
};
