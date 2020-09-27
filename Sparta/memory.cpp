#include "memory.h"

#include <ntddk.h>

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


void operator delete(void* address)
{
	KdPrint(("[*] deleting mem @0x%p\n", address));
	::ExFreePool(address);
}

void operator delete[](void* address)
{
	KdPrint(("[*] deleting [] @0x%p\n", address));
	::ExFreePool(address);
}
