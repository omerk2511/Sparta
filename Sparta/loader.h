#pragma once

namespace loader
{
	struct SpartaContext
	{
		unsigned long long host_cr3;
	};

	auto load_sparta(SpartaContext* sparta_context) -> bool;
}
