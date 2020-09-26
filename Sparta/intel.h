#pragma once

namespace intel
{
    enum class Msr
    {
        kIa32FeatureControl = 0x3a
    };

    union Ia32FeatureControl
    {
        unsigned long long all;

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
}
