#pragma once

#include <ntddk.h>

void* operator new(size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);
void* operator new[](size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);

void operator delete(void* address);
void operator delete[](void* address);

template<typename T>
class UniquePointer final
{
public:
	UniquePointer()
		: _ptr{ nullptr }
	{
	}

	UniquePointer(T* ptr)
		: _ptr{ ptr }
	{
	}

public:
	~UniquePointer()
	{
		if (_ptr)
		{
			delete _ptr;
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
			if (_ptr) delete _ptr;

			_ptr = other._ptr;
			other._ptr = nullptr;
		}

		return *this;
	}

public:
	T* get() { return _ptr; }
	const T* get() const { return _ptr; }

public:
	T* release()
	{
		auto temp = _ptr;
		_ptr = nullptr;
		return temp;
	}

	void reset(T* ptr)
	{
		auto current = _ptr;
		_ptr = ptr;

		if (_ptr)
		{
			delete _ptr;
		}
	}

	void swap(UniquePointer<T>& other)
	{
		auto temp = _ptr;
		_ptr = other._ptr;
		other._ptr = temp;
	}

public:
	T& operator*() { return *_ptr; }
	const T& operator*() const { return *_ptr; }

public:
	T* operator->() { return _ptr; }
	const T* operator->() const { return _ptr; }

public:
	T& operator[](int index) { return _ptr[index]; };
	const T& operator[](int index) const { return _ptr[index]; };

public:
	explicit operator bool() const { return _ptr; }

private:
	T* _ptr;
};

template<typename T>
class UniquePointer<T[]> final
{
public:
	UniquePointer()
		: _ptr{ nullptr }
	{
	}

	UniquePointer(T* ptr)
		: _ptr{ ptr }
	{
	}

public:
	~UniquePointer()
	{
		if (_ptr)
		{
			delete[] _ptr;
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
			if (_ptr) delete[] _ptr;

			_ptr = other._ptr;
			other._ptr = nullptr;
		}

		return *this;
	}

public:
	T* get() { return _ptr; }
	const T* get() const { return _ptr; }

public:
	T* release()
	{
		auto temp = _ptr;
		_ptr = nullptr;
		return temp;
	}

	void reset(T* ptr)
	{
		auto current = _ptr;
		_ptr = ptr;

		if (_ptr)
		{
			delete[] _ptr;
		}
	}

	void swap(UniquePointer<T>& other)
	{
		auto temp = _ptr;
		_ptr = other._ptr;
		other._ptr = temp;
	}

public:
	T& operator*() { return *_ptr; }
	const T& operator*() const { return *_ptr; }

public:
	T* operator->() { return _ptr; }
	const T* operator->() const { return _ptr; }

public:
	T& operator[](int index) { return _ptr[index]; };
	const T& operator[](int index) const { return _ptr[index]; };

public:
	explicit operator bool() const { return _ptr; }

private:
	T* _ptr;
};
