#pragma once

#include <ntddk.h>

#include "helpers.h"

void* operator new(size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);
void* operator new[](size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);

void operator delete(void* address);
void operator delete[](void* address);

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

public:
	~UniquePointer()
	{
		if (_ptr)
		{
			_deleter(_ptr);
		}
	}

public:
	UniquePointer(const UniquePointer&) = delete;
	UniquePointer& operator=(const UniquePointer&) = delete;

public:
	UniquePointer(UniquePointer&& other)
		: _ptr{ other._ptr }
	{
		other._ptr = nullptr;
	}

	UniquePointer& operator=(UniquePointer&& other)
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

public:
	_T* get() { return _ptr; }
	const _T* get() const { return _ptr; }

public:
	_T* release()
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

public:
	_T& operator*() { return *_ptr; }
	const _T& operator*() const { return *_ptr; }

public:
	_T* operator->() { return _ptr; }
	const _T* operator->() const { return _ptr; }

public:
	_T& operator[](int index) { return _ptr[index]; };
	const _T& operator[](int index) const { return _ptr[index]; };

public:
	explicit operator bool() const { return _ptr; }

private:
	_T* _ptr;
	DefaultDeleter<T> _deleter;
};
