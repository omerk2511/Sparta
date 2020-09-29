#include <ntddk.h>
#include <intrin.h>

#include "vmx.h"
#include "intel.h"
#include "memory.h"
#include "processor_context.h"
#include "templates.h"
#include "asm_helpers.h"

extern ProcessorContext* g_processors_context;

namespace vmx
{
	void enable_vmx_operation();
	void* vmxon();
	void* setup_vmcs(unsigned int processor_index);
	void vmlaunch();

	template<typename T>
		requires is_unsigned_integer<T>
	struct VmreadResult
	{
		bool success;
		T value;
	};

	template<typename T>
	static inline constexpr VmreadResult<T> vmread(intel::VmcsField vmcs_field)
		requires is_unsigned_integer<T>
	{
		size_t value{ };

		auto ret = ::__vmx_vmread(static_cast<size_t>(vmcs_field), &value);

		if (ret == STATUS_SUCCESS)
		{
			return {
				true,
				static_cast<T>(value)
			};
		}

		return {
			false,
			0
		};
	}

	template<typename T>
	static inline constexpr bool vmwrite(intel::VmcsField vmcs_field, T value)
		requires is_unsigned_integer<T>
	{
		auto ret = ::__vmx_vmwrite(static_cast<size_t>(vmcs_field), static_cast<size_t>(value));
		return (ret == STATUS_SUCCESS);
	}
}

bool vmx::initialize_vmx(unsigned int processor_index)
{
	KdPrint(("[*] initializing vmx in processor %d\n", processor_index));

	enable_vmx_operation();
	KdPrint(("[+] enabled vmx operation successfully\n"));

	auto vmxon_region = vmxon();

	if (!vmxon_region)
	{
		KdPrint(("[-] failed initializing vmx in processor %d\n", processor_index));
		return false;
	}

	g_processors_context[processor_index].vmxon_region = vmxon_region;
	KdPrint(("[+] entered vmx root mode successfully\n", processor_index));

	// initialize vmcs
	// vm entry (?)

	KdPrint(("[+] successfully initialized vmx in processor %d\n", processor_index));

	return true;
}

void vmx::enable_vmx_operation()
{
	intel::Cr4 cr4 = { ::__readcr4() };
	cr4.vmxe = true;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully set the vmx enabled bit of cr4\n"));

	intel::Ia32FeatureControl feature_control = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_FEATURE_CONTROL)) };

	if (!feature_control.lock)
	{
		feature_control.lock = true;
		feature_control.enable_smx = true;
		feature_control.enable_vmxon = true;

		::__writemsr(
			static_cast<unsigned long>(intel::Msr::IA32_FEATURE_CONTROL),
			feature_control.raw
		);

		KdPrint(("[+] successfully enabled vmxon in ia32_feature_control msr\n"));
	}
	else
	{
		KdPrint(("[+] vmxon is already enabled in ia32_feature_control msr\n"));
	}

	auto ia32_vmx_cr0_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR0_FIXED0)
	);

	auto ia32_vmx_cr0_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR0_FIXED1)
	);

	auto cr0 = ::__readcr0();
	cr0 |= ia32_vmx_cr0_fixed0;
	cr0 &= ia32_vmx_cr0_fixed1;
	::__writecr0(cr0);

	auto ia32_vmx_cr4_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR4_FIXED0)
	);

	auto ia32_vmx_cr4_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::IA32_VMX_CR4_FIXED1)
	);

	cr4.raw = ::__readcr4();
	cr4.raw |= ia32_vmx_cr4_fixed0;
	cr4.raw &= ia32_vmx_cr4_fixed1;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully updated cr0 and cr4 based on the required bits\n"));
}

void* vmx::vmxon()
{
	auto virtual_vmxon_region = reinterpret_cast<void*>(new (NonPagedPool) unsigned char[intel::VMXON_REGION_SIZE]);

	if (!virtual_vmxon_region)
	{
		KdPrint(("[-] could not allocate a vmxon region\n"));
		return nullptr;
	}

	::RtlSecureZeroMemory(virtual_vmxon_region, intel::VMXON_REGION_SIZE);

	auto physical_vmxon_region = ::MmGetPhysicalAddress(virtual_vmxon_region).QuadPart;

	KdPrint(("[+] vmxon region allocated @0x%p (virtual) -> @0x%p (physical)\n", virtual_vmxon_region, physical_vmxon_region));

	intel::Ia32VmxBasic ia32_vmx_basic = { ::__readmsr(static_cast<unsigned long>(intel::Msr::IA32_VMX_BASIC)) };
	*reinterpret_cast<unsigned long*>(virtual_vmxon_region) = static_cast<unsigned long>(ia32_vmx_basic.revision_identifier);

	auto ret = ::__vmx_on(reinterpret_cast<unsigned long long*>(&physical_vmxon_region));
	
	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmxon_region;
		KdPrint(("[-] vmxon failed\n"));

		return nullptr;
	}

	KdPrint(("[+] vmxon succeeded\n"));

	return virtual_vmxon_region;
}

void* vmx::setup_vmcs(unsigned int processor_index)
{
	auto virtual_vmcs_region = new (NonPagedPool) intel::Vmcs;

	if (!virtual_vmcs_region)
	{
		KdPrint(("[-] could not allocate a vmcs region\n"));
		return nullptr;
	}

	::RtlSecureZeroMemory(virtual_vmcs_region, intel::VMXON_REGION_SIZE);

	auto physical_vmcs_region = ::MmGetPhysicalAddress(virtual_vmcs_region).QuadPart;
	KdPrint(("[+] vmcs region allocated @0x%p (virtual) -> @0x%p (physical)\n", virtual_vmcs_region, physical_vmcs_region));

	auto ret = ::__vmx_vmclear(reinterpret_cast<unsigned long long*>(physical_vmcs_region));

	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmclear failed\n"));

		return nullptr;
	}

	ret = ::__vmx_vmptrld(reinterpret_cast<unsigned long long*>(physical_vmcs_region));

	if (ret != STATUS_SUCCESS)
	{
		delete[] virtual_vmcs_region;
		KdPrint(("[-] vmptrld failed\n"));

		return nullptr;
	}

	vmwrite(intel::VmcsField::VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, static_cast<unsigned short>(processor_index + 1));
	
	auto segment_selectors = asm_helpers::get_segment_selectors();

	vmwrite(intel::VmcsField::VMCS_GUEST_ES_SELECTOR, segment_selectors.es);
	vmwrite(intel::VmcsField::VMCS_GUEST_CS_SELECTOR, segment_selectors.cs);
	vmwrite(intel::VmcsField::VMCS_GUEST_SS_SELECTOR, segment_selectors.ss);
	vmwrite(intel::VmcsField::VMCS_GUEST_DS_SELECTOR, segment_selectors.ds);
	vmwrite(intel::VmcsField::VMCS_GUEST_FS_SELECTOR, segment_selectors.fs);
	vmwrite(intel::VmcsField::VMCS_GUEST_GS_SELECTOR, segment_selectors.gs);
	vmwrite(intel::VmcsField::VMCS_GUEST_LDTR_SELECTOR, segment_selectors.ldtr);
	vmwrite(intel::VmcsField::VMCS_GUEST_TR_SELECTOR, segment_selectors.tr);

	return virtual_vmcs_region;
}
