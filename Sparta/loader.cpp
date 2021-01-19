#include "loader.h"

#include "vcpu.h"
#include "ept.h"
#include "vmcs.h"
#include "multiprocessing.h"
#include "vmx.h"
#include "asm_helpers.h"
#include "logging.h"
#include "memory.h"

static auto allocate_vcpu_context() -> VcpuContext*;

static auto is_hypervisor_present() -> bool
{
	intel::GeneralCpuidInfo cpuid_info = { 0 };
	::__cpuid(reinterpret_cast<int*>(&cpuid_info), 0);
	return (cpuid_info.ebx == 0x72617053);
}

static bool handle_write_hook(VcpuContext* vcpu_context)
{
	KdPrint(("[*] got ept write hook! blocking the write attempt.\n"));
	ept::unhook(vcpu_context);
	return true;
}

auto loader::load_sparta(SpartaContext* sparta_context) -> bool
{
	auto processor_index = static_cast<unsigned long>(multiprocessing::get_current_processor_id());
	KdPrint(("[*] loading sparta on processor %ul\n", processor_index));

	auto vcpu_context = allocate_vcpu_context();

	if (!vcpu_context)
	{
		KdPrint(("[-] could not allocate a vcpu context in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully allocated a vcpu context @0x%llx\n", vcpu_context));

	vcpu_context->processor_index = processor_index;

	vmx::enable_vmx();
	KdPrint(("[+] enabled vmx operation successfully\n"));

	vcpu_context->vmxon_region = vmx::vmxon();

	if (!vcpu_context->vmxon_region)
	{
		KdPrint(("[-] failed initializing vmx in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] entered vmx root mode successfully in processor %ul\n", processor_index));

	ept::setup(vcpu_context);
	vmcs::setup(vcpu_context, sparta_context->host_cr3);

	if (!vcpu_context->vmcs_region)
	{
		KdPrint(("[-] failed initializing the vmcs in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized the vmcs in processor %ul\n", processor_index));

	auto data = new (NonPagedPool) char[4096];
	::RtlSecureZeroMemory(data, 4096);

	ept::hook_write(reinterpret_cast<void*>(::MmGetPhysicalAddress(&data[5]).QuadPart), handle_write_hook, vcpu_context);

	auto success = true;
	::RtlCaptureContext(&vcpu_context->guest_context);
	
	if (!is_hypervisor_present())
	{
		KdPrint(("[*] vmlaunch in host\n"));
		success = vmx::vmlaunch();
	}
	else
	{
		// KdPrint(("[*] in guest :)\n"));
	}

	if (!success)
	{
		KdPrint(("[-] failed initializing vmx in processor %ul\n", processor_index));
		return false;
	}
	KdPrint(("[+] successfully initialized vmx in processor %ul\n", processor_index));

	// log some MSRs to check syscall stuff
	// logging::dump_syscall_check();

	// dump idt
	// logging::dump_idt();

	data[0] = 'c';
	data[1] = 'a';
	data[5] = 'b';
	data[7] = 'd';

	delete[] data;

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
