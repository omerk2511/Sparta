#include <ntddk.h>

#include "memory.h"
#include "validation.h"
#include "vmx.h"
#include "proccessor_info.h"
#include "multiprocessing.h"

ProcessorInfo* g_processors_info{ nullptr };

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

	auto processor_count = multiprocessing::get_processor_count();
	g_processors_info = new (NonPagedPool) ProcessorInfo[processor_count];

	if (!g_processors_info)
	{
		KdPrint(("[-] could not allocate the processor info array\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	multiprocessing::execute_callback_in_each_processor(vmx::enable_vmx_operation);
	KdPrint(("[+] enabled vmx operation successfully\n"));

	auto results = multiprocessing::execute_callback_in_each_processor(vmx::vmxon);

	for (auto i = 0; i < results.processor_count; i++)
	{
		if (!results.return_values[i])
		{
			KdPrint(("[-] could not enter vmx root mode on processor %d\n", i));
			return STATUS_HV_OPERATION_FAILED; // TODO: vmxoff in every initialized processor
		}
	}

	KdPrint(("[+] entered vmx root mode successfully\n"));
	KdPrint(("[+] loaded sparta successfully\n"));

	return STATUS_SUCCESS;
}

void unload_routine(PDRIVER_OBJECT driver_object)
{
	UNREFERENCED_PARAMETER(driver_object);

	// TODO: vmxoff in every processor, free vmxon & vmcs regions

	delete[] g_processors_info;
	g_processors_info = nullptr;

	KdPrint(("[+] unloaded sparta successfully\n"));
}
