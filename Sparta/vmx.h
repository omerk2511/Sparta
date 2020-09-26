#pragma once

namespace vmx
{
	void enable_vmx_operation();

	struct VmxonResult
	{
		bool success;
		void* vmxon_region;
	};

	VmxonResult vmxon();
}
