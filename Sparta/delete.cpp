#include "delete.h"

#include <ntddk.h>

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
