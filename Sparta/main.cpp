#include <ntddk.h>

#include "validation.h"
#include "vmx.h"

void unload_routine(PDRIVER_OBJECT driver_object);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(registry_path);

	driver_object->DriverUnload = unload_routine;

	::ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	if (!validation::is_os_supported())
	{
		KdPrint(("[-] operating system not supported\n"));
		return STATUS_NOT_SUPPORTED;
	}

	if (!validation::is_vmx_supported())
	{
		KdPrint(("[-] vmx operation not supported\n"));
		return STATUS_NOT_SUPPORTED;
	}

	vmx::enable_vmx_operation(); // TODO: execute on every processor
	KdPrint(("[+] enabled vmx operation successfully\n"));

	KdPrint(("[+] loaded sparta successfully\n"));
	return STATUS_SUCCESS;
}

void unload_routine(PDRIVER_OBJECT driver_object)
{
	UNREFERENCED_PARAMETER(driver_object);

	KdPrint(("[+] unloaded sparta successfully\n"));
}
