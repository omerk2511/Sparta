#pragma once

namespace kstd
{
	template<bool B, typename T, typename F>
	struct conditional { using type = F; };

	template<typename T, typename F>
	struct conditional<true, T, F> { using type = T; };

	template<bool B, typename T, typename F>
	using conditional_t = typename conditional<B, T, F>::type;

	template<typename T, typename F>
	constexpr bool is_same = false;

	template<typename T>
	constexpr bool is_same<T, T> = true;

	template<typename T>
	struct remove_extent { using type = T; };

	template<typename T>
	struct remove_extent<T[]> { using type = T; };

	template<typename T>
	using remove_extent_t = typename remove_extent<T>::type;

	template<typename T>
	constexpr bool is_unsigned_integer = false;

	template<>
	constexpr bool is_unsigned_integer<unsigned int> = true;

	template<>
	constexpr bool is_unsigned_integer<unsigned short> = true;

	template<>
	constexpr bool is_unsigned_integer<unsigned long> = true;

	template<>
	constexpr bool is_unsigned_integer<unsigned long long> = true;

	template<typename T, typename F>
	struct pair
	{
		T first;
		F second;
	};
}
