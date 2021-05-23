#pragma once

namespace nt_tricks
{
	struct SectionInformation
	{
		void* address;
		size_t size;
	};

	void* get_syscall_handler_address(const char* syscall_name);

	void load_ntdll();

	void* get_ntoskrnl_base();

	void* get_ssdt_base();
	void* get_ssdt_end();

	size_t get_pe_virtual_size(void* image_base);
	SectionInformation get_section_information(void* image_base, const char* section_name);
	void* find_pe_export(void* pe_base, const char* export_name);
}
