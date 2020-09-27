#include "new.h"

void* operator new(size_t size, POOL_TYPE pool_type, ULONG tag)
{
    KdPrint(("[*] allocating mem sized 0x%x\n", size));

    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}

void* operator new[](size_t size, POOL_TYPE pool_type, ULONG tag)
{
    KdPrint(("[*] allocating [] sized 0x%x\n", size));

    return ::ExAllocatePoolWithTag(
        pool_type,
        size,
        tag
    );
}
