#pragma once

#include "intel.h"
#include "vcpu.h"
#include "memory.h"

namespace sparta
{
#pragma pack(push, 1)
	struct VmExitGuestState
	{
		unsigned long long rsp;
		unsigned long long rax;
		unsigned long long rcx;
		unsigned long long rdx;
		unsigned long long rbx;
		unsigned long long rbp;
		unsigned long long rsi;
		unsigned long long rdi;
		unsigned long long r8;
		unsigned long long r9;
		unsigned long long r10;
		unsigned long long r11;
		unsigned long long r12;
		unsigned long long r13;
		unsigned long long r14;
		unsigned long long cr2;
		unsigned long long r15;
	};
#pragma pack(pop)

	using vmexit_handler = void (*)(VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip);

	class BaseModule
	{
	public:
		explicit BaseModule() : _vmexit_handlers() {}
		virtual ~BaseModule() {}

	public:
		virtual void initialize() volatile = 0;

	public:
		inline void handle_vmexit(intel::VmExitReason reason, VcpuContext* vcpu_context, VmExitGuestState* guest_state, bool& increment_rip) const volatile
		{
			auto handler = _vmexit_handlers[static_cast<size_t>(reason)];
			if (handler)
			{
				handler(vcpu_context, guest_state, increment_rip);
			}
		}

	protected:
		vmexit_handler _vmexit_handlers[static_cast<size_t>(intel::VmExitReason::MAX)];
	};
}
