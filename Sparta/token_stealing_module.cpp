#include "token_stealing_module.h"
#include "vmx.h"
#include "logging.h"

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemPathInformation = 4,
    SystemProcessInformation = 5,
    SystemCallCountInformation = 6,
    SystemDeviceInformation = 7,
    SystemProcessorPerformanceInformation = 8,
    SystemFlagsInformation = 9,
    SystemCallTimeInformation = 10,
    SystemModuleInformation = 11,
    SystemLocksInformation = 12,
    SystemStackTraceInformation = 13,
    SystemPagedPoolInformation = 14,
    SystemNonPagedPoolInformation = 15,
    SystemHandleInformation = 16,
    SystemObjectInformation = 17,
    SystemPageFileInformation = 18,
    SystemVdmInstemulInformation = 19,
    SystemVdmBopInformation = 20,
    SystemFileCacheInformation = 21,
    SystemPoolTagInformation = 22,
    SystemInterruptInformation = 23,
    SystemDpcBehaviorInformation = 24,
    SystemFullMemoryInformation = 25,
    SystemLoadGdiDriverInformation = 26,
    SystemUnloadGdiDriverInformation = 27,
    SystemTimeAdjustmentInformation = 28,
    SystemSummaryMemoryInformation = 29,
    SystemMirrorMemoryInformation = 30,
    SystemPerformanceTraceInformation = 31,
    SystemObsolete0 = 32,
    SystemExceptionInformation = 33,
    SystemCrashDumpStateInformation = 34,
    SystemKernelDebuggerInformation = 35,
    SystemContextSwitchInformation = 36,
    SystemRegistryQuotaInformation = 37,
    SystemExtendServiceTableInformation = 38,
    SystemPrioritySeperation = 39,
    SystemVerifierAddDriverInformation = 40,
    SystemVerifierRemoveDriverInformation = 41,
    SystemProcessorIdleInformation = 42,
    SystemLegacyDriverInformation = 43,
    SystemCurrentTimeZoneInformation = 44,
    SystemLookasideInformation = 45,
    SystemTimeSlipNotification = 46,
    SystemSessionCreate = 47,
    SystemSessionDetach = 48,
    SystemSessionInformation = 49,
    SystemRangeStartInformation = 50,
    SystemVerifierInformation = 51,
    SystemVerifierThunkExtend = 52,
    SystemSessionProcessInformation = 53,
    SystemLoadGdiDriverInSystemSpace = 54,
    SystemNumaProcessorMap = 55,
    SystemPrefetcherInformation = 56,
    SystemExtendedProcessInformation = 57,
    SystemRecommendedSharedDataAlignment = 58,
    SystemComPlusPackage = 59,
    SystemNumaAvailableMemory = 60,
    SystemProcessorPowerInformation = 61,
    SystemEmulationBasicInformation = 62,
    SystemEmulationProcessorInformation = 63,
    SystemExtendedHandleInformation = 64,
    SystemLostDelayedWriteInformation = 65,
    SystemBigPoolInformation = 66,
    SystemSessionPoolTagInformation = 67,
    SystemSessionMappedViewInformation = 68,
    SystemHotpatchInformation = 69,
    SystemObjectSecurityMode = 70,
    SystemWatchdogTimerHandler = 71,
    SystemWatchdogTimerInformation = 72,
    SystemLogicalProcessorInformation = 73,
    SystemWow64SharedInformation = 74,
    SystemRegisterFirmwareTableInformationHandler = 75,
    SystemFirmwareTableInformation = 76,
    SystemModuleInformationEx = 77,
    SystemVerifierTriageInformation = 78,
    SystemSuperfetchInformation = 79,
    SystemMemoryListInformation = 80,
    SystemFileCacheInformationEx = 81,
    MaxSystemInfoClass = 82  // MaxSystemInfoClass should always be the last enum
} SYSTEM_INFORMATION_CLASS;

struct SYSTEM_THREAD_INFORMATION {
    ULONGLONG KernelTime;
    ULONGLONG UserTime;
    ULONGLONG CreateTime;
    ULONG WaitTime;
    // Padding here in 64-bit
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitchCount;
    ULONG State;
    KWAIT_REASON WaitReason;
};

struct SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    ULONGLONG WorkingSetPrivateSize;
    ULONG HardFaultCount;
    ULONG Reserved1;
    ULONGLONG CycleTime;
    ULONGLONG CreateTime;
    ULONGLONG UserTime;
    ULONGLONG KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE ProcessId;
    HANDLE ParentProcessId;
    ULONG HandleCount;
    ULONG Reserved2[2];
    VM_COUNTERS VirtualMemoryCounters;
    size_t Reserved3;
    IO_COUNTERS IoCounters;
    SYSTEM_THREAD_INFORMATION Threads[1];
};

