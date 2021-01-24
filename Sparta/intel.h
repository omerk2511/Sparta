#pragma once
#pragma warning(disable: 4201)
#pragma pack(push, 1)

extern "C" void _sgdt(void*);

namespace intel
{
    enum class Msr : unsigned long
    {
        IA32_FEATURE_CONTROL = 0x3a,
        IA32_MTRRCAP = 0xfe,
        IA32_SYSENTER_CS = 0x174,
        IA32_SYSENTER_ESP = 0x175,
        IA32_SYSENTER_EIP = 0x176,
        IA32_DEBUGCTL = 0x1d9,
        IA32_MTRR_DEF_TYPE = 0x2ff,
        IA32_VMX_BASIC = 0x480,
        IA32_VMX_PINBASED_CTLS = 0x481,
        IA32_VMX_PROCBASED_CTLS = 0x482,
        IA32_VMX_EXIT_CTLS = 0x483,
        IA32_VMX_ENTRY_CTLS = 0x484,
        IA32_VMX_CR0_FIXED0 = 0x486,
        IA32_VMX_CR0_FIXED1 = 0x487,
        IA32_VMX_CR4_FIXED0 = 0x488,
        IA32_VMX_CR4_FIXED1 = 0x489,
        IA32_VMX_PROCBASED_CTLS2 = 0x48b,
        IA32_VMX_TRUE_PINBASED_CTLS = 0x48d,
        IA32_VMX_TRUE_PROCBASED_CTLS = 0x48e,
        IA32_VMX_TRUE_EXIT_CTLS = 0x48f,
        IA32_VMX_TRUE_ENTRY_CTLS = 0x490,
        IA32_EFER = 0xc0000080,
        IA32_FS_BASE = 0xc0000100,
        IA32_GS_BASE = 0xc0000101,
        IA32_STAR = 0xc0000081,
        IA32_LSTAR = 0xc0000082,
        IA32_CSTAR = 0xc0000083
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

    union Ia32VmxControlsHint
    {
        unsigned long long raw;

        struct
        {
            unsigned long allowed_0_settings;
            unsigned long allowed_1_settings;
        };
    };

    enum class CpuidType
    {
        GET_VENDOR_STRING,
        GET_FEATURES
    };

    struct GeneralCpuidInfo
    {
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
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

    union Cr0
    {
        unsigned long long raw;

        struct
        {
            unsigned long long pe : 1;
            unsigned long long mp : 1;
            unsigned long long em : 1;
            unsigned long long ts : 1;
            unsigned long long et : 1;
            unsigned long long ne : 1;
            unsigned long long : 10;
            unsigned long long wp : 1;
            unsigned long long : 1;
            unsigned long long am : 1;
            unsigned long long : 10;
            unsigned long long nw : 1;
            unsigned long long cd : 1;
            unsigned long long pg : 1;
        };
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

    union SegmentDescriptor
    {
        unsigned long long raw;

        struct
        {
            unsigned long long limit_0 : 16;
            unsigned long long base_0 : 24;
            unsigned long long attr_0 : 8;
            unsigned long long limit_1 : 4;
            unsigned long long attr_1 : 4;
            unsigned long long base_1 : 8;
        };
    };

    union SystemSegmentDescriptor
    {
        struct
        {
            unsigned long long raw_1;
            unsigned long long raw_2;
        };

        struct
        {
            unsigned long long limit_0 : 16;
            unsigned long long base_0 : 24;
            unsigned long long attr_0 : 8;
            unsigned long long limit_1 : 4;
            unsigned long long attr_1 : 4;
            unsigned long long base_1 : 8;
            unsigned long long base_2 : 32;
            unsigned long long : 32;
        };
    };

    struct Gdtr
    {
        unsigned short limit;
        unsigned long long base;
    };

    struct Idtr
    {
        unsigned short limit;
        unsigned long long base;
    };

    union SegmentAccessRights
    {
        unsigned long raw;

