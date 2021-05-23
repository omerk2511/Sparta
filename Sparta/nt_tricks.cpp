#include "nt_tricks.h"
#include "memory.h"

#include <ntddk.h>
#include <aux_klib.h>

#pragma comment(lib, "aux_klib.lib")

static constexpr char NTOSKRNL_NAME[] = "ntoskrnl.exe";

static bool aux_klib_initialized = false;

static void* ntdll_base;

void* nt_tricks::get_ntoskrnl_base()
{
	// race condition - move to main!
	if (!aux_klib_initialized)
	{
		auto status = ::AuxKlibInitialize();

		if (!NT_SUCCESS(status)) {
			return nullptr;
		}

		aux_klib_initialized = true;
	}

	ULONG modules_size{ };

	auto status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		nullptr
	);

	if (!NT_SUCCESS(status) || modules_size == 0) {
		return nullptr;
	}

	auto module_count = modules_size / sizeof(AUX_MODULE_EXTENDED_INFO);
	auto modules = new (NonPagedPool) AUX_MODULE_EXTENDED_INFO[module_count];

	if (!modules) {
		return nullptr;
	}

	RtlZeroMemory(modules, modules_size);

	status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		modules
	);

	if (!NT_SUCCESS(status)) {
		delete modules;
		return nullptr;
	}

	void* ntoskrnl_base = nullptr;

	for (size_t i = 0; i < module_count; i++) {
		auto module = &modules[i];
		auto module_name = reinterpret_cast<char*>(module->FullPathName) + module->FileNameOffset;

		if (::strcmp(module_name, NTOSKRNL_NAME) == 0) {
			ntoskrnl_base = module->BasicInfo.ImageBase;
			break;
		}
	}

	delete modules;

    return ntoskrnl_base;
}

size_t nt_tricks::get_pe_virtual_size(void* image_base)
{
	auto pe_base = reinterpret_cast<unsigned char*>(image_base);

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(pe_base);
	auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(pe_base + dos_header->e_lfanew);

	return nt_headers->OptionalHeader.SizeOfImage;
}

nt_tricks::SectionInformation nt_tricks::get_section_information(void* image_base, const char* section_name)
{
	auto pe_base = reinterpret_cast<unsigned char*>(image_base);

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(pe_base);
	auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(pe_base + dos_header->e_lfanew);

	auto current_section_header = reinterpret_cast<PIMAGE_SECTION_HEADER>(pe_base + dos_header->e_lfanew +
		sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER) + nt_headers->FileHeader.SizeOfOptionalHeader);

	SectionInformation section_information = { 0 };

	for (auto i = 0; i < nt_headers->FileHeader.NumberOfSections; i++) {
		auto current_section_name = reinterpret_cast<char*>(current_section_header->Name);

		if (::strlen(current_section_name) == ::strlen(section_name)) {
			auto match = ::RtlCompareMemory(current_section_name, section_name, ::strlen(current_section_name));

			if (match == ::strlen(current_section_name)) {
				section_information.address = pe_base + current_section_header->VirtualAddress;
				section_information.size = current_section_header->Misc.VirtualSize;

				break;
			}
		}

		current_section_header++;
	}

	return section_information;
}

void* nt_tricks::find_pe_export(void* pe_base, const char* export_name)
{
	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_EXPORT_DIRECTORY pIED;

	PULONG Address, Name;
	PUSHORT Ordinal;

	ULONG i;

	pIDH = (PIMAGE_DOS_HEADER)pe_base;
	pINH = (PIMAGE_NT_HEADERS)((PUCHAR)pe_base + pIDH->e_lfanew);

	pIED = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pe_base + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	Address = (PULONG)((PUCHAR)pe_base + pIED->AddressOfFunctions);
	Name = (PULONG)((PUCHAR)pe_base + pIED->AddressOfNames);

	Ordinal = (PUSHORT)((PUCHAR)pe_base + pIED->AddressOfNameOrdinals);

	for (i = 0; i < pIED->AddressOfFunctions; i++)
	{
		if (!strcmp(export_name, (char*)pe_base + Name[i]))
		{
			return (PVOID)((PUCHAR)pe_base + Address[Ordinal[i]]);
		}
	}

	return nullptr;
}

void* nt_tricks::get_ssdt_base()
{
	auto ntoskrnl_base = get_ntoskrnl_base();

	return reinterpret_cast<char*>(ntoskrnl_base) + 0xc83a0;

	/*auto ntoskrnl_end = reinterpret_cast<void*>(reinterpret_cast<char*>(ntoskrnl_base) + get_pe_virtual_size(ntoskrnl_base));

	auto rdata_section_info = get_section_information(ntoskrnl_base, ".rdata");

	auto rdata_section_arr = reinterpret_cast<unsigned long*>(rdata_section_info.address);
	auto rdata_section_arr_size = rdata_section_info.size / sizeof(unsigned long);

	for (auto i = 0; i < rdata_section_arr_size; i++)
	{
		auto ssdt_count = rdata_section_arr[i];
		if (ssdt_count < 0x100 || ssdt_count > 0x200)
		{
			continue;
		}

		auto ssdt_base = &rdata_section_arr[i] - ssdt_count;
		if (ssdt_base < rdata_section_arr)
		{
			continue;
		}

		unsigned valid_syscalls_count = 0;
		for (unsigned ssdt_index = 0; ssdt_index < ssdt_count; ssdt_index++)
		{
			auto ssdt_entry = ssdt_base[ssdt_index];

			auto syscall_handler = reinterpret_cast<void*>(reinterpret_cast<char*>(ssdt_base) + (ssdt_entry >> 4));
			if ((syscall_handler < rdata_section_info.address ||
				syscall_handler >= reinterpret_cast<char*>(rdata_section_info.address) + rdata_section_info.size) &&
				syscall_handler >= ntoskrnl_base &&
				syscall_handler < ntoskrnl_end)
			{
				valid_syscalls_count++;
			}
		}

		if (valid_syscalls_count < 4 * ssdt_count / 5)
		{
			continue;
		}

		return ssdt_base;
	}

	return nullptr;*/
}

