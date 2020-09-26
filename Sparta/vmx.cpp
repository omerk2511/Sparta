#include "vmx.h"
#include "intel.h"

#include <ntddk.h>
#include <intrin.h>

void vmx::enable_vmx_operation()
{
	intel::Cr4 cr4 = { ::__readcr4() };
	cr4.vmxe = true;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully set the vmx enabled bit of cr4\n"));

	intel::Ia32FeatureControl feature_control = { ::__readmsr(static_cast<unsigned long>(intel::Msr::kIa32FeatureControl)) };

	if (!feature_control.lock)
	{
		feature_control.lock = true;
		feature_control.enable_smx = true;
		feature_control.enable_vmxon = true;

		::__writemsr(
			static_cast<unsigned long>(intel::Msr::kIa32FeatureControl),
			feature_control.raw
		);

		KdPrint(("[+] successfully enabled vmxon in ia32_feature_control msr\n"));
	}
	else
	{
		KdPrint(("[+] vmxon is already enabled in ia32_feature_control msr\n"));
	}

	auto ia32_vmx_cr0_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::kIa32VmxCr0Fixed0)
	);

	auto ia32_vmx_cr0_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::kIa32VmxCr0Fixed0)
	);

	auto cr0 = ::__readcr0();
	cr0 |= (ia32_vmx_cr0_fixed0 & 0xffffffff);
	cr0 &= ia32_vmx_cr0_fixed1;
	::__writecr0(cr0);

	auto ia32_vmx_cr4_fixed0 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::kIa32VmxCr4Fixed0)
	);

	auto ia32_vmx_cr4_fixed1 = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::kIa32VmxCr4Fixed1)
	);

	cr4.raw = ::__readcr4();
	cr4.raw |= (ia32_vmx_cr4_fixed0 & 0xffffffff);
	cr4.raw &= ia32_vmx_cr4_fixed1;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully updated cr0 and cr4 based on the required bits\n"));
}

static constexpr size_t kVmxonRegionSize = 4096;

vmx::VmxonResult vmx::vmxon()
{
	VmxonResult result = { 0 };

	PHYSICAL_ADDRESS max_addr = { 0 };
	max_addr.QuadPart = MAXULONG64;
	auto address = ::MmAllocateContiguousMemory(kVmxonRegionSize * 2, max_addr);

	if (!address)
	{
		KdPrint(("[-] could not allocate a vmxon region\n"));
		return result;
	}

	::RtlSecureZeroMemory(address, kVmxonRegionSize * 2);

	auto virtual_aligned = reinterpret_cast<void*>(
		(reinterpret_cast<unsigned long long>(address) + kVmxonRegionSize - 1) & ~(kVmxonRegionSize - 1));
	auto physical_aligned = ::MmGetPhysicalAddress(virtual_aligned).QuadPart;

	intel::Ia32VmxBasic ia32_vmx_basic = { ::__readmsr(static_cast<unsigned long>(intel::Msr::kIa32VmxBasic)) };
	*reinterpret_cast<unsigned long*>(virtual_aligned) = static_cast<unsigned long>(ia32_vmx_basic.revision_identifier);

	auto ret = ::__vmx_on(reinterpret_cast<unsigned long long*>(&physical_aligned));

	result.success = (ret == STATUS_SUCCESS);
	result.vmxon_region = virtual_aligned;

	return result;
}
