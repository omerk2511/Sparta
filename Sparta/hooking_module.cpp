#include "hooking_module.h"

#include "ept.h"
#include "asm_helpers.h"
#include "ldisasm.h"

using ptr_old_function = NTSTATUS (*)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer,
	ULONG EaLength
);

static ptr_old_function old_function = nullptr;

NTSTATUS hook(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer,
	ULONG EaLength
)
{
	KdPrint(("[*] NtCreateFile was called!\n"));
	return old_function(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength
	);
}

void HookingModule::initialize(VcpuContext* vcpu_context) volatile
{
	auto hooked_function = reinterpret_cast<void*>(&NtCreateFile);

	_original_virtual_page = reinterpret_cast<void*>(reinterpret_cast<unsigned long long>(hooked_function) & 0xfffffffffffff000);
	_guest_physical_page = ::MmGetPhysicalAddress(_original_virtual_page).QuadPart;

	_hooked_virtual_page = new (NonPagedPool) unsigned char[intel::RAW_PAGE_SIZE];
	::memcpy(_hooked_virtual_page, _original_virtual_page, intel::RAW_PAGE_SIZE);

	auto trampoline = reinterpret_cast<unsigned char*>(_hooked_virtual_page) +
		reinterpret_cast<unsigned long long>(hooked_function) % intel::RAW_PAGE_SIZE;

	size_t overriden_length = 0;
	while (overriden_length < sizeof(JMP_PATCH))
		overriden_length += ldisasm::get_instruction_length(&trampoline[overriden_length], ldisasm::DisassmeblyMode::X86_64);

	::memcpy(trampoline, JMP_PATCH, sizeof(JMP_PATCH));
	*reinterpret_cast<void**>(&trampoline[3]) = &hook;

	old_function = reinterpret_cast<ptr_old_function>(new (NonPagedPool) unsigned char[overriden_length + sizeof(JMP_PATCH)]);
	::memcpy(old_function, hooked_function, overriden_length);
	::memcpy(&reinterpret_cast<unsigned char*>(old_function)[overriden_length], JMP_PATCH, sizeof(JMP_PATCH));
	*reinterpret_cast<void**>(&reinterpret_cast<unsigned char*>(old_function)[overriden_length + 3]) =
		&reinterpret_cast<unsigned char*>(hooked_function)[overriden_length];

	auto pt = ept::split_large_page(&vcpu_context->pd[(_guest_physical_page >> 21) / intel::EPT_ENTRY_COUNT][(_guest_physical_page >> 21) % intel::EPT_ENTRY_COUNT]);
	_pte = &pt[(_guest_physical_page >> 12) % intel::EPT_ENTRY_COUNT];

	_pte->read = false;
	_pte->write = false;

	_pte->pfn = ::MmGetPhysicalAddress(_hooked_virtual_page).QuadPart >> 12;

	asm_helpers::invept();
}

void HookingModule::handle_mtf(VcpuContext*, sparta::VmExitGuestState*, bool& increment_rip) volatile
{
	if (!_restore_page)
	{
		return;
	}

	_pte->read = false;
	_pte->write = false;

	_pte->supervisor_mode_execute = true;

	_pte->pfn = ::MmGetPhysicalAddress(_hooked_virtual_page).QuadPart >> 12;

	asm_helpers::invept();

	vmx::turn_mtf_off();
	_restore_page = false;

	increment_rip = false;
}

void HookingModule::handle_ept_violation(VcpuContext*, sparta::VmExitGuestState*, bool& increment_rip) volatile
{
	auto guest_physical_page = vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PHYSICAL_ADDRESS).value & 0xfffffffffffff000;
	if (guest_physical_page != _guest_physical_page)
	{
		return;
	}

	_pte->read = true;
	_pte->write = true;

	_pte->supervisor_mode_execute = false;

	_pte->pfn = ::MmGetPhysicalAddress(_original_virtual_page).QuadPart >> 12;

	asm_helpers::invept();

	vmx::turn_mtf_on();
	_restore_page = true;

	increment_rip = false;
}
