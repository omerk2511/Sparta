#pragma once

#include <ntddk.h>

namespace driver_io
{
	NTSTATUS create_close(PDEVICE_OBJECT device_object, PIRP irp);
	NTSTATUS read(PDEVICE_OBJECT device_object, PIRP irp);
	NTSTATUS ioctl(PDEVICE_OBJECT device_object, PIRP irp);
}
