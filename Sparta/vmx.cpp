#include "vmx.h"
#include "intel.h"

#include <ntddk.h>
#include <intrin.h>

void vmx::enable_vmx_operation()
{
	intel::Cr4 cr4 = { 0 };
	cr4.raw = ::__readcr4();
	cr4.vmxe = true;
	::__writecr4(cr4.raw);

	KdPrint(("[+] successfully set the vmx enabled bit of cr4\n"));

	intel::Ia32FeatureControl feature_control = { 0 };

	feature_control.raw = ::__readmsr(
		static_cast<unsigned long>(intel::Msr::kIa32FeatureControl)
	);

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
