#pragma once

namespace logging
{
	void dump_vmcs_guest_state_area();
	void dump_syscall_check();
	void dump_idt();

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
		bool used;
		ThreatType type;
		unsigned long long id;
	};

	struct UsermodeThreatReport
	{
		ThreatType type;
		unsigned long long id;
	};
#pragma pack(pop)

	void log_to_usermode(ThreatType type, unsigned long long id);
	size_t dump_log(UsermodeThreatReport* buffer, size_t buffer_length);
}
