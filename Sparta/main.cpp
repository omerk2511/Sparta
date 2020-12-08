#include <ntddk.h>

#include "memory.h"
#include "validation.h"
#include "vmx.h"
#include "multiprocessing.h"
#include "loader.h"

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

	loader::SpartaContext sparta_context;
	sparta_context.host_cr3 = ::__readcr3();

	auto results = multiprocessing::execute_callback_in_each_processor(&loader::load_sparta, &sparta_context);

	for (auto i = 0; i < results.processor_count; i++)
	{
		if (!results.return_values[i])
		{
			KdPrint(("[-] could not initialize vmx on processor %d\n", i));
			return STATUS_HV_OPERATION_FAILED; // TODO: vmxoff in every initialized processor
		}
	}

	KdPrint(("[+] loaded sparta successfully\n"));

	return STATUS_SUCCESS;
}

void unload_routine(PDRIVER_OBJECT driver_object)
{
	UNREFERENCED_PARAMETER(driver_object);

	// TODO: vmxoff in every processor, free vmxon & vmcs regions

	KdPrint(("[+] unloaded sparta successfully\n"));
}
