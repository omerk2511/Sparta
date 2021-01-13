#include "memory.h"

#include <ntddk.h>

auto operator new(size_t size, POOL_TYPE pool_type, ULONG tag) -> void*
{
    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}

auto operator new[](size_t size, POOL_TYPE pool_type, ULONG tag) -> void*
{
    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}

void operator delete(void* address)
{
	::ExFreePool(address);
}

void operator delete[](void* address)
{
	::ExFreePool(address);
}
