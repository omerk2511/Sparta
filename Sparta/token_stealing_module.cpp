#include "token_stealing_module.h"
#include "vmx.h"

PACCESS_TOKEN TokenStealingModule::_system_token;
bool TokenStealingModule::_should_bsod;

TokenStealingModule::TokenStealingModule()
{
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::WRMSR)] = handle_wrmsr;

	PEPROCESS system_process = { 0 };
	::PsLookupProcessByProcessId(::ULongToHandle(4ul), &system_process);

	_system_token = ::PsReferencePrimaryToken(system_process);
	::PsDereferencePrimaryToken(_system_token);

	_should_bsod = false;
	::KeRegisterNmiCallback(reinterpret_cast<PNMI_CALLBACK>(handle_nmi), nullptr);
}

void TokenStealingModule::handle_wrmsr(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&)
{
	if (static_cast<intel::Msr>(guest_state->rcx) != intel::Msr::IA32_KERNEL_GS_BASE)
	{
		return;
	}

	if (intel::is_um_address(::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_GS_BASE))))
	{
		return;
	}

	auto token = ::PsReferencePrimaryToken(PsGetCurrentProcess());
	auto pid = ::PsGetCurrentProcessId();

	if (token != _system_token || pid == ::ULongToHandle(4ul))
	{
		::PsDereferencePrimaryToken(token);
		return;
	}

	_should_bsod = true;
	vmx::inject_nmi();
}

bool TokenStealingModule::handle_nmi(void*, bool)
{
	if (!_should_bsod)
	{
		return false;
	}

	::KeBugCheck(0x1337ul);
}
