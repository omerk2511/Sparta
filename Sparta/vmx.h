#pragma once

#include "intel.h"
#include "templates.h"

#include <intrin.h>

extern "C" void _sgdt(void*); // for some weird reason, this intrinsic function is not in <intrin.h>

namespace vmx
{
	bool initialize_vmx(unsigned int processor_index);

	template<typename T>
		requires is_unsigned_integer<T>
	struct VmreadResult
	{
		bool success;
		T value;
	};

	template<typename T>
	static inline constexpr VmreadResult<T> vmread(intel::VmcsField vmcs_field)
		requires is_unsigned_integer<T>
	{
		size_t value{ };

		auto ret = ::__vmx_vmread(static_cast<size_t>(vmcs_field), &value);

		if (ret == STATUS_SUCCESS)
		{
			return { true, static_cast<T>(value) };
		}

		return { false, 0 };
	}

	template<typename T>
	static inline constexpr bool vmwrite(intel::VmcsField vmcs_field, T value)
		requires is_unsigned_integer<T>
	{
		auto ret = ::__vmx_vmwrite(static_cast<size_t>(vmcs_field), static_cast<size_t>(value));
		return (ret == STATUS_SUCCESS);
	}
}
