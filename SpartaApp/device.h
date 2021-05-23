#pragma once

#include <string>
#include <windows.h>

class Device final
{
public:
	explicit Device(const std::wstring& device_name);
	~Device();

public:
	bool read(void* buffer, size_t length, DWORD& bytes_read);

	bool ioctl(uint32_t control_code, void* input_buf, size_t input_size,
		void* output_buf, size_t output_size, DWORD* bytes_returned = nullptr);

private:
	HANDLE _handle;
};
