#pragma once

#include "base_module.h"

class HookingModule final : public sparta::BaseModule
{
public:
	virtual void initialize(VcpuContext* vcpu_context) volatile override;

protected:
	virtual void handle_mtf(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;
	virtual void handle_ept_violation(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;

private:
	struct SyscallHookRegistration
	{
		const char* name;
		void* hook;
	};

	struct SyscallHookEntry
	{
		const char* name;
		void* hooked_virtual_page;
		void* original_virtual_page;
		void* original_function;
		unsigned long long guest_physical_page;
		intel::EptPte* pte;
	};

public:
	static bool insert_syscall_hook_entry(const SyscallHookEntry& syscall_hook_entry);
	static SyscallHookEntry* get_syscall_hook_entry(unsigned long long guest_physical_page);
	static SyscallHookEntry* get_syscall_hook_entry(const char* name);

private:
	static constexpr size_t MAX_HOOKS = 64;
	static SyscallHookEntry* _hooks[MAX_HOOKS];

	SyscallHookEntry* _hook_to_restore;

private:
	static const SyscallHookRegistration SYSCALL_HOOK_REGISTRATIONS[];
	static constexpr unsigned char JMP_PATCH[] = "\x48\xB8\x78\x56\x34\x12\x78\x56\x34\x12\xFF\xE0";
};
