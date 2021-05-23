#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <md5.h>

#include "httplib.h"
#include "device.h"
#include "json11.h"

#define CBASE64_IMPLEMENTATION
#include "cbase64.h"

#include <windows.h>
#include <psapi.h>
#include <minidumpapiset.h>
#include <tlhelp32.h>

typedef long NTSTATUS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS       ((NTSTATUS)0x00000000L)
#endif

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,                         // 0x00 SYSTEM_BASIC_INFORMATION
	SystemProcessorInformation,                     // 0x01 SYSTEM_PROCESSOR_INFORMATION
	SystemPerformanceInformation,                   // 0x02
	SystemTimeOfDayInformation,                     // 0x03
	SystemPathInformation,                          // 0x04
	SystemProcessInformation,                       // 0x05
	SystemCallCountInformation,                     // 0x06
	SystemDeviceInformation,                        // 0x07
	SystemProcessorPerformanceInformation,          // 0x08
	SystemFlagsInformation,                         // 0x09
	SystemCallTimeInformation,                      // 0x0A
	SystemModuleInformation,                        // 0x0B SYSTEM_MODULE_INFORMATION
	SystemLocksInformation,                         // 0x0C
	SystemStackTraceInformation,                    // 0x0D
	SystemPagedPoolInformation,                     // 0x0E
	SystemNonPagedPoolInformation,                  // 0x0F
	SystemHandleInformation,                        // 0x10
	SystemObjectInformation,                        // 0x11
	SystemPageFileInformation,                      // 0x12
	SystemVdmInstemulInformation,                   // 0x13
	SystemVdmBopInformation,                        // 0x14
	SystemFileCacheInformation,                     // 0x15
	SystemPoolTagInformation,                       // 0x16
	SystemInterruptInformation,                     // 0x17
	SystemDpcBehaviorInformation,                   // 0x18
	SystemFullMemoryInformation,                    // 0x19
	SystemLoadGdiDriverInformation,                 // 0x1A
	SystemUnloadGdiDriverInformation,               // 0x1B
	SystemTimeAdjustmentInformation,                // 0x1C
	SystemSummaryMemoryInformation,                 // 0x1D
	SystemMirrorMemoryInformation,                  // 0x1E
	SystemPerformanceTraceInformation,              // 0x1F
	SystemObsolete0,                                // 0x20
	SystemExceptionInformation,                     // 0x21
	SystemCrashDumpStateInformation,                // 0x22
	SystemKernelDebuggerInformation,                // 0x23
	SystemContextSwitchInformation,                 // 0x24
	SystemRegistryQuotaInformation,                 // 0x25
	SystemExtendServiceTableInformation,            // 0x26
	SystemPrioritySeperation,                       // 0x27
	SystemPlugPlayBusInformation,                   // 0x28
	SystemDockInformation,                          // 0x29
	SystemPowerInformationNative,                   // 0x2A
	SystemProcessorSpeedInformation,                // 0x2B
	SystemCurrentTimeZoneInformation,               // 0x2C
	SystemLookasideInformation,                     // 0x2D
	SystemTimeSlipNotification,                     // 0x2E
	SystemSessionCreate,                            // 0x2F
	SystemSessionDetach,                            // 0x30
	SystemSessionInformation,                       // 0x31
	SystemRangeStartInformation,                    // 0x32
	SystemVerifierInformation,                      // 0x33
	SystemAddVerifier,                              // 0x34
	SystemSessionProcessesInformation,              // 0x35
	SystemLoadGdiDriverInSystemSpaceInformation,    // 0x36
	SystemNumaProcessorMap,                         // 0x37
	SystemPrefetcherInformation,                    // 0x38
	SystemExtendedProcessInformation,               // 0x39
	SystemRecommendedSharedDataAlignment,           // 0x3A
	SystemComPlusPackage,                           // 0x3B
	SystemNumaAvailableMemory,                      // 0x3C
	SystemProcessorPowerInformation,                // 0x3D
	SystemEmulationBasicInformation,                // 0x3E
	SystemEmulationProcessorInformation,            // 0x3F
	SystemExtendedHanfleInformation,                // 0x40
	SystemLostDelayedWriteInformation,              // 0x41
	SystemBigPoolInformation,                       // 0x42
	SystemSessionPoolTagInformation,                // 0x43
	SystemSessionMappedViewInformation,             // 0x44
	SystemHotpatchInformation,                      // 0x45
	SystemObjectSecurityMode,                       // 0x46
	SystemWatchDogTimerHandler,                     // 0x47
	SystemWatchDogTimerInformation,                 // 0x48
	SystemLogicalProcessorInformation,              // 0x49
	SystemWo64SharedInformationObosolete,           // 0x4A
	SystemRegisterFirmwareTableInformationHandler,  // 0x4B
	SystemFirmwareTableInformation,                 // 0x4C
	SystemModuleInformationEx,                      // 0x4D
	SystemVerifierTriageInformation,                // 0x4E
	SystemSuperfetchInformation,                    // 0x4F
	SystemMemoryListInformation,                    // 0x50
	SystemFileCacheInformationEx,                   // 0x51
	SystemThreadPriorityClientIdInformation,        // 0x52
	SystemProcessorIdleCycleTimeInformation,        // 0x53
	SystemVerifierCancellationInformation,          // 0x54
	SystemProcessorPowerInformationEx,              // 0x55
	SystemRefTraceInformation,                      // 0x56
	SystemSpecialPoolInformation,                   // 0x57
	SystemProcessIdInformation,                     // 0x58
	SystemErrorPortInformation,                     // 0x59
	SystemBootEnvironmentInformation,               // 0x5A SYSTEM_BOOT_ENVIRONMENT_INFORMATION
	SystemHypervisorInformation,                    // 0x5B
	SystemVerifierInformationEx,                    // 0x5C
	SystemTimeZoneInformation,                      // 0x5D
	SystemImageFileExecutionOptionsInformation,     // 0x5E
	SystemCoverageInformation,                      // 0x5F
	SystemPrefetchPathInformation,                  // 0x60
	SystemVerifierFaultsInformation,                // 0x61
	MaxSystemInfoClass                              // 0x67
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_MODULE
{
	HANDLE Section;                 // Not filled in
	PVOID  MappedBase;
	PVOID  ImageBase;
	ULONG  ImageSize;
	ULONG  Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	CHAR   ImageName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG         ModulesCount;
	SYSTEM_MODULE Modules[1];

} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

