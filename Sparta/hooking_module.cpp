#pragma warning(disable: 4238)
#pragma warning(disable: 4100)

#include "hooking_module.h"

#include "ept.h"
#include "asm_helpers.h"
#include "nt_tricks.h"
#include "logging.h"

#define NMD_ASSEMBLY_IMPLEMENTATION
#define NMD_ASSEMBLY_DEFINE_INT_TYPES
#include "nmd_assembly.h"

extern "C"
NTKERNELAPI
BOOLEAN
NTAPI
PsIsProtectedProcess(
	_In_ PEPROCESS Process
);

using _NtCreateProcessEx = NTSTATUS(*)(
	HANDLE ProcessHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	HANDLE ParentProcess,
	BOOLEAN InheritObjectTable,
	HANDLE SectionHandle,
	HANDLE DebugPort,
	HANDLE ExceptionPort,
	BOOLEAN InJob
);

using _NtCreateThreadEx = NTSTATUS (*)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	HANDLE ProcessHandle,
	PVOID StartRoutine,
	PVOID Argument,
	ULONG CreateFlags,
	SIZE_T ZeroBits,
	SIZE_T StackSize,
	SIZE_T MaximumStackSize,
	PVOID AttributeList
);

using _NtQueueApcThreadEx = NTSTATUS (*)(
	HANDLE ThreadHandle,
	HANDLE MemoryReserveHandle,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	PVOID SystemArgument3
);

void* LoadLibraryA = nullptr;
void* LoadLibraryW = nullptr;

static constexpr auto STSTEM_PROCESS_ID = 4ul;

NTSTATUS NtCreateProcessExHook(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ParentProcess, BOOLEAN InheritObjectTable, HANDLE SectionHandle, HANDLE DebugPort, HANDLE ExceptionPort, BOOLEAN InJob)
{
	// logging::log_to_usermode("[*] NtCreateProcessEx was called.\n");

	auto original_function = reinterpret_cast<_NtCreateProcessEx>(HookingModule::get_syscall_hook_entry("NtCreateProcessEx")->original_function);
	return original_function(ProcessHandle, DesiredAccess, ObjectAttributes, ParentProcess, InheritObjectTable, SectionHandle, DebugPort, ExceptionPort, InJob);
}

NTSTATUS NtCreateThreadExHook(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PVOID StartRoutine, PVOID Argument, ULONG CreateFlags, SIZE_T ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList)
{
	// KdPrint(("[*] NtCreateThreadEx was called!\n"));

	PEPROCESS eprocess = nullptr;
	auto status = ::ObReferenceObjectByHandle(
		ProcessHandle,
		DesiredAccess,
		*PsProcessType,
		KernelMode,
		reinterpret_cast<PVOID*>(&eprocess),
		nullptr
	);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (::PsGetCurrentProcessId() != ::PsGetProcessId(eprocess) &&
		::PsGetCurrentProcessId() != ::ULongToHandle(STSTEM_PROCESS_ID) &&
		!::PsIsProtectedProcess(::PsGetCurrentProcess()))
	{
		logging::log_to_usermode(logging::ThreatType::REMOTE_THREAD_CREATION, ::HandleToULong(::PsGetCurrentProcessId()));
	}

	::ObDereferenceObject(eprocess);

	if (StartRoutine == LoadLibraryA || StartRoutine == LoadLibraryW)
	{
		logging::log_to_usermode(logging::ThreatType::DLL_INJECTION, ::HandleToULong(::PsGetCurrentProcessId()));
		return STATUS_ACCESS_DENIED;
	}

	auto original_function = reinterpret_cast<_NtCreateThreadEx>(HookingModule::get_syscall_hook_entry("NtCreateThreadEx")->original_function);
	return original_function(ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle, StartRoutine, Argument, CreateFlags, ZeroBits, StackSize, MaximumStackSize, AttributeList);
}

