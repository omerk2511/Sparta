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
	void* _hooked_virtual_page;
	void* _original_virtual_page;

	unsigned long long _guest_physical_page;

	intel::EptPte* _pte;

	bool _restore_page;

private:
	static constexpr unsigned char JMP_PATCH[] = "\x50\x48\xB8\x78\x56\x34\x12\x78\x56\x34\x12\x48\x87\x04\x24\xC3";
};