        struct
        {
            unsigned long segment_type : 4;
            unsigned long descriptor_type : 1;
            unsigned long dpl : 2;
            unsigned long segment_present : 1;
            unsigned long : 4;
            unsigned long avl : 1;
            unsigned long cs_64_bit_mode_active : 1;
            unsigned long default_operation_size : 1;
            unsigned long granularity : 1;
            unsigned long segment_unusable : 1;
        };
    };

    inline constexpr size_t VMXON_REGION_SIZE = 4096;
    inline constexpr size_t VMCS_REGION_SIZE = 4096;

    struct Vmcs
    {
        unsigned int revision_identifier : 31;
        unsigned int shadow_vmcs_indicatior : 1;

        unsigned int vmx_abort_indicator;

        unsigned char data[VMCS_REGION_SIZE - 2 * sizeof(unsigned int)];
    };

    enum class VmcsField
    {
        VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER = 0x0,           // replace with CTRL_VIRTUAL_PROCESSOR_IDENTIFIER
        VMCS_CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR = 0x2,   // replace with CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR
        VMCS_CTRL_EPTP_INDEX = 0x4,
        VMCS_GUEST_ES_SELECTOR = 0x800,
        VMCS_GUEST_CS_SELECTOR = 0x802,
        VMCS_GUEST_SS_SELECTOR = 0x804,
        VMCS_GUEST_DS_SELECTOR = 0x806,
        VMCS_GUEST_FS_SELECTOR = 0x808,
        VMCS_GUEST_GS_SELECTOR = 0x80a,
        VMCS_GUEST_LDTR_SELECTOR = 0x80c,
        VMCS_GUEST_TR_SELECTOR = 0x80e,
        VMCS_GUEST_INTERRUPT_STATUS = 0x810,
        VMCS_GUEST_PML_INDEX = 0x812,
        VMCS_HOST_ES_SELECTOR = 0xc00,
        VMCS_HOST_CS_SELECTOR = 0xc02,
        VMCS_HOST_SS_SELECTOR = 0xc04,
        VMCS_HOST_DS_SELECTOR = 0xc06,
        VMCS_HOST_FS_SELECTOR = 0xc08,
        VMCS_HOST_GS_SELECTOR = 0xc0a,
        VMCS_HOST_TR_SELECTOR = 0xc0c,
        VMCS_CTRL_IO_BITMAP_A_ADDRESS = 0x2000,
        VMCS_CTRL_IO_BITMAP_B_ADDRESS = 0x2002,
        VMCS_CTRL_MSR_BITMAP_ADDRESS = 0x2004,
        VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS = 0x2006,
        VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS = 0x2008,
        VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS = 0x200a,
        VMCS_CTRL_EXECUTIVE_VMCS_POINTER = 0x200c,
        VMCS_CTRL_PML_ADDRESS = 0x200e,
        VMCS_CTRL_TSC_OFFSET = 0x2010,
        VMCS_CTRL_VIRTUAL_APIC_ADDRESS = 0x2012,
        VMCS_CTRL_APIC_ACCESS_ADDRESS = 0x2014,
        VMCS_CTRL_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS = 0x2016,
        VMCS_CTRL_VMFUNC_CONTROLS = 0x2018,
        VMCS_CTRL_EPT_POINTER = 0x201a,
        VMCS_CTRL_EOI_EXIT_BITMAP_0 = 0x201c,
        VMCS_CTRL_EOI_EXIT_BITMAP_1 = 0x201e,
        VMCS_CTRL_EOI_EXIT_BITMAP_2 = 0x2020,
        VMCS_CTRL_EOI_EXIT_BITMAP_3 = 0x2022,
        VMCS_CTRL_EPT_POINTER_LIST_ADDRESS = 0x2024,
        VMCS_CTRL_VMREAD_BITMAP_ADDRESS = 0x2026,
        VMCS_CTRL_VMWRITE_BITMAP_ADDRESS = 0x2028,
        VMCS_CTRL_VIRTUALIZATION_EXCEPTION_INFORMATION_ADDRESS = 0x202a,
        VMCS_CTRL_XSS_EXITING_BITMAP = 0x202c,
        VMCS_CTRL_ENCLS_EXITING_BITMAP = 0x202e,
        VMCS_CTRL_TSC_MULTIPLIER = 0x2032,
        VMCS_GUEST_PHYSICAL_ADDRESS = 0x2400,
        VMCS_GUEST_VMCS_LINK_POINTER = 0x2800,
        VMCS_GUEST_DEBUGCTL = 0x2802,
        VMCS_GUEST_PAT = 0x2804,
        VMCS_GUEST_EFER = 0x2806,
        VMCS_GUEST_PERF_GLOBAL_CTRL = 0x2808,
        VMCS_GUEST_PDPTE0 = 0x280a,
        VMCS_GUEST_PDPTE1 = 0x280c,
        VMCS_GUEST_PDPTE2 = 0x280e,
        VMCS_GUEST_PDPTE3 = 0x2810,
        VMCS_HOST_PAT = 0x2c00,
        VMCS_HOST_EFER = 0x2c02,
        VMCS_HOST_PERF_GLOBAL_CTRL = 0x2c04,
        VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS = 0x4000,
        VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS = 0x4002,
        VMCS_CTRL_EXCEPTION_BITMAP = 0x4004,
        VMCS_CTRL_PAGEFAULT_ERROR_CODE_MASK = 0x4006,
        VMCS_CTRL_PAGEFAULT_ERROR_CODE_MATCH = 0x4008,
        VMCS_CTRL_CR3_TARGET_COUNT = 0x400a,
        VMCS_CTRL_VMEXIT_CONTROLS = 0x400c,
        VMCS_CTRL_VMEXIT_MSR_STORE_COUNT = 0x400e,
        VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT = 0x4010,
        VMCS_CTRL_VMENTRY_CONTROLS = 0x4012,
        VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT = 0x4014,
        VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD = 0x4016,
        VMCS_CTRL_VMENTRY_EXCEPTION_ERROR_CODE = 0x4018,
        VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH = 0x401a,
        VMCS_CTRL_TPR_THRESHOLD = 0x401c,
        VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS = 0x401e,
        VMCS_CTRL_PLE_GAP = 0x4020,
        VMCS_CTRL_PLE_WINDOW = 0x4022,
        VMCS_VM_INSTRUCTION_ERROR = 0x4400,
        VMCS_EXIT_REASON = 0x4402,
        VMCS_VMEXIT_INTERRUPTION_INFORMATION = 0x4404,
        VMCS_VMEXIT_INTERRUPTION_ERROR_CODE = 0x4406,
        VMCS_IDT_VECTORING_INFORMATION = 0x4408,
        VMCS_IDT_VECTORING_ERROR_CODE = 0x440a,
        VMCS_VMEXIT_INSTRUCTION_LENGTH = 0x440c,
        VMCS_VMEXIT_INSTRUCTION_INFO = 0x440e,
        VMCS_GUEST_ES_LIMIT = 0x4800,
        VMCS_GUEST_CS_LIMIT = 0x4802,
        VMCS_GUEST_SS_LIMIT = 0x4804,
        VMCS_GUEST_DS_LIMIT = 0x4806,
        VMCS_GUEST_FS_LIMIT = 0x4808,
        VMCS_GUEST_GS_LIMIT = 0x480a,
        VMCS_GUEST_LDTR_LIMIT = 0x480c,
        VMCS_GUEST_TR_LIMIT = 0x480e,
        VMCS_GUEST_GDTR_LIMIT = 0x4810,
        VMCS_GUEST_IDTR_LIMIT = 0x4812,
        VMCS_GUEST_ES_ACCESS_RIGHTS = 0x4814,
        VMCS_GUEST_CS_ACCESS_RIGHTS = 0x4816,
        VMCS_GUEST_SS_ACCESS_RIGHTS = 0x4818,
        VMCS_GUEST_DS_ACCESS_RIGHTS = 0x481a,
        VMCS_GUEST_FS_ACCESS_RIGHTS = 0x481c,
        VMCS_GUEST_GS_ACCESS_RIGHTS = 0x481e,
        VMCS_GUEST_LDTR_ACCESS_RIGHTS = 0x4820,
        VMCS_GUEST_TR_ACCESS_RIGHTS = 0x4822,
        VMCS_GUEST_INTERRUPTIBILITY_STATE = 0x4824,
        VMCS_GUEST_ACTIVITY_STATE = 0x4826,
        VMCS_GUEST_SMBASE = 0x4828,
        VMCS_GUEST_SYSENTER_CS = 0x482a,
        VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE = 0x482e,
        VMCS_SYSENTER_CS = 0x4c00,
        VMCS_CTRL_CR0_GUEST_HOST_MASK = 0x6000,
        VMCS_CTRL_CR4_GUEST_HOST_MASK = 0x6002,
        VMCS_CTRL_CR0_READ_SHADOW = 0x6004,
        VMCS_CTRL_CR4_READ_SHADOW = 0x6006,
        VMCS_CTRL_CR3_TARGET_VALUE_0 = 0x6008,
        VMCS_CTRL_CR3_TARGET_VALUE_1 = 0x600a,
        VMCS_CTRL_CR3_TARGET_VALUE_2 = 0x600c,
        VMCS_CTRL_CR3_TARGET_VALUE_3 = 0x600e,
        VMCS_EXIT_QUALIFICATION = 0x6400,
        VMCS_IO_RCX = 0x6402,
        VMCS_IO_RSX = 0x6404,
        VMCS_IO_RDI = 0x6406,
        VMCS_IO_RIP = 0x6408,
        VMCS_EXIT_GUEST_LINEAR_ADDRESS = 0x640a,
        VMCS_GUEST_CR0 = 0x6800,
        VMCS_GUEST_CR3 = 0x6802,
        VMCS_GUEST_CR4 = 0x6804,
        VMCS_GUEST_ES_BASE = 0x6806,
        VMCS_GUEST_CS_BASE = 0x6808,
        VMCS_GUEST_SS_BASE = 0x680a,
        VMCS_GUEST_DS_BASE = 0x680c,
        VMCS_GUEST_FS_BASE = 0x680e,
        VMCS_GUEST_GS_BASE = 0x6810,
        VMCS_GUEST_LDTR_BASE = 0x6812,
        VMCS_GUEST_TR_BASE = 0x6814,
        VMCS_GUEST_GDTR_BASE = 0x6816,
        VMCS_GUEST_IDTR_BASE = 0x6818,
        VMCS_GUEST_DR7 = 0x681a,
        VMCS_GUEST_RSP = 0x681c,
        VMCS_GUEST_RIP = 0x681e,
        VMCS_GUEST_RFLAGS = 0x6820,
        VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS = 0x6822,
        VMCS_GUEST_SYSENTER_ESP = 0x6824,
        VMCS_GUEST_SYSENTER_EIP = 0x6826,
        VMCS_HOST_CR0 = 0x6c00,
        VMCS_HOST_CR3 = 0x6c02,
        VMCS_HOST_CR4 = 0x6c04,
        VMCS_HOST_FS_BASE = 0x6c06,
        VMCS_HOST_GS_BASE = 0x6c08,
        VMCS_HOST_TR_BASE = 0x6c0a,
        VMCS_HOST_GDTR_BASE = 0x6c0c,
        VMCS_HOST_IDTR_BASE = 0x6c0e,
        VMCS_HOST_SYSENTER_ESP = 0x6c10,
        VMCS_HOST_SYSENTER_EIP = 0x6c12,
        VMCS_HOST_RSP = 0x6c14,
        VMCS_HOST_RIP = 0x6c16
    };

