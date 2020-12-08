#pragma once
#pragma warning(disable: 4201)

#include <ntifs.h>

namespace loader
{
	struct SpartaContext
	{
		unsigned long long host_cr3;
	};

	constexpr size_t STACK_LIMIT = 0x8000;

	union VcpuContext
	{
		struct
		{
			CONTEXT guest_context;
			void* vmxon_region;
			void* vmcs_region;
			unsigned long processor_index;
		};

		unsigned char stack[STACK_LIMIT];
	};

	bool load_sparta(SpartaContext* sparta_context);
}
