#include <ntifs.h>
#include <ntddk.h>

#include "memory.h"
#include "validation.h"
#include "vmx.h"
#include "multiprocessing.h"
#include "loader.h"
#include "nt_tricks.h"
#include "driver_io.h"

PDEVICE_OBJECT g_device_object = nullptr;

void unload_routine(PDRIVER_OBJECT driver_object);

extern "C" auto DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) -> NTSTATUS
{
	UNREFERENCED_PARAMETER(registry_path);

	::ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	driver_object->DriverUnload = unload_routine;

	driver_object->MajorFunction[IRP_MJ_CREATE] = &driver_io::create_close;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = &driver_io::create_close;
	driver_object->MajorFunction[IRP_MJ_READ] = &driver_io::read;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &driver_io::ioctl;

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

	UNICODE_STRING device_name;
	::RtlInitUnicodeString(&device_name, L"\\Device\\Sparta");

	PDEVICE_OBJECT device_object;
	NTSTATUS status = ::IoCreateDevice(
		driver_object,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&device_object
	);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("[-] failed to create a device object\n"));
		return status;
	}

	g_device_object = device_object;

	device_object->Flags |= DO_DIRECT_IO;

	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, L"\\??\\Sparta");

	status = ::IoCreateSymbolicLink(&symbolic_link, &device_name);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		KdPrint(("[-] failed to create a symbolic link\n"));

		return status;
	}

	loader::SpartaContext sparta_context;
	sparta_context.host_cr3 = ::__readcr3();

	auto lolz = reinterpret_cast<unsigned char*>(nt_tricks::get_syscall_handler_address("NtCreateThreadEx"));
	KdPrint(("[*] NtCreateThreadEx before: %x %x %x %x %x %x %x %x %x\n", lolz[0], lolz[1], lolz[2], lolz[3], lolz[4], lolz[5], lolz[6], lolz[7], lolz[8]));

	auto results = multiprocessing::execute_callback_in_each_processor(&loader::load_sparta, &sparta_context);

	for (auto i = 0; i < results.processor_count; i++)
	{
		if (!results.return_values[i])
		{
			::IoDeleteSymbolicLink(&symbolic_link);
			::IoDeleteDevice(device_object);

			KdPrint(("[-] could not initialize vmx\n"));

			return STATUS_HV_OPERATION_FAILED; // TODO: vmxoff in every initialized processor
		}
	}

	KdPrint(("[+] loaded sparta successfully\n"));

	KdPrint(("[*] NtCreateThreadEx after: %x %x %x %x %x %x %x %x %x\n", lolz[0], lolz[1], lolz[2], lolz[3], lolz[4], lolz[5], lolz[6], lolz[7], lolz[8]));

	return STATUS_SUCCESS;
}

void unload_routine(PDRIVER_OBJECT driver_object)
{
	UNREFERENCED_PARAMETER(driver_object);

	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, L"\\??\\Sparta");

	::IoDeleteSymbolicLink(&symbolic_link);

	::IoDeleteDevice(driver_object->DeviceObject);

	// TODO: vmxoff in every processor, free vmxon & vmcs regions

	KdPrint(("[+] unloaded sparta successfully\n"));
}