void* nt_tricks::get_ssdt_end()
{
	auto ntoskrnl_base = get_ntoskrnl_base();

	return reinterpret_cast<char*>(ntoskrnl_base) + 0xc8afc;

	/*auto ntoskrnl_base = get_ntoskrnl_base();
	auto ntoskrnl_end = reinterpret_cast<void*>(reinterpret_cast<char*>(ntoskrnl_base) + get_pe_virtual_size(ntoskrnl_base));

	auto rdata_section_info = get_section_information(ntoskrnl_base, ".rdata");

	auto rdata_section_arr = reinterpret_cast<unsigned long*>(rdata_section_info.address);
	auto rdata_section_arr_size = rdata_section_info.size / sizeof(unsigned long);

	for (auto i = 0; i < rdata_section_arr_size; i++)
	{
		auto ssdt_count = rdata_section_arr[i];
		if (ssdt_count < 0x100 || ssdt_count > 0x200)
		{
			continue;
		}

		auto ssdt_base = &rdata_section_arr[i] - ssdt_count;
		if (ssdt_base < rdata_section_arr)
		{
			continue;
		}

		unsigned valid_syscalls_count = 0;
		for (unsigned ssdt_index = 0; ssdt_index < ssdt_count; ssdt_index++)
		{
			auto ssdt_entry = ssdt_base[ssdt_index];

			auto syscall_handler = reinterpret_cast<void*>(reinterpret_cast<char*>(ssdt_base) + (ssdt_entry >> 4));
			if ((syscall_handler < rdata_section_info.address ||
				syscall_handler >= reinterpret_cast<char*>(rdata_section_info.address) + rdata_section_info.size) &&
				syscall_handler >= ntoskrnl_base &&
				syscall_handler < ntoskrnl_end)
			{
				valid_syscalls_count++;
			}
		}

		if (valid_syscalls_count < 4 * ssdt_count / 5)
		{
			continue;
		}

		return &rdata_section_arr[i];
	}

	return nullptr;*/
}

void* nt_tricks::get_syscall_handler_address(const char* syscall_name)
{
	load_ntdll();

	auto syscall_stub = find_pe_export(ntdll_base, syscall_name);
	auto ssdt_index = *reinterpret_cast<unsigned long*>(reinterpret_cast<char*>(syscall_stub) + 4);

	auto ssdt_base = reinterpret_cast<unsigned long*>(get_ssdt_base());
	auto syscall_handler = reinterpret_cast<unsigned char*>(ssdt_base) + (ssdt_base[ssdt_index] >> 4);

	return syscall_handler;
}

void nt_tricks::load_ntdll()
{
	if (ntdll_base)
	{
		return;
	}

	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;

	PIMAGE_SECTION_HEADER pISH;

	ULONG i, FileSize;
	HANDLE hFile;

	PVOID Buffer, Image;

	IO_STATUS_BLOCK isb;
	OBJECT_ATTRIBUTES oa;

	FILE_STANDARD_INFORMATION FileInfo;
	UNICODE_STRING FileName = RTL_CONSTANT_STRING(L"\\SystemRoot\\System32\\ntdll.dll");

	InitializeObjectAttributes(&oa, &FileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	if (!NT_SUCCESS(::ZwOpenFile(&hFile, FILE_READ_ACCESS | SYNCHRONIZE, &oa, &isb, FILE_SHARE_READ, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT)))
	{
		return;
	}

	if (!NT_SUCCESS(::ZwQueryInformationFile(hFile, &isb, &FileInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation)))
	{
		::ZwClose(hFile);
		return;
	}

	FileSize = FileInfo.EndOfFile.LowPart;
	Buffer = new (NonPagedPool) unsigned char[FileSize];

	if (!Buffer)
	{
		::ZwClose(hFile);
		return;
	}

	if (!NT_SUCCESS(::ZwReadFile(hFile, NULL, NULL, NULL, &isb, Buffer, FileSize, NULL, NULL)))
	{
		delete Buffer;

		::ZwClose(hFile);
		return;
	}

	::ZwClose(hFile);

	pIDH = (PIMAGE_DOS_HEADER)Buffer;
	pINH = (PIMAGE_NT_HEADERS)((PUCHAR)Buffer + pIDH->e_lfanew);

	Image = new (NonPagedPool) unsigned char[pINH->OptionalHeader.SizeOfImage];

	if (!Image)
	{
		delete Buffer;
		return;
	}

	memcpy(Image, Buffer, pINH->OptionalHeader.SizeOfHeaders);
	pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);

	for (i = 0; i < pINH->FileHeader.NumberOfSections; i++)
	{
		memcpy((PUCHAR)Image + pISH[i].VirtualAddress, (PUCHAR)Buffer + pISH[i].PointerToRawData, pISH[i].SizeOfRawData);
	}

	delete Buffer;
	
	ntdll_base = Image;
}