extern "C" NTSTATUS ZwQuerySystemInformation(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

extern "C" const char* NTAPI PsGetProcessImageFileName(_In_ PEPROCESS Process);

const char* TokenStealingModule::SYSTEM_PROCESS_NAMES[TokenStealingModule::SYSTEM_PROCESS_COUNT] = {
    "Idle", "System", "smss.exe", "csrss.exe",
    "wininit.exe", "services.exe", "lsass.exe", "winlogon.exe"
};

TokenStealingModule::TokenStealingModule() :
	_system_process_pids(),
	_system_token(nullptr),
    _should_kill_process(false),
    _pid(0),
    _dpc(),
    _work_item(nullptr)
{
	PEPROCESS system_process = { 0 };
	::PsLookupProcessByProcessId(::ULongToHandle(SYSTEM_PROCESS_PID), &system_process);

	_system_token = ::PsReferencePrimaryToken(system_process);
	::PsDereferencePrimaryToken(_system_token);

    for (auto i = 0; i < SYSTEM_PROCESS_COUNT; i++)
    {
        _system_process_pids[i] = get_pid_by_name(SYSTEM_PROCESS_NAMES[i]);
    }

	::KeRegisterNmiCallback(reinterpret_cast<PNMI_CALLBACK>(handle_nmi), this);
}

void TokenStealingModule::handle_wrmsr(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&) volatile
{
    if (_should_kill_process || _pid != 0)
    {
        return;
    }

	if (static_cast<intel::Msr>(guest_state->rcx) != intel::Msr::IA32_KERNEL_GS_BASE)
	{
		return;
	}

	if (intel::is_um_address(::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE))))
	{
		return;
	}

	auto token = ::PsReferencePrimaryToken(PsGetCurrentProcess());
	auto pid = ::HandleToULong(::PsGetProcessId(::PsGetCurrentProcess()));

	if (token != _system_token || pid == SYSTEM_PROCESS_PID)
	{
		::PsDereferencePrimaryToken(token);
		return;
	}

	for (const auto system_process_pid : _system_process_pids)
	{
		if (pid == system_process_pid)
		{
			::PsDereferencePrimaryToken(token);
			return;
		}
	}

    ::PsDereferencePrimaryToken(token);

    logging::log_to_usermode(logging::ThreatType::SYSTEM_TOKEN_STEALING, pid);

    /*_should_kill_process = true;
    _pid = pid;

	vmx::inject_nmi();*/
}

bool TokenStealingModule::handle_nmi(void* context, bool)
{
    auto _this = reinterpret_cast<TokenStealingModule*>(context);
	if (!_this->_should_kill_process)
	{
		return false;
	}

    ::KeInitializeDpc(
        &_this->_dpc,
        handle_dpc,
        _this
    );

    ::KeInsertQueueDpc(
        &_this->_dpc,
        nullptr,
        nullptr
    );

    _this->_should_kill_process = false;

    return true;
}

extern PDEVICE_OBJECT g_device_object;

void TokenStealingModule::handle_dpc(KDPC*, void* context, void*, void*)
{
    auto _this = reinterpret_cast<TokenStealingModule*>(context);

    _this->_work_item = ::IoAllocateWorkItem(g_device_object);
    ::IoInitializeWorkItem(g_device_object, _this->_work_item);

    ::IoQueueWorkItem(
        _this->_work_item,
        handle_work_item,
        DelayedWorkQueue,
        _this
    );
}

void TokenStealingModule::handle_work_item(PDEVICE_OBJECT, void* context)
{
    auto _this = reinterpret_cast<TokenStealingModule*>(context);

    PEPROCESS process;
    ::PsLookupProcessByProcessId(
        ::ULongToHandle(_this->_pid),
        &process
    );

    ::KeAttachProcess(process);

    ::ZwTerminateProcess(
        nullptr,
        1
    );

    ::KeDetachProcess();

    ::IoUninitializeWorkItem(_this->_work_item);
    ::IoFreeWorkItem(_this->_work_item);

    _this->_pid = 0;
    _this->_work_item = nullptr;
}

unsigned long TokenStealingModule::get_pid_by_name(const char* process_name)
{
    unsigned long buffer_length = 0;
    auto status = ::ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &buffer_length);

    if (status != STATUS_INFO_LENGTH_MISMATCH)
    {
        return 0;
    }

    auto buffer = new (NonPagedPool) unsigned char[buffer_length];
    status = ::ZwQuerySystemInformation(SystemProcessInformation, buffer, buffer_length, &buffer_length);

    if (!NT_SUCCESS(status))
    {
        return 0;
    }

    SYSTEM_PROCESS_INFORMATION* current_process_entry = nullptr;
    for (unsigned i = 0; !current_process_entry || current_process_entry->NextEntryOffset != 0; i += current_process_entry->NextEntryOffset)
    {
        current_process_entry = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(&buffer[i]);
        
        PEPROCESS eprocess = nullptr;
        status = ::PsLookupProcessByProcessId(current_process_entry->ProcessId, &eprocess);

        if (!NT_SUCCESS(status))
        {
            continue;
        }

        auto name = ::PsGetProcessImageFileName(eprocess);
        if (::strcmp(name, process_name) != 0)
        {
            ::ObDereferenceObject(eprocess);
            continue;
        }

        auto pid = ::HandleToULong(current_process_entry->ProcessId);
        delete buffer;

        ::ObDereferenceObject(eprocess);
        return pid;
    }

    delete buffer;

	return 0;
}
