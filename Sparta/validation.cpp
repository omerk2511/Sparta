#include "validation.h"
#include "intel.h"

#include <ntddk.h>
#include <intrin.h>

bool validation::is_os_supported()
{
#ifndef _WIN64
    KdPrint(("[-] sparta only supports 64 bit machines\n"));
    return false;
#endif

    RTL_OSVERSIONINFOW os_version_info = { 0 };
    auto status = ::RtlGetVersion(&os_version_info);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("[-] could not get os version\n"));
        return false;
    }

    if (os_version_info.dwMajorVersion != 10)
    {
        KdPrint(("[-] sparta only supports windows 10 machines\n"));
        return false;
    }

    return true;
}

bool validation::is_vmx_supported()
{
    intel::CpuidGetFeaturesInfo features_info = { 0 };

    ::__cpuid(
        reinterpret_cast<int*>(&features_info),
        static_cast<int>(intel::CpuidType::kGetFeatures)
    );

    if (!features_info.ecx.vmx)
    {
        KdPrint(("[-] cpuid vmx bit is not set\n"));
        return false;
    }

    intel::Ia32FeatureControl feature_control = { ::__readmsr(static_cast<unsigned long>(intel::Msr::kIa32FeatureControl)) };

    if (feature_control.lock &&
        ((!feature_control.enable_vmxon && !features_info.ecx.smx) ||
         (!feature_control.enable_smx && features_info.ecx.smx)))
    {
        KdPrint(("[-] vmx operation was disabled in the bios setup\n"));
        return false;
    }

    return true;
}