    union PinBasedVmxControls
    {
        unsigned long raw;

        struct
        {
            unsigned long external_interrupt_exiting : 1;
            unsigned long : 2;
            unsigned long nmi_exiting : 1;
            unsigned long : 1;
            unsigned long virtual_nmis : 1;
            unsigned long activate_vmx_preemption_time : 1;
            unsigned long process_posted_interrupts : 1;
        };
    };

    union PrimaryProcessorBasedVmxControls
    {
        unsigned long raw;

        struct
        {
            unsigned long : 2;
            unsigned long interrupt_window_exiting : 1;
            unsigned long use_tsc_offsetting : 1;
            unsigned long : 3;
            unsigned long hlt_exiting : 1;
            unsigned long : 1;
            unsigned long invlpg_exiting : 1;
            unsigned long mwait_exiting : 1;
            unsigned long rdpmc_exiting : 1;
            unsigned long rdtsc_exiting : 1;
            unsigned long : 2;
            unsigned long cr3_load_exiting : 1;
            unsigned long cr3_store_exiting : 1;
            unsigned long : 2;
            unsigned long cr8_load_exiting : 1;
            unsigned long cr8_store_exiting : 1;
            unsigned long use_tpr_shadow : 1;
            unsigned long nmi_window_exiting : 1;
            unsigned long mov_dr_exiting : 1;
            unsigned long unconditional_io_exiting : 1;
            unsigned long use_io_bitmaps : 1;
            unsigned long : 1;
            unsigned long monitor_trap_flag : 1;
            unsigned long use_msr_bitmaps : 1;
            unsigned long monitor_exiting : 1;
            unsigned long pause_exiting : 1;
            unsigned long activate_secondary_controls : 1;
        };
    };

