#include "loader.h"

#include "vcpu.h"
#include "ept.h"
#include "vmcs.h"
#include "multiprocessing.h"
#include "vmx.h"
#include "asm_helpers.h"
#include "logging.h"
#include "memory.h"
#include "module_manager.h"
#include "basic_module.h"
#include "token_stealing_module.h"

static auto allocate_vcpu_context() -> VcpuContext*;

static auto is_hypervisor_present() -> bool
{
	intel::GeneralCpuidInfo cpuid_info = { 0 };
	::__cpuid(reinterpret_cast<int*>(&cpuid_info), 0);
	return (cpuid_info.ebx == 0x72617053);
}

auto loader::load_sparta(SpartaContext* sparta_context) -> bool
{
	auto processor_index = static_cast<unsigned long>(multiprocessing::get_current_processor_id());
	KdPrint(("[*] loading sparta on processor %u\n", processor_index));

	sparta::register_module(new (NonPagedPool) BasicModule);
	sparta::register_module(new (NonPagedPool) TokenStealingModule);

	auto vcpu_context = allocate_vcpu_context();

	if (!vcpu_context)
	{
		KdPrint(("[-] could not allocate a vcpu context in processor %u\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully allocated a vcpu context @0x%llx\n", vcpu_context));

	vcpu_context->processor_index = processor_index;

	vmx::enable_vmx();
	KdPrint(("[+] enabled vmx operation successfully\n"));

	vcpu_context->vmxon_region = vmx::vmxon();

	if (!vcpu_context->vmxon_region)
	{
		KdPrint(("[-] failed initializing vmx in processor %u\n", processor_index));
		return false;
	}
	KdPrint(("[+] entered vmx root mode successfully in processor %u\n", processor_index));

	ept::setup(vcpu_context);
	KdPrint(("[+] successfully initialized the ept in processor %u\n", processor_index));

	vmcs::setup(vcpu_context, sparta_context->host_cr3);
	sparta::initialize_modules();

	if (!vcpu_context->vmcs_region)
	{
		KdPrint(("[-] failed initializing the vmcs in processor %u\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized the vmcs in processor %u\n", processor_index));

	auto success = true;
	::RtlCaptureContext(&vcpu_context->guest_context);
	
	if (!is_hypervisor_present())
	{
		KdPrint(("[*] vmlaunch in host\n"));
		success = vmx::vmlaunch();
	}
	else
	{
		KdPrint(("[*] in guest :)\n"));
	}

	if (!success)
	{
		KdPrint(("[-] failed initializing vmx in processor %u\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized vmx in processor %u\n", processor_index));

	return true;
}

auto allocate_vcpu_context() -> VcpuContext*
{
	auto vcpu_context = new (NonPagedPool) VcpuContext;
	if (!vcpu_context)
	{
		return nullptr;
	}

	::RtlSecureZeroMemory(vcpu_context, sizeof(VcpuContext));

	return vcpu_context;
}
