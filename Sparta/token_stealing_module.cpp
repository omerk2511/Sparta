#include "token_stealing_module.h"
#include "vmx.h"

TokenStealingModule::TokenStealingModule() :
	_system_token(nullptr),
	_should_bsod(false)
{
	PEPROCESS system_process = { 0 };
	::PsLookupProcessByProcessId(::ULongToHandle(4ul), &system_process);

	_system_token = ::PsReferencePrimaryToken(system_process);
	::PsDereferencePrimaryToken(_system_token);

	_should_bsod = false;
	::KeRegisterNmiCallback(reinterpret_cast<PNMI_CALLBACK>(handle_nmi), this);
}

void TokenStealingModule::handle_wrmsr(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&) volatile
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

bool TokenStealingModule::handle_nmi(void* context, bool)
{
	auto _this = reinterpret_cast<TokenStealingModule*>(context);
	if (!_this->_should_bsod)
	{
		return false;
	}

	::KeBugCheck(0x1337ul);
}