    union SecondaryProcessorBasedVmxControls
    {
        unsigned long raw;

        struct
        {
            unsigned long virtualize_apic_accesses : 1;
            unsigned long enable_ept : 1;
            unsigned long descriptor_table_exiting : 1;
            unsigned long enable_rdtscp : 1;
            unsigned long virtualize_x2apic_mode : 1;
            unsigned long enable_vpid : 1;
            unsigned long wbinvd_exiting : 1;
            unsigned long unrestricted_guest : 1;
            unsigned long apic_register_virtualization : 1;
            unsigned long virtual_interrupt_delivery : 1;
            unsigned long pause_loop_exiting : 1;
            unsigned long rdrand_exiting : 1;
            unsigned long enable_invpcid : 1;
            unsigned long enable_vm_functions : 1;
            unsigned long vmcs_shadowing : 1;
            unsigned long enable_encls_exiting : 1;
            unsigned long rdseed_exiting : 1;
            unsigned long enable_pml : 1;
            unsigned long ept_violation_ve : 1;
            unsigned long conceal_vmx_from_pt : 1;
            unsigned long enable_xsaves_xrstors : 1;
            unsigned long : 1;
            unsigned long mode_based_execute_control_for_ept : 1;
            unsigned long sub_page_write_permissions_for_ept : 1;
            unsigned long intel_pt_uses_guest_physical_addresses : 1;
            unsigned long use_tsc_scaling : 1;
            unsigned long enable_user_wait_and_pause : 1;
            unsigned long : 1;
            unsigned long enable_enclv_exiting : 1;
        };
    };

