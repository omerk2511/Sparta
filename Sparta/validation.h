#pragma once

#include <ntddk.h>

namespace validation
{
	bool is_os_supported();
	bool is_vmx_supported();
}
