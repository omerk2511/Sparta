#include "driver_io.h"
#include "logging.h"

extern void* LoadLibraryA;
extern void* LoadLibraryW;

#define SPARTA_DEVICE 0x8000

#define IOCTL_SPARTA_CONFIGURE_LOAD_LIBRARY CTL_CODE( \
	SPARTA_DEVICE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPARTA_DUMP_KERNEL_MEMORY CTL_CODE( \
	SPARTA_DEVICE, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

struct ConfigureLoadLibraryParameters
{
    void* LoadLibraryA;
    void* LoadLibraryW;
};

struct DumpKernelMemoryParameters
{
    void* address;
    size_t size;
};

NTSTATUS driver_io::create_close(PDEVICE_OBJECT, PIRP irp)
{
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;

    ::IoCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS driver_io::read(PDEVICE_OBJECT, PIRP irp)
{
    auto irp_stack_location = ::IoGetCurrentIrpStackLocation(irp);
    auto length = irp_stack_location->Parameters.Read.Length;

    auto buffer = reinterpret_cast<logging::UsermodeThreatReport*>(::MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority));

    if (!buffer)
    {
        irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        irp->IoStatus.Information = 0;

        ::IoCompleteRequest(irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    auto returned_size = logging::dump_log(buffer, length);

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = returned_size;

    ::IoCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS driver_io::ioctl(PDEVICE_OBJECT, PIRP irp)
{
    auto irp_stack_location = ::IoGetCurrentIrpStackLocation(irp);
    auto ioctl = irp_stack_location->Parameters.DeviceIoControl.IoControlCode;

    auto status = STATUS_INVALID_DEVICE_REQUEST;
    auto information = 0ull;

    switch (ioctl)
    {
    case IOCTL_SPARTA_CONFIGURE_LOAD_LIBRARY:
    {
        auto buffer = irp->AssociatedIrp.SystemBuffer;
        auto length = irp_stack_location->Parameters.DeviceIoControl.InputBufferLength;

        if (length != sizeof(ConfigureLoadLibraryParameters) || buffer == nullptr)
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        auto parameters = reinterpret_cast<ConfigureLoadLibraryParameters*>(buffer);

        LoadLibraryA = parameters->LoadLibraryA;
        LoadLibraryW = parameters->LoadLibraryW;

        status = STATUS_SUCCESS;

        break;
    }

    case IOCTL_SPARTA_DUMP_KERNEL_MEMORY:
    {
        auto in_buffer = irp_stack_location->Parameters.DeviceIoControl.Type3InputBuffer;
        auto in_length = irp_stack_location->Parameters.DeviceIoControl.InputBufferLength;

        if (in_length != sizeof(ConfigureLoadLibraryParameters) || in_buffer == nullptr)
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        auto parameters = reinterpret_cast<DumpKernelMemoryParameters*>(in_buffer);

        auto out_buffer = irp->UserBuffer;
        auto out_length = irp_stack_location->Parameters.DeviceIoControl.OutputBufferLength;

        if (out_length < parameters->size || out_buffer == nullptr)
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        ::memcpy(out_buffer, parameters->address, parameters->size);

        status = STATUS_SUCCESS;
        information = parameters->size;

        break;
    }
    }

    irp->IoStatus.Status = status;
    irp->IoStatus.Information = information;

    ::IoCompleteRequest(irp, IO_NO_INCREMENT);

    return status;
}
