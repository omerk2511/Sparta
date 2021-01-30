#include "basic_module.h"
#include "vmx.h"
#include "asm_helpers.h"

BasicModule::BasicModule()
{
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::TRIPLE_FAULT)] = handle_triple_fault;
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::CPUID)] = handle_cpuid;
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::INVD)] = handle_invd;
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::RDMSR)] = handle_rdmsr;
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::WRMSR)] = handle_wrmsr;
	_vmexit_handlers[static_cast<size_t>(intel::VmExitReason::XSETBV)] = handle_xsetbv;
}

void BasicModule::handle_triple_fault(VcpuContext*, sparta::VmExitGuestState* guest_state, bool& increment_rip)
{
	KdPrint(("rsp: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RSP).value));
	KdPrint(("rax: 0x%llx\n", guest_state->rax));
	KdPrint(("rcx: 0x%llx\n", guest_state->rcx));
	KdPrint(("rdx: 0x%llx\n", guest_state->rdx));
	KdPrint(("rbx: 0x%llx\n", guest_state->rbx));
	KdPrint(("rbp: 0x%llx\n", guest_state->rbp));
	KdPrint(("rsi: 0x%llx\n", guest_state->rsi));
	KdPrint(("rdi: 0x%llx\n", guest_state->rdi));
	KdPrint(("r8: 0x%llx\n", guest_state->r8));
	KdPrint(("r9: 0x%llx\n", guest_state->r9));
	KdPrint(("r10: 0x%llx\n", guest_state->r10));
	KdPrint(("r11: 0x%llx\n", guest_state->r11));
	KdPrint(("r12: 0x%llx\n", guest_state->r12));
	KdPrint(("r13: 0x%llx\n", guest_state->r13));
	KdPrint(("r14: 0x%llx\n", guest_state->r14));
	KdPrint(("r15: 0x%llx\n", guest_state->r15));

	::__debugbreak();

	increment_rip = false;
}

void BasicModule::handle_cpuid(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&)
{
	intel::GeneralCpuidInfo cpuid_info = { 0 };

	::__cpuid(
		reinterpret_cast<int*>(&cpuid_info),
		static_cast<int>(guest_state->rax)
	);

	if ((guest_state->rax & 0xffffffff) == 0)
	{
		cpuid_info.ebx = 0x72617053;
		cpuid_info.edx = 0x70536174;
		cpuid_info.ecx = 0x61747261;
	}

	guest_state->rax = static_cast<unsigned long long>(cpuid_info.eax);
	guest_state->rbx = static_cast<unsigned long long>(cpuid_info.ebx);
	guest_state->rcx = static_cast<unsigned long long>(cpuid_info.ecx);
	guest_state->rdx = static_cast<unsigned long long>(cpuid_info.edx);
}

void BasicModule::handle_invd(VcpuContext*, sparta::VmExitGuestState*, bool&)
{
	asm_helpers::invd();
}

void BasicModule::handle_rdmsr(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&)
{
	auto msr = ::__readmsr(static_cast<unsigned long>(guest_state->rcx));

	guest_state->rax = msr & 0xffffffff;
	guest_state->rdx = msr >> 32;
}

void BasicModule::handle_wrmsr(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&)
{
	::__writemsr(
		static_cast<unsigned long>(guest_state->rcx),
		(guest_state->rax & 0xffffffff) | (guest_state->rdx << 32)
	);
}

void BasicModule::handle_xsetbv(VcpuContext*, sparta::VmExitGuestState* guest_state, bool&)
{
	::_xsetbv(
		static_cast<unsigned long>(guest_state->rcx),
		(guest_state->rax & 0xffffffff) | (guest_state->rdx << 32)
	);
}
