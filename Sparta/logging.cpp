#include "logging.h"
#include "vmx.h"

#include <ntddk.h>

void logging::dump_vmcs_guest_state_area()
{
	KdPrint(("VMCS_GUEST_ES_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_ES_SELECTOR).value));
	KdPrint(("VMCS_GUEST_CS_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_CS_SELECTOR).value));
	KdPrint(("VMCS_GUEST_SS_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_SS_SELECTOR).value));
	KdPrint(("VMCS_GUEST_DS_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_DS_SELECTOR).value));
	KdPrint(("VMCS_GUEST_FS_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_FS_SELECTOR).value));
	KdPrint(("VMCS_GUEST_GS_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_GS_SELECTOR).value));
	KdPrint(("VMCS_GUEST_LDTR_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_LDTR_SELECTOR).value));
	KdPrint(("VMCS_GUEST_TR_SELECTOR: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_TR_SELECTOR).value));
	KdPrint(("VMCS_GUEST_INTERRUPT_STATUS: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_INTERRUPT_STATUS).value));
	KdPrint(("VMCS_GUEST_PML_INDEX: 0x%hx\n", vmx::vmread<unsigned short>(intel::VmcsField::VMCS_GUEST_PML_INDEX).value));

	KdPrint(("VMCS_GUEST_PHYSICAL_ADDRESS: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PHYSICAL_ADDRESS).value));
	KdPrint(("VMCS_GUEST_VMCS_LINK_POINTER: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_VMCS_LINK_POINTER).value));
	KdPrint(("VMCS_GUEST_DEBUGCTL: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_DEBUGCTL).value));
	KdPrint(("VMCS_GUEST_PAT: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PAT).value));
	KdPrint(("VMCS_GUEST_EFER: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_EFER).value));
	KdPrint(("VMCS_GUEST_PERF_GLOBAL_CTRL: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PERF_GLOBAL_CTRL).value));
	KdPrint(("VMCS_GUEST_PDPTE0: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PDPTE0).value));
	KdPrint(("VMCS_GUEST_PDPTE1: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PDPTE1).value));
	KdPrint(("VMCS_GUEST_PDPTE2: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PDPTE2).value));
	KdPrint(("VMCS_GUEST_PDPTE3: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PDPTE3).value));

	KdPrint(("VMCS_GUEST_ES_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_ES_LIMIT).value));
	KdPrint(("VMCS_GUEST_CS_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_CS_LIMIT).value));
	KdPrint(("VMCS_GUEST_SS_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_SS_LIMIT).value));
	KdPrint(("VMCS_GUEST_DS_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_DS_LIMIT).value));
	KdPrint(("VMCS_GUEST_FS_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_FS_LIMIT).value));
	KdPrint(("VMCS_GUEST_GS_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_GS_LIMIT).value));
	KdPrint(("VMCS_GUEST_LDTR_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_LDTR_LIMIT).value));
	KdPrint(("VMCS_GUEST_TR_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_TR_LIMIT).value));
	KdPrint(("VMCS_GUEST_GDTR_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_GDTR_LIMIT).value));
	KdPrint(("VMCS_GUEST_IDTR_LIMIT: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_IDTR_LIMIT).value));
	KdPrint(("VMCS_GUEST_ES_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_ES_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_CS_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_CS_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_SS_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_SS_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_DS_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_DS_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_FS_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_FS_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_GS_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_GS_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_LDTR_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_LDTR_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_TR_ACCESS_RIGHTS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_TR_ACCESS_RIGHTS).value));
	KdPrint(("VMCS_GUEST_INTERRUPTIBILITY_STATE: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_INTERRUPTIBILITY_STATE).value));
	KdPrint(("VMCS_GUEST_ACTIVITY_STATE: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_ACTIVITY_STATE).value));
	KdPrint(("VMCS_GUEST_SMBASE: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_SMBASE).value));
	KdPrint(("VMCS_GUEST_SYSENTER_CS: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_SYSENTER_CS).value));
	KdPrint(("VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE: 0x%lx\n", vmx::vmread<unsigned long>(intel::VmcsField::VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE).value));

	KdPrint(("VMCS_GUEST_CR0: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_CR0).value));
	KdPrint(("VMCS_GUEST_CR3: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_CR3).value));
	KdPrint(("VMCS_GUEST_CR4: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_CR4).value));
	KdPrint(("VMCS_GUEST_ES_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_ES_BASE).value));
	KdPrint(("VMCS_GUEST_CS_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_CS_BASE).value));
	KdPrint(("VMCS_GUEST_SS_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_SS_BASE).value));
	KdPrint(("VMCS_GUEST_DS_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_DS_BASE).value));
	KdPrint(("VMCS_GUEST_FS_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_FS_BASE).value));
	KdPrint(("VMCS_GUEST_GS_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_GS_BASE).value));
	KdPrint(("VMCS_GUEST_LDTR_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_LDTR_BASE).value));
	KdPrint(("VMCS_GUEST_TR_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_TR_BASE).value));
	KdPrint(("VMCS_GUEST_GDTR_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_GDTR_BASE).value));
	KdPrint(("VMCS_GUEST_IDTR_BASE: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_IDTR_BASE).value));
	KdPrint(("VMCS_GUEST_DR7: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_DR7).value));
	KdPrint(("VMCS_GUEST_RSP: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RSP).value));
	KdPrint(("VMCS_GUEST_RIP: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RIP).value));
	KdPrint(("VMCS_GUEST_RFLAGS: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_RFLAGS).value));
	KdPrint(("VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS).value));
	KdPrint(("VMCS_GUEST_SYSENTER_ESP: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_SYSENTER_ESP).value));
	KdPrint(("VMCS_GUEST_SYSENTER_EIP: 0x%llx\n", vmx::vmread<unsigned long long>(intel::VmcsField::VMCS_GUEST_SYSENTER_EIP).value));
}