    union VmExitControls
    {
        unsigned long raw;

        struct
        {
            unsigned long : 2;
            unsigned long save_debug_controls : 1;
            unsigned long : 6;
            unsigned long host_address_space_size : 1;
            unsigned long : 2;
            unsigned long load_ia32_perf_global_ctrl : 1;
            unsigned long : 2;
            unsigned long acknowledge_interrupt_on_exit : 1;
            unsigned long : 2;
            unsigned long save_ia32_pat : 1;
            unsigned long load_ia32_pat : 1;
            unsigned long save_ia32_efer : 1;
            unsigned long load_ia32_efer : 1;
            unsigned long save_vmx_preemption_timer_value : 1;
            unsigned long clear_ia32_bndcfgs : 1;
            unsigned long conceal_vmx_from_pt : 1;
            unsigned long clear_ia32_rtit_ctl : 1;
            unsigned long : 2;
            unsigned long load_cet_state : 1;
            unsigned long load_pkrs : 1;
        };
    };

    union VmEntryControls
    {
        unsigned long raw;

        struct
        {
            unsigned long : 2;
            unsigned long load_debug_controls : 1;
            unsigned long : 6;
            unsigned long ia32e_mode_guest : 1;
            unsigned long entry_to_smm : 1;
            unsigned long deactivate_dual_monitor_treatment : 1;
            unsigned long : 1;
            unsigned long load_ia32_perf_global_ctrl : 1;
            unsigned long load_ia32_pat : 1;
            unsigned long load_ia32_efer : 1;
            unsigned long load_ia32_bndcfgs : 1;
            unsigned long conceal_vmx_from_pt : 1;
            unsigned long load_ia32_rtit_ctl : 1;
            unsigned long : 1;
            unsigned long load_cet_state : 1;
            unsigned long : 1;
            unsigned long load_pkrs : 1;
        };
    };

