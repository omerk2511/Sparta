#pragma once

#include "base_module.h"

class TokenStealingModule final : public sparta::BaseModule
{
public:
	explicit TokenStealingModule();

protected:
	virtual void handle_wrmsr(VcpuContext* vcpu_context, sparta::VmExitGuestState* guest_state, bool& increment_rip) volatile override;

private:
	static bool handle_nmi(void* context, bool handled);
	static void handle_dpc(KDPC* dpc, void* context, void* system_argument_1, void* system_argument_2);
	static void handle_work_item(PDEVICE_OBJECT device_object, void* context);

private:
	static unsigned long get_pid_by_name(const char* process_name);

private:
	static constexpr size_t SYSTEM_PROCESS_COUNT = 8;
	static const char* SYSTEM_PROCESS_NAMES[SYSTEM_PROCESS_COUNT];

private:
	unsigned long _system_process_pids[SYSTEM_PROCESS_COUNT];
	PACCESS_TOKEN _system_token;

private:
	bool _should_kill_process;
	int _pid;
	KDPC _dpc;
	PIO_WORKITEM _work_item;

private:
	static constexpr unsigned long SYSTEM_PROCESS_PID = 4;
};
