#pragma once
#pragma warning(disable: 4201)

namespace intel
{
    enum class Msr
    {
        IA32_FEATURE_CONTROL = 0x3a,
        IA32_VMX_BASIC = 0x480,
        IA32_VMX_CR0_FIXED0 = 0x486,
        IA32_VMX_CR0_FIXED1 = 0x487,
        IA32_VMX_CR4_FIXED0 = 0x488,
        IA32_VMX_CR4_FIXED1 = 0x489
    };

    union Ia32VmxBasic
    {
        unsigned long long raw;

        struct
        {
            unsigned long long revision_identifier : 31;
            unsigned long long : 1;
            unsigned long long region_size : 12;
            unsigned long long : 3;
            unsigned long long supported_ia64 : 1;
            unsigned long long supported_dual_monitor : 1;
            unsigned long long memory_type : 4;
            unsigned long long vm_exit_report : 1;
            unsigned long long vmx_capability_hint : 1;
            unsigned long long : 8;
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
        GET_VENDOR_STRING,
        GET_FEATURES
    };

    struct CpuidGetFeaturesInfo
    {
        union
        {
            unsigned int raw;
        } eax;

        union
        {
            unsigned int raw;
        } ebx;

        union
        {
            struct
            {
                unsigned int sse3 : 1;
                unsigned int pclmul : 1;
                unsigned int dtes64 : 1;
                unsigned int monitor : 1;
                unsigned int ds_cpl : 1;
                unsigned int vmx : 1;
                unsigned int smx : 1;
                unsigned int est : 1;
                unsigned int tm2 : 1;
                unsigned int ssse3 : 1;
                unsigned int cid : 1;
                unsigned int fma : 1;
                unsigned int cx16 : 1;
                unsigned int etprd : 1;
                unsigned int pdcm : 1;
                unsigned int pcide : 1;
                unsigned int dca : 1;
                unsigned int sse4_1 : 1;
                unsigned int sse4_2 : 1;
                unsigned int x2apic : 1;
                unsigned int movbe : 1;
                unsigned int popcnt : 1;
                unsigned int aes : 1;
                unsigned int xsave : 1;
                unsigned int osxsave : 1;
                unsigned int avx : 1;
            };

            unsigned int raw;
        } ecx;

        union
        {
            struct
            {
                unsigned int fpu : 1;
                unsigned int vme : 1;
                unsigned int de : 1;
                unsigned int pse : 1;
                unsigned int tsc : 1;
                unsigned int msr : 1;
                unsigned int pae : 1;
                unsigned int mce : 1;
                unsigned int cx8 : 1;
                unsigned int apic : 1;
                unsigned int sep : 1;
                unsigned int mtrr : 1;
                unsigned int pge : 1;
                unsigned int mca : 1;
                unsigned int cmov : 1;
                unsigned int pat : 1;
                unsigned int pse36 : 1;
                unsigned int psn : 1;
                unsigned int clf : 1;
                unsigned int dtes : 1;
                unsigned int acpi : 1;
                unsigned int mmx : 1;
                unsigned int fxsr : 1;
                unsigned int sse : 1;
                unsigned int sse2 : 1;
                unsigned int ss : 1;
                unsigned int htt : 1;
                unsigned int tm1 : 1;
                unsigned int ia64 : 1;
                unsigned int pbe : 1;
            };

            unsigned int raw;
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

    constexpr size_t VMXON_REGION_SIZE = 4096;
    constexpr size_t VMCS_REGION_SIZE = 4096;

    struct Vmcs
    {
        unsigned int revision_identifier : 31;
        unsigned int shadow_vmcs_indicatior : 1;

        unsigned int vmx_abort_indicator;

        unsigned char data[VMCS_REGION_SIZE - 0x08];
    };
}