    union ControlRegisterAccessExitQualification
    {
        unsigned long long raw;

        struct
        {
            unsigned long long cr_number : 4;
            unsigned long long access_type : 2;
            unsigned long long lmsw_operand_type : 1;
            unsigned long long : 1;
            unsigned long long mov_cr_gp_register : 4;
            unsigned long long : 4;
            unsigned long long lmsw_source_data : 16;
        };
    };

    union VmxEptp
    {
        unsigned long long raw;

        struct
        {
            unsigned long long type : 3;
            unsigned long long page_walk_length : 3;
            unsigned long long enable_accessed_dirty : 1;
            unsigned long long enable_supervisor_shadow_stack : 1;
            unsigned long long : 4;
            unsigned long long pfn : 40;
        };
    };

    union EptPml4e
    {
        unsigned long long raw;

        struct
        {
            unsigned long long read : 1;
            unsigned long long write : 1;
            unsigned long long supervisor_mode_execute : 1;
            unsigned long long : 5;
            unsigned long long accessed : 1;
            unsigned long long : 1;
            unsigned long long user_mode_execute : 1;
            unsigned long long : 1;
            unsigned long long pfn : 40;
        };
    };

    union EptPdpte
    {
        unsigned long long raw;

        struct
        {
            unsigned long long read : 1;
            unsigned long long write : 1;
            unsigned long long supervisor_mode_execute : 1;
            unsigned long long : 5;
            unsigned long long accessed : 1;
            unsigned long long : 1;
            unsigned long long user_mode_execute : 1;
            unsigned long long : 1;
            unsigned long long pfn : 40;
        };
    };

    union EptPde
    {
        unsigned long long raw;

        struct
        {
            unsigned long long read : 1;
            unsigned long long write : 1;
            unsigned long long supervisor_mode_execute : 1;
            unsigned long long : 5;
            unsigned long long accessed : 1;
            unsigned long long : 1;
            unsigned long long user_mode_execute : 1;
            unsigned long long : 1;
            unsigned long long pfn : 40;
        };
    };

    union EptLargePde
    {
        unsigned long long raw;

        struct
        {
            unsigned long long read : 1;
            unsigned long long write : 1;
            unsigned long long supervisor_mode_execute : 1;
            unsigned long long type : 3;
            unsigned long long ignore_pat : 1;
            unsigned long long must_be_1 : 1;
            unsigned long long accessed : 1;
            unsigned long long dirty : 1;
            unsigned long long user_mode_execute : 1;
            unsigned long long : 10;
            unsigned long long pfn : 31;
            unsigned long long : 8;
            unsigned long long supervisor_shadow_stack : 1;
            unsigned long long : 2;
            unsigned long long suppress_ve : 1;
        };
    };

    union EptPte
    {
        unsigned long long raw;

        struct
        {
            unsigned long long read : 1;
            unsigned long long write : 1;
            unsigned long long supervisor_mode_execute : 1;
            unsigned long long type : 3;
            unsigned long long ignore_pat : 1;
            unsigned long long : 1;
            unsigned long long accessed : 1;
            unsigned long long dirty : 1;
            unsigned long long user_mode_execute : 1;
            unsigned long long : 1;
            unsigned long long pfn : 40;
            unsigned long long : 8;
            unsigned long long supervisor_shadow_stack : 1;
            unsigned long long sub_page_write_permissions : 1;
            unsigned long long : 1;
            unsigned long long suppress_ve : 1;
        };
    };

