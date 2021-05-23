import DllInjectionImage from '../assets/dll-injection.png';
import RemoteThreadCreationImage from '../assets/remote-thread-creation.jpg';
import SystemTokenStealingImage from '../assets/system-token-stealing.png';
import SmepBypassImage from '../assets/smep-bypass.jpg';
import KernelStructureCorruptionImage from '../assets/kernel-structure-corruption.png';
import ApcInjectionImage from '../assets/apc-injection.png';

const DLL_INJECTION_CODE = `HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, ...);
LPVOID remote_buffer = VirtualAllocEx(process_handle, ..., sizeof(dll_path), ..., PAGE_READWRITE);	
WriteProcessMemory(process_handle, remote_buffer, dll_path, ...);
HANDLE thread_handle = CreateRemoteThread(process_handle, ..., (LPTHREAD_START_ROUTINE) LoadLibraryW, remote_buffer, ...);
WaitForSingleObject(thread_handle, INFINITE);
VirtualFreeEx(process_handle, remote_buffer, ...);`;

const REMOTE_THREAD_CREATION_CODE = `HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, ...);
LPVOID remote_shellcode = VirtualAllocEx(process_handle, ..., sizeof(shellcode), ..., PAGE_EXECUTE_READWRITE);
WriteProcessMemory(process_handle, remote_shellcode, shellcode, ...);
HANDLE thread_handle = CreateRemoteThread(process_handle, ..., (LPTHREAD_START_ROUTINE) remote_shellcode, ...);
WaitForSingleObject(thread_handle, INFINITE);
VirtualFreeEx(process_handle, remote_shellcode, ...);`;

const SYSTEM_TOKEN_STEALING_CODE = `mov r13, 0x358
mov r14, [rbp - 0x10]   ; System EPROCESS
mov r14, [r14 + r13]    ; System TOKEN
and r14, 0xfffffffffffffff0
mov r12, [rbp - 0x8]    ; Attacker's EPROCESS
mov r15, [r12 + r13]    ; Attacker's TOKEN
and r15, 0x07
or r14, r15
mov [r12 + r13], r14    ; Override Attacker's TOKEN`;

const SMEP_BYPASS_CODE = `push rax
mov rax, cr4
and rax, 0xffffffffffefffff
mov cr4, rax
pop rax
ret`;

const KERNEL_STRUCTURE_CORRUPTION_CODE = `/* disable WP bit in CR0 to enable writing to SSDT */
disable_write_protect();

/* identify the address of SSDT table */
PLONG ssdt = KeServiceDescriptorTable.ServiceTable;

/* identify ‘syscall’ index into the SSDT table */
UINT32 index = *((PULONG)(syscall + 0x1));

/* get the address of the service routine in SSDT */
PLONG target = (PLONG)&(ssdt[index]);

/* hook the service routine in SSDT */
return (PUCHAR)InterlockedExchange(target, hookaddr);`;

const APC_INJECTION_CODE = `HANDLE target_process_handle = OpenProcess(PROCESS_ALL_ACCESS, ...);
LPVOID target_process_buffer = VirtualAllocEx(target_process_handle, ..., sizeof(shellcode), ..., PAGE_READWRITE);
WriteProcessMemory(target_process_handle, target_process_buffer, shellcode, ...);
VirtualProtectEx(target_process_handle, target_process_buffer, ..., PAGE_EXECUTE_READ, ...);
HANDLE target_thread_handle = OpenThread(THREAD_ALL_ACCESS, ...);
QueueUserAPC(target_process_buffer, target_thread_handle, ...);`;