NTSTATUS NtQueueApcThreadExHook(HANDLE ThreadHandle, HANDLE MemoryReserveHandle, PIO_APC_ROUTINE ApcRoutine, PVOID SystemArgument1, PVOID SystemArgument2, PVOID SystemArgument3)
{
	// logging::log_to_usermode("[*] NtQueueApcThreadHookEx was called.\n");

	/*PETHREAD ethread = nullptr;
	auto status = ::ObReferenceObjectByHandle(
		ThreadHandle,
		STANDARD_RIGHTS_ALL,
		*PsThreadType,
		KernelMode,
		reinterpret_cast<PVOID*>(&ethread),
		nullptr
	);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if (::PsGetCurrentProcessId() != ::PsGetThreadProcessId(ethread) &&
		::PsGetCurrentProcessId() != ::ULongToHandle(STSTEM_PROCESS_ID) &&
		!::PsIsProtectedProcess(::PsGetCurrentProcess()))
	{
		logging::log_to_usermode("[*] remote apc queue detected.\n");
	}

	::ObDereferenceObject(ethread);*/

	if (ApcRoutine == LoadLibraryA || ApcRoutine == LoadLibraryW ||
		SystemArgument1 == LoadLibraryA || SystemArgument1 == LoadLibraryW)
	{
		logging::log_to_usermode(logging::ThreatType::APC_INJECTION, ::HandleToULong(::PsGetCurrentProcessId()));
		return STATUS_ACCESS_DENIED;
	}

	auto original_function = reinterpret_cast<_NtQueueApcThreadEx>(HookingModule::get_syscall_hook_entry("NtQueueApcThreadEx")->original_function);
	return original_function(ThreadHandle, MemoryReserveHandle, ApcRoutine, SystemArgument1, SystemArgument2, SystemArgument3);
}

const HookingModule::SyscallHookRegistration HookingModule::SYSCALL_HOOK_REGISTRATIONS[] = {
	/*{
		"NtCreateProcessEx",
		&NtCreateProcessExHook
	},*/
	{
		"NtCreateThreadEx",
		&NtCreateThreadExHook
	},
	{
		"NtQueueApcThreadEx",
		&NtQueueApcThreadExHook
	}
};

HookingModule::SyscallHookEntry* HookingModule::_hooks[HookingModule::MAX_HOOKS];

void HookingModule::initialize(VcpuContext* vcpu_context) volatile
{
	for (const auto& [name, hook] : SYSCALL_HOOK_REGISTRATIONS)
	{
		SyscallHookEntry syscall_hook_entry = { 0 };
		syscall_hook_entry.name = name;

		auto hooked_function = nt_tricks::get_syscall_handler_address(name);

		syscall_hook_entry.original_virtual_page = reinterpret_cast<void*>(reinterpret_cast<unsigned long long>(hooked_function) & 0xfffffffffffff000);
		syscall_hook_entry.guest_physical_page = ::MmGetPhysicalAddress(syscall_hook_entry.original_virtual_page).QuadPart;

		syscall_hook_entry.hooked_virtual_page = new (NonPagedPool) unsigned char[intel::RAW_PAGE_SIZE];
		::memcpy(syscall_hook_entry.hooked_virtual_page, syscall_hook_entry.original_virtual_page, intel::RAW_PAGE_SIZE);

		auto trampoline = reinterpret_cast<unsigned char*>(syscall_hook_entry.hooked_virtual_page) +
			reinterpret_cast<unsigned long long>(hooked_function) % intel::RAW_PAGE_SIZE;

		size_t overriden_length = 0;
		while (overriden_length < sizeof(JMP_PATCH))
			overriden_length += nmd_x86_ldisasm(&trampoline[overriden_length], intel::RAW_PAGE_SIZE, NMD_X86_MODE_64);

		::memcpy(trampoline, JMP_PATCH, sizeof(JMP_PATCH));
		*reinterpret_cast<void**>(&trampoline[2]) = hook;

		syscall_hook_entry.original_function = new (NonPagedPool) unsigned char[overriden_length + sizeof(JMP_PATCH)];
		::memcpy(syscall_hook_entry.original_function, hooked_function, overriden_length);
		::memcpy(&reinterpret_cast<unsigned char*>(syscall_hook_entry.original_function)[overriden_length], JMP_PATCH, sizeof(JMP_PATCH));
		*reinterpret_cast<void**>(&reinterpret_cast<unsigned char*>(syscall_hook_entry.original_function)[overriden_length + 2]) =
			&reinterpret_cast<unsigned char*>(hooked_function)[overriden_length];

		auto pt = ept::split_large_page(&vcpu_context->pd[(syscall_hook_entry.guest_physical_page >> 21) / intel::EPT_ENTRY_COUNT][(syscall_hook_entry.guest_physical_page >> 21) % intel::EPT_ENTRY_COUNT]);
		syscall_hook_entry.pte = &pt[(syscall_hook_entry.guest_physical_page >> 12) % intel::EPT_ENTRY_COUNT];

		syscall_hook_entry.pte->read = false;
		syscall_hook_entry.pte->write = false;

		syscall_hook_entry.pte->pfn = ::MmGetPhysicalAddress(syscall_hook_entry.hooked_virtual_page).QuadPart >> 12;

		insert_syscall_hook_entry(syscall_hook_entry);
	}

	asm_helpers::invept();
}

