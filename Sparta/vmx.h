#pragma once

#include "proccessor_info.h"

namespace vmx
{
	void enable_vmx_operation(unsigned int processor_index);

	bool vmxon(unsigned int processor_index);
}