const TECHNIQUES = [
    {
        icon: "mdi-needle",
        id: "dll-injection",
        name: "DLL Injection",
        description: "A technique for injecting code into another process by forcing it to load a dynamically linked library",
        image: DllInjectionImage,
        explanation: "DLL injection is a technique used for running code within the address space of another process by forcing it to load a dynamic-link library. DLL injection is often used by external programs to influence the behavior of another program in a way its authors did not anticipate or intend. For example, the injected code could hook system function calls, or read the contents of password textboxes, which cannot be done the usual way. A program used to inject arbitrary code into arbitrary processes is called a DLL injector.",
        code: DLL_INJECTION_CODE,
    },
    {
        icon: "mdi-cctv",
        id: "remote-thread-creation",
        name: "Remote Thread Creation",
        description: "A technique for running code in another process by creating a new thread remotely",
        image: RemoteThreadCreationImage,
        explanation: "There are some primary differences between DLL injection and code injection; however, they are both achieved in the same manner: remote thread creation. The Win32 API comes preloaded with a function to do just that, CreateRemoteThreadO,1 which is exported from kernel32.dll. This function allows creating threads in other processes remotely, thus achieving code execution in other processes. This gives attackers almost full control of usermode processes and hence, it's the basic building block for many code injection techniques, and not just DLL Injection. Sparta monitors it as well in order to detect sophisticated code injection techniques, like raw shellcode injection.",
        code: REMOTE_THREAD_CREATION_CODE,
    },
    {
        icon: "mdi-account-lock",
        id: "system-token-stealing",
        name: "SYSTEM Token Stealing",
        description: "A privilege escalation technique based on stealing a SYSTEM process access token",
        image: SystemTokenStealingImage,
        explanation: "As a part of Windows, there is something known as the SYSTEM process. The SYSTEM process, PID of 4, houses the majority of kernel mode system threads. The threads stored in the SYSTEM process, only run in context of kernel mode. In Windows, each process object, known as _EPROCESS, has something known as an access token. This access token determines the security context of a process or a thread. Since the SYSTEM process houses execution of kernel mode code, it will need to run in a security context that allows it to access the kernel. This would require system or administrative privilege. This is why the goal of various kernel LPE exploits will be to identify the access token value of the SYSTEM process and copy it to a process that the ttacker controls. From there, he can spawn cmd.exe from the now privileged process, which will grant him NT AUTHORITY\\SYSTEM privileged code execution.",
        code: SYSTEM_TOKEN_STEALING_CODE,
    },
    {
        icon: "mdi-debug-step-over",
        id: "smep-bypass",
        name: "SMEP Bypass",
        description: "A technique for exploiting kernel-mode vulnerabilities by running shellcode residing in userland",
        image: SmepBypassImage,
        explanation: "Starting with Windows 8, Microsoft implemented a new mitigation by default called Supervisor Mode Execution Prevention (SMEP). SMEP detects kernel mode code running in userspace and stops attackers from being able to hijack execution in the kernel and send it to their shellcode pointer residing in userspace. This mitigation can be bypassed using a kernel ROP chain which disables the SMEP bit in the CR4 register.",
        code: SMEP_BYPASS_CODE,
    },
    {
        icon: "mdi-anchor",
        id: "kernel-structure-corruption",
        name: "Kernel Structure Corruption",
        description: "A kernel-mode rootkit technique for getting full control of the machine by corrupting its internal structures",
        image: KernelStructureCorruptionImage,
        explanation: "The kernel holds many important structures which are used for saving runtime operation information. For example, the IDT saves the interrupt descriptors for each vector, and the SSDT saves the syscall handlers for each index. Many kernel-mode rootkits corrupt these structures in order to gain full control of the system and intercept various low-level system-wide operations. Sparta detects these corruptions in order to prevent kernel-mode rootkit infection.",
        code: KERNEL_STRUCTURE_CORRUPTION_CODE,
    },
    {
        icon: "mdi-needle",
        id: "apc-injection",
        name: "APC Injection",
        description: "A technique for running code in another process by queueing an APC to one of its thread",
        image: ApcInjectionImage,
        explanation: "Adversaries may inject malicious code into processes via the asynchronous procedure call (APC) queue in order to evade process-based defenses as well as possibly elevate privileges. APC injection is a method of executing arbitrary code in the address space of a separate live process. APC injection is commonly performed by attaching malicious code to the APC Queue of a process's thread. Queued APC functions are executed when the thread enters an alterable state. A handle to an existing victim process is first created with native Windows API calls such as OpenThread. At this point QueueUserAPC can be used to invoke a function (such as LoadLibrayA pointing to a malicious DLL). A variation of APC injection, dubbed \"Early Bird injection\", involves creating a suspended process in which malicious code can be written and executed before the process' entry point (and potentially subsequent anti-malware hooks) via an APC. AtomBombing is another variation that utilizes APCs to invoke malicious code previously written to the global atom table. Running code in the context of another process may allow access to the process's memory, system/network resources, and possibly elevated privileges. Execution via APC injection may also evade detection from security products since the execution is masked under a legitimate process.",
        code: APC_INJECTION_CODE,
    },
];

export default class TechniquesService {
    static getAllTechniques() {
        return TECHNIQUES;
    }

    static getTechnique(id) {
        for (const technique of TECHNIQUES) {
            if (technique.id == id) {
                return technique;
            }
        }

        return null;
    }
}