void HookingModule::handle_mtf(VcpuContext*, sparta::VmExitGuestState*, bool& increment_rip) volatile
{
	if (!_hook_to_restore)
	{
		return;
	}

	_hook_to_restore->pte->read = false;
	_hook_to_restore->pte->write = false;

	_hook_to_restore->pte->supervisor_mode_execute = true;

	_hook_to_restore->pte->pfn = ::MmGetPhysicalAddress(_hook_to_restore->hooked_virtual_page).QuadPart >> 12;

	asm_helpers::invept();

	vmx::turn_mtf_off();
	_hook_to_restore = nullptr;

	increment_rip = false;
}

void HookingModule::handle_ept_violation(VcpuContext*, sparta::VmExitGuestState*, bool& increment_rip) volatile
{
	auto guest_physical_page = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PHYSICAL_ADDRESS).value & 0xfffffffffffff000;
	auto syscall_hook_entry = get_syscall_hook_entry(guest_physical_page);

	if (!syscall_hook_entry)
	{
		return;
	}

	syscall_hook_entry->pte->read = true;
	syscall_hook_entry->pte->write = true;

	syscall_hook_entry->pte->supervisor_mode_execute = false;

	syscall_hook_entry->pte->pfn = ::MmGetPhysicalAddress(syscall_hook_entry->original_virtual_page).QuadPart >> 12;

	asm_helpers::invept();

	vmx::turn_mtf_on();
	_hook_to_restore = syscall_hook_entry;

	increment_rip = false;
}

bool HookingModule::insert_syscall_hook_entry(const SyscallHookEntry& syscall_hook_entry)
{
	for (auto& hook : _hooks)
	{
		if (hook != nullptr)
		{
			continue;
		}

		hook = new (NonPagedPool) SyscallHookEntry(syscall_hook_entry);
		return true;
	}

	return false;
}

HookingModule::SyscallHookEntry* HookingModule::get_syscall_hook_entry(unsigned long long guest_physical_page)
{
	for (const auto hook : _hooks)
	{
		if (!hook)
		{
			continue;
		}

		if (hook->guest_physical_page != guest_physical_page)
		{
			continue;
		}

		return hook;
	}

	return nullptr;
}

HookingModule::SyscallHookEntry* HookingModule::get_syscall_hook_entry(const char* name)
{
	for (const auto hook : _hooks)
	{
		if (!hook)
		{
			continue;
		}

		if (::strcmp(hook->name, name) != 0)
		{
			continue;
		}

		return hook;
	}

	return nullptr;
}
