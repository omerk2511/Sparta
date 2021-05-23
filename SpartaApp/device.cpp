#include "device.h"

Device::Device(const std::wstring& device_name) :
    _handle(::CreateFile(
			device_name.c_str(),
			GENERIC_ALL,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr))
{
	if (_handle == INVALID_HANDLE_VALUE)
	{
		throw std::exception("could not open a device handle");
	}
}

Device::~Device()
{
	::CloseHandle(_handle);
}

bool Device::read(void* buffer, size_t length, DWORD& bytes_read)
{
	auto success = ::ReadFile(
		_handle,
		buffer,
		length,
		&bytes_read,
		nullptr
	);

	return success;
}

bool Device::ioctl(uint32_t control_code, void* input_buf, size_t input_size, void* output_buf, size_t output_size, DWORD* bytes_returned)
{
	auto success = ::DeviceIoControl(
		_handle,
		control_code,
		input_buf,
		input_size,
		output_buf,
		output_size,
		bytes_returned,
		nullptr
	);

	return success;
}
