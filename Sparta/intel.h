#pragma once
#pragma warning(disable: 4201)

namespace intel
{
    enum class Msr
    {
        kIa32FeatureControl = 0x3a,
        kIa32VmxBasic = 0x480,
        kIa32VmxCr0Fixed0 = 0x486,
        kIa32VmxCr0Fixed1 = 0x487,
        kIa32VmxCr4Fixed0 = 0x488,
        kIa32VmxCr4Fixed1 = 0x489
    };

    union Ia32VmxBasic
    {
        unsigned long long raw;

        struct
        {
            unsigned long long revision_identifier : 31;
            unsigned long long reserved_1 : 1;
            unsigned long long region_size : 12;
            unsigned long long reserved_2 : 3;
            unsigned long long supported_ia64 : 1;
            unsigned long long supported_dual_monitor : 1;
            unsigned long long memory_type : 4;
            unsigned long long vm_exit_report : 1;
            unsigned long long vmx_capability_hint : 1;
            unsigned long long reserved_3 : 8;
        };
    };

    union Ia32FeatureControl
    {
        unsigned long long raw;

        struct
        {
            unsigned long long lock : 1;
            unsigned long long enable_smx : 1;
            unsigned long long enable_vmxon : 1;
        };
    };

    enum class CpuidType
    {
        kGetVendorString,
        kGetFeatures
    };

    struct CpuidGetFeaturesInfo
    {
        union
        {
            int raw;
        } eax;

        union
        {
            int raw;
        } ebx;

        union
        {
            struct
            {
                int sse3 : 1;
                int pclmul : 1;
                int dtes64 : 1;
                int monitor : 1;
                int ds_cpl : 1;
                int vmx : 1;
                int smx : 1;
                int est : 1;
                int tm2 : 1;
                int ssse3 : 1;
                int cid : 1;
                int fma : 1;
                int cx16 : 1;
                int etprd : 1;
                int pdcm : 1;
                int pcide : 1;
                int dca : 1;
                int sse4_1 : 1;
                int sse4_2 : 1;
                int x2apic : 1;
                int movbe : 1;
                int popcnt : 1;
                int aes : 1;
                int xsave : 1;
                int osxsave : 1;
                int avx : 1;
            };

            int raw;
        } ecx;

        union
        {
            struct
            {
                int fpu : 1;
                int vme : 1;
                int de : 1;
                int pse : 1;
                int tsc : 1;
                int msr : 1;
                int pae : 1;
                int mce : 1;
                int cx8 : 1;
                int apic : 1;
                int sep : 1;
                int mtrr : 1;
                int pge : 1;
                int mca : 1;
                int cmov : 1;
                int pat : 1;
                int pse36 : 1;
                int psn : 1;
                int clf : 1;
                int dtes : 1;
                int acpi : 1;
                int mmx : 1;
                int fxsr : 1;
                int sse : 1;
                int sse2 : 1;
                int ss : 1;
                int htt : 1;
                int tm1 : 1;
                int ia64 : 1;
                int pbe : 1;
            };

            int raw;
        } edx;
    };

    union Cr4
    {
        unsigned long long raw;

        struct
        {
            unsigned long long vme : 1;
            unsigned long long pvi : 1;
            unsigned long long tsd : 1;
            unsigned long long de : 1;
            unsigned long long pse : 1;
            unsigned long long pae : 1;
            unsigned long long mce : 1;
            unsigned long long pge : 1;
            unsigned long long pce : 1;
            unsigned long long osfxsr : 1;
            unsigned long long osxmmexcpt : 1;
            unsigned long long umip : 1;
            unsigned long long la57 : 1;
            unsigned long long vmxe : 1;
            unsigned long long smxe : 1;
            unsigned long long fsgsbase : 1;
            unsigned long long pcide : 1;
            unsigned long long osxsave : 1;
            unsigned long long smep : 1;
            unsigned long long smap : 1;
            unsigned long long pke : 1;
        };
    };
}
