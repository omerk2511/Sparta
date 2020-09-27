#include "new.h"

void* operator new(size_t size, POOL_TYPE pool_type, ULONG tag)
{
    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}

void* operator new[](size_t size, POOL_TYPE pool_type, ULONG tag)
{
    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}
