#pragma once

#include <ntddk.h>

#include "kstd.h"

auto operator new(size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0) -> void*;
auto operator new[](size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0) -> void*;

void operator delete(void* address);
void operator delete[](void* address);

namespace kstd
{
	template<typename T>
	class DefaultDeleter final
	{
	public:
		void operator()(T* ptr)
		{
			delete ptr;
		}
	};

	template<typename T>
	class DefaultDeleter<T[]> final
	{
	public:
		void operator()(T* ptr)
		{
			delete[] ptr;
		}
	};

	template<typename T>
	class UniquePointer final
	{
	private:
		using _T = remove_extent_t<T>;

	public:
		UniquePointer()
			: _ptr{ nullptr }
		{
		}

		UniquePointer(_T* ptr)
			: _ptr{ ptr }
		{
		}

		~UniquePointer()
		{
			if (_ptr)
			{
				_deleter(_ptr);
			}
		}

		UniquePointer(const UniquePointer&) = delete;
		auto operator=(const UniquePointer&) -> UniquePointer& = delete;

		UniquePointer(UniquePointer&& other)
			: _ptr{ other._ptr }
		{
			other._ptr = nullptr;
		}

		auto operator=(UniquePointer&& other) -> UniquePointer&
		{
			if (this != &other)
			{
				if (_ptr)
				{
					_deleter(_ptr);
				}

				_ptr = other._ptr;
				other._ptr = nullptr;
			}

			return *this;
		}

		[[nodiscard]] auto get() -> _T* { return _ptr; }
		[[nodiscard]] auto get() const -> const _T* { return _ptr; }

		auto release() -> _T*
		{
			auto temp = _ptr;
			_ptr = nullptr;
			return temp;
		}

		void reset(_T* ptr)
		{
			auto current = _ptr;
			_ptr = ptr;

			if (_ptr)
			{
				_deleter(_ptr);
			}
		}

		void swap(UniquePointer<T>& other)
		{
			auto temp = _ptr;
			_ptr = other._ptr;
			other._ptr = temp;
		}

		auto operator*() -> _T& { return *_ptr; }
		auto operator*() const -> const _T& { return *_ptr; }

		auto operator->() -> _T* { return _ptr; }
		auto operator->() const -> const _T* { return _ptr; }

		auto operator[](int index) -> _T& { return _ptr[index]; };
		auto operator[](int index) const -> const _T& { return _ptr[index]; };

		explicit operator bool() const { return _ptr; }

	private:
		_T* _ptr;
		DefaultDeleter<T> _deleter;
	};
}