    inline constexpr size_t EPT_ENTRY_COUNT = 512;

    enum class MtrrType
    {
        UC = 0,
        WC = 1,
        WT = 4,
        WP = 5,
        WB = 6
    };

    union Ia32MtrrCap
    {
        unsigned long long raw;
        
        struct
        {
            unsigned long long vcnt : 8;
            unsigned long long fix : 1;
            unsigned long long : 1;
            unsigned long long wc : 1;
            unsigned long long smrr : 1;
        };
    };

    union Ia32MtrrDefType
    {
        unsigned long long raw;

        struct
        {
            unsigned long long type : 8;
            unsigned long long : 2;
            unsigned long long fe : 10;
            unsigned long long e : 11;
        };
    };

    enum class VmExitReason : unsigned long
    {
        EXCEPTION_OR_NMI,
        EXTERNAL_INTERRUPT,
        TRIPLE_FAULT,
        INIT_SIGNAL,
        SIPI,
        IO_SMI,
        OTHER_SMI,
        INTERRUPT_WINDOW,
        NMI_WINDOW,
        TASK_SWITCH,
        CPUID,
        GETSEC,
        HLT,
        INVD,
        INVLPG,
        RDPMC,
        RDTMC,
        RSM,
        VMCALL,
        VMCLEAR,
        VMLAUNCH,
        VMPTRLD,
        VMPTRST,
        VMREAD,
        VMRESUME,
        VMWRITE,
        VMXOFF,
        VMXON,
        CR_ACCESS,
        MOV_DR,
        IO_INSTRUCTION,
        RDMSR,
        WRMSR,
        VM_ENTRY_FAILURE_GUEST_STATE,
        VM_ENTRY_FAILURE_MSR_LOADING,
        MWAIT = 36,
        MTF,
        MONITOR = 39,
        PAUSE,
        VM_ENTRY_FAILURE_MACHINE_CHECK_EVENT,
        TPR_BELOW_THRESHOLD = 43,
        APIC_ACCESS,
        VIRTUALIZED_EOI,
        ACCESS_TO_GDTR_IDTR,
        ACESS_TO_LDTR_TR,
        EPT_VIOLATION,
        EPT_MISCONFIGURATION,
        INVEPT,
        RDTSCP,
        VMX_PREEMPTION_TIME_EXPIRED,
        INVVPID,
        WBINVD,
        XSETBV,
        APIC_WRITE,
        RDRAND,
        INVPCID,
        VMFUNC,
        ENCLS,
        RDSEED,
        PML_FULL,
        XSAVES,
        XRSTORS,
        SPP_RELATED_EVENT,
        UMWAIT,
        TPAUSE,
        MAX
    };

    union VmExitInterruptionInformation
    {
        unsigned long raw;

        struct
        {
            unsigned long vector : 8;
            unsigned long type : 3;
            unsigned long error_code_valid : 1;
            unsigned long nmi_unblocking_due_to_iret : 1; // check
            unsigned long : 18;
            unsigned long valid : 1;
        };
    };

    union IdtVectoringInformation
    {
        unsigned long raw;

        struct
        {
            unsigned long vector : 8;
            unsigned long type : 3;
            unsigned long error_code_valid : 1;
            unsigned long : 19;
            unsigned long valid : 1;
        };
    };

    union VmEntryInterruptionInformation
    {
        unsigned long raw;

        struct
        {
            unsigned long vector : 8;
            unsigned long type : 3;
            unsigned long deliver_error_code : 1;
            unsigned long : 19;
            unsigned long valid : 1;
        };
    };

    auto get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base) -> unsigned long long;
    auto get_segment_access_rights(unsigned short segment_selector) -> SegmentAccessRights;

    auto is_km_address(unsigned long long address) -> bool;
    auto is_um_address(unsigned long long address) -> bool;
}

#pragma pack(pop)
