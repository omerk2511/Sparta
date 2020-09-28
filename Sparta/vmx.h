#pragma once

namespace vmx
{
	void enable_vmx_operation(unsigned int processor_index);

	bool vmxon(unsigned int processor_index);
}
