#pragma once

#include "intel.h"
#include "kstd.h"
#include "ept.h"

#include <intrin.h>
#include <ntddk.h>

namespace vmx
{
	void enable_vmx();
	auto vmxon() -> void*;
	auto vmlaunch() -> bool;
	auto vmclear(unsigned long long* vmcs) -> bool;
	auto vmptrld(unsigned long long* vmcs) -> bool;

	void adjust_vmx_controls(unsigned long& vmx_controls, intel::Ia32VmxControlsHint hint);

	void inject_nmi();

	template<typename T>
		requires kstd::is_unsigned_integer<T>
	struct VmreadResult
	{
		bool success;
		T value;
	};

	template<typename T>
	static inline constexpr auto vmread(intel::VmcsField vmcs_field) -> VmreadResult<T>
		requires kstd::is_unsigned_integer<T>
	{
		size_t value{ };
		auto ret = ::__vmx_vmread(static_cast<size_t>(vmcs_field), &value);
		return { ret == STATUS_SUCCESS, static_cast<T>(value) };
	}

	template<typename T>
	static inline constexpr auto vmwrite(intel::VmcsField vmcs_field, T value) -> bool
		requires kstd::is_unsigned_integer<T>
	{
		auto ret = ::__vmx_vmwrite(static_cast<size_t>(vmcs_field), static_cast<size_t>(value));
		return ret == STATUS_SUCCESS;
	}
}