extern "C" NTSTATUS WINAPI NtQuerySystemInformation(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

#define SPARTA_DEVICE 0x8000

#define IOCTL_SPARTA_CONFIGURE_LOAD_LIBRARY CTL_CODE( \
	SPARTA_DEVICE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPARTA_DUMP_KERNEL_MEMORY CTL_CODE( \
	SPARTA_DEVICE, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

static unsigned long last_privesc_pid = -1;

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

enum class ThreatType
{
	SYSTEM_TOKEN_STEALING,
	DLL_INJECTION,
	APC_INJECTION,
	REMOTE_THREAD_CREATION,
	KERNEL_STRUCTURE_CORRUPTION,
	SMEP_BYPASS,
};

#pragma pack(push, 1)
struct ThreatReport
{
	ThreatType type;
	unsigned long long id;
};
#pragma pack(pop)

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream token_stream(s);
	while (std::getline(token_stream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

std::string get_process_path(HANDLE process_handle)
{
	std::string raw_process_path;
	raw_process_path.resize(MAX_PATH);

	auto ret = ::GetModuleFileNameExA(
		process_handle,
		nullptr,
		const_cast<char*>(raw_process_path.c_str()),
		MAX_PATH
	);

	if (ret == 0)
	{
		throw;
	}

	return std::string(raw_process_path.c_str());
}

std::string get_process_name(HANDLE process_handle)
{
	auto splitted_path = split(get_process_path(process_handle), '\\');
	return splitted_path[splitted_path.size() - 1];
}

std::string get_module_path(unsigned long long address)
{
	ULONG size;
	::NtQuerySystemInformation(SystemModuleInformation, nullptr, 0, &size);

	auto raw_module_information = std::make_unique<unsigned char[]>(size);
	auto module_information = reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(raw_module_information.get());

	auto status = ::NtQuerySystemInformation(
		SystemModuleInformation,
		module_information,
		size,
		&size
	);
	if (NT_SUCCESS(status))
	{
		for (auto i = 0; i < module_information->ModulesCount; i++)
		{
			const auto& module = module_information->Modules[i];
			
			auto module_start = reinterpret_cast<unsigned long long>(module.ImageBase);
			auto module_end = reinterpret_cast<unsigned long long>(module.ImageBase) + module.ImageSize;

			if (address < module_start || address >= module_end)
			{
				continue;
			}

			return module.ImageName;
		}
	}

	return "";
}

std::pair<void*, size_t> get_module_info(unsigned long long address)
{
	ULONG size;
	::NtQuerySystemInformation(SystemModuleInformation, nullptr, 0, &size);

	auto raw_module_information = std::make_unique<unsigned char[]>(size);
	auto module_information = reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(raw_module_information.get());

	auto status = ::NtQuerySystemInformation(
		SystemModuleInformation,
		module_information,
		size,
		&size
	);
	if (NT_SUCCESS(status))
	{
		for (auto i = 0; i < module_information->ModulesCount; i++)
		{
			const auto& module = module_information->Modules[i];

			auto module_start = reinterpret_cast<unsigned long long>(module.ImageBase);
			auto module_end = reinterpret_cast<unsigned long long>(module.ImageBase) + module.ImageSize;

			if (address < module_start || address >= module_end)
			{
				continue;
			}

			return { module.ImageBase, module.ImageSize };
		}
	}

	return { 0, 0 };
}

std::string get_module_name(unsigned long long address)
{
	auto splitted_path = split(get_module_path(address), '\\');
	return splitted_path[splitted_path.size() - 1];
}

std::string get_threat_description(ThreatType type)
{
	switch (type)
	{
	case ThreatType::SYSTEM_TOKEN_STEALING: return "A SYSTEM token stealing attempt was detected and blocked by Sparta.";
	case ThreatType::DLL_INJECTION: return "A DLL injection attempt was detected and blocked by Sparta.";
	case ThreatType::APC_INJECTION: return "An APC injection attempt was detected and blocked by Sparta.";
	case ThreatType::REMOTE_THREAD_CREATION: return "A remote thread creation attempt was detected by Sparta.";
	case ThreatType::KERNEL_STRUCTURE_CORRUPTION: return "A kernel structure corruption attempt was detected and blocked by Sparta.";
	case ThreatType::SMEP_BYPASS: return "An SMEP bypass attempt was detected and blocked by Sparta.";
	}

	throw;
}

std::string get_threat_hash(const std::string& path)
{
	Chocobo1::MD5 hash;

	size_t bytes;
	unsigned char data[1024] = { 0 };

	FILE* file;
	fopen_s(&file, path.c_str(), "rb");

	while ((bytes = fread(data, 1, 1024, file)) != 0)
	{
		hash.addData(data, bytes);
	}

	fclose(file);

	hash.finalize();
	return hash.toString();
}

std::string get_threat_type_string(ThreatType type)
{
	switch (type)
	{
	case ThreatType::SYSTEM_TOKEN_STEALING: return "systemTokenStealing";
	case ThreatType::DLL_INJECTION: return "dllInjection";
	case ThreatType::APC_INJECTION: return "apcInjection";
	case ThreatType::REMOTE_THREAD_CREATION: return "remoteThreadCreation";
	case ThreatType::KERNEL_STRUCTURE_CORRUPTION: return "kernelStructureCorruption";
	case ThreatType::SMEP_BYPASS: return "smepBypass";
	}

	throw;
}

std::string get_threat_dump(HANDLE process_handle)
{
	char temp_path_buffer[MAX_PATH];
	::GetTempPathA(MAX_PATH, temp_path_buffer);

	char temp_file_name[MAX_PATH];
	::GetTempFileNameA(temp_path_buffer, "SPARTA", 0, temp_file_name);

	auto file_handle = ::CreateFileA(
		temp_file_name,
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	::MiniDumpWriteDump(
		process_handle,
		::GetProcessId(process_handle),
		file_handle,
		MiniDumpWithFullMemoryInfo,
		nullptr,
		nullptr,
		nullptr
	);

	::SetFilePointer(
		file_handle,
		0,
		nullptr,
		FILE_BEGIN
	);

	auto file_size = ::GetFileSize(
		file_handle,
		nullptr
	);

	auto buffer = new unsigned char[file_size];

	DWORD bytes_read;
	::ReadFile(
		file_handle,
		buffer,
		file_size,
		&bytes_read,
		nullptr
	);

	::CloseHandle(file_handle);

	auto encoded_length = cbase64_calc_encoded_length(file_size);
	auto encoded_buffer = new char[encoded_length];

	cbase64_encodestate encode_state;
	cbase64_init_encodestate(&encode_state);

	auto final_length = cbase64_encode_block(buffer, file_size, encoded_buffer, &encode_state);
	cbase64_encode_blockend(&encoded_buffer[final_length], &encode_state);

	return encoded_buffer;
}

std::string get_threat_dump(Device& device, void* address, size_t size)
{
	auto kernel_dump = std::make_unique<unsigned char[]>(size);
	unsigned long bytes_read = 0;

	DumpKernelMemoryParameters parameters = {
		address,
		size
	};

	bool succeeded = device.ioctl(
		IOCTL_SPARTA_DUMP_KERNEL_MEMORY,
		&parameters,
		sizeof(DumpKernelMemoryParameters),
		kernel_dump.get(),
		size,
		&bytes_read
	);

	auto encoded_length = cbase64_calc_encoded_length(bytes_read);
	auto encoded_buffer = new char[encoded_length];

	cbase64_encodestate encode_state;
	cbase64_init_encodestate(&encode_state);

	auto final_length = cbase64_encode_block(kernel_dump.get(), bytes_read, encoded_buffer, &encode_state);
	cbase64_encode_blockend(&encoded_buffer[final_length], &encode_state);

	return encoded_buffer;
}

unsigned long get_parent_pid(unsigned long pid)
{
	auto snapshot_handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32W pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32W);

	if (::Process32FirstW(snapshot_handle, &pe)) {
		do {
			if (pe.th32ProcessID == pid) {
				CloseHandle(snapshot_handle);
				return pe.th32ParentProcessID;
			}
		} while (::Process32NextW(snapshot_handle, &pe));
	}

	CloseHandle(snapshot_handle);
	return 0;
}

void report_threat(const std::string& name, const std::string& description, const std::string& hash, const std::string& type, const std::string& dump)
{
	std::cout << "[*] " << name << " - " << description << std::endl;

	httplib::Client cli("http://SpartaServer:3000");
	json11::Json request_json = json11::Json::object{
		{ "name", name.c_str() },
		{ "description", description.c_str() },
		{ "hash", hash.c_str() },
		{ "type", type.c_str() },
		{ "dump", dump.c_str() },
	};
	cli.Post("/api/threats", request_json.dump(), "application/json");
}

void handle_kernel_threat(Device& device, unsigned long long address, ThreatType type)
{
	auto name = get_module_name(address);
	auto description = get_threat_description(type);
	auto hash = get_threat_hash(get_module_path(address));
	auto type_str = get_threat_type_string(type);
	auto module_info = get_module_info(address);
	auto dump = get_threat_dump(device, module_info.first, module_info.second == 0x8000 ? 0x6000 : (module_info.second == 0x7000 ? 0x5000 : module_info.second));

	report_threat(name, description, hash, type_str, dump);
}

void handle_threat(Device& device, unsigned long long pid, ThreatType type)
{
	if (type == ThreatType::KERNEL_STRUCTURE_CORRUPTION || type == ThreatType::SMEP_BYPASS)
	{
		return handle_kernel_threat(device, pid, type);
	}

	if (pid == last_privesc_pid && type == ThreatType::SYSTEM_TOKEN_STEALING)
	{
		last_privesc_pid = -1;
		return;
	}

	auto handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!handle)
	{
		return;
	}

	if (type == ThreatType::SYSTEM_TOKEN_STEALING)
	{
		last_privesc_pid = pid;
	}

	auto name = get_process_name(handle);

	if (name == "cmd.exe" && type == ThreatType::SYSTEM_TOKEN_STEALING)
	{
		unsigned long parent_pid = get_parent_pid(pid);
		::TerminateProcess(handle, 1);
		::CloseHandle(handle);
		handle_threat(device, parent_pid, type);
		last_privesc_pid = pid;
		return;
	}

	auto description = get_threat_description(type);
	auto hash = get_threat_hash(get_process_path(handle));
	auto type_str = get_threat_type_string(type);
	auto dump = get_threat_dump(handle);

	if (type == ThreatType::SYSTEM_TOKEN_STEALING)
	{
		::TerminateProcess(handle, 1);
	}

	::CloseHandle(handle);

	report_threat(name, description, hash, type_str, dump);
}

int main(int argc, char* argv[])
{
	try
	{
		Device device(L"\\\\.\\Sparta");

		ConfigureLoadLibraryParameters parameters = {
			&LoadLibraryA,
			&LoadLibraryW
		};

		bool succeeded = device.ioctl(
			IOCTL_SPARTA_CONFIGURE_LOAD_LIBRARY,
			&parameters,
			sizeof(ConfigureLoadLibraryParameters),
			nullptr,
			0
		);

		if (!succeeded)
		{
			std::cout << "[-] could not configure sparta's load library" << std::endl;
			return 1;
		}

		std::cout << "[+] successfully configured sparta's load library" << std::endl;

		ThreatReport report_buffer[16] = { };
		DWORD bytes_read;

		while (true)
		{
			device.read(report_buffer, sizeof(report_buffer), bytes_read);

			for (auto i = 0; i < bytes_read / sizeof(ThreatReport); i++)
			{
				try
				{
					handle_threat(device, report_buffer[i].id, report_buffer[i].type);
				} catch(...) { }
			}

			std::memset(report_buffer, 0, sizeof(report_buffer));

			::Sleep(1000);
		}

		return 0;
	}
	catch (std::exception& e)
	{
		std::cout << "[-] " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "[-] unknown error" << std::endl;
		return 1;
	}
}
