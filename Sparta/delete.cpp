#include "delete.h"

#include <ntddk.h>

void operator delete(void* address)
{
	::ExFreePool(address);
}

void operator delete[](void* address)
{
	::ExFreePool(address);
}
