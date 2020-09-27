#pragma once

#include <ntddk.h>

void* operator new(size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);
void* operator new[](size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = 0);
