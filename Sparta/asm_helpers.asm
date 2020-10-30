.code

extern vmexit_handler:PROC

_get_es_selector PROC
	mov ax, es
	ret
_get_es_selector ENDP

_get_cs_selector PROC
	mov ax, cs
	ret
_get_cs_selector ENDP

_get_ss_selector PROC
	mov ax, ss
	ret
_get_ss_selector ENDP

_get_ds_selector PROC
	mov ax, ds
	ret
_get_ds_selector ENDP

_get_fs_selector PROC
	mov ax, fs
	ret
_get_fs_selector ENDP

_get_gs_selector PROC
	mov ax, gs
	ret
_get_gs_selector ENDP

_get_ldtr_selector PROC
	sldt ax
	ret
_get_ldtr_selector ENDP

_get_tr_selector PROC
	str ax
	ret
_get_tr_selector ENDP

_get_segment_access_rights PROC
	lar rax, rcx
	ret
_get_segment_access_rights ENDP

_get_rsp PROC
	mov rax, rsp
	ret
_get_rsp ENDP

_vmexit_handler PROC
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rbp
	push rbx
	push rdx
	push rcx
	push rax

	mov rcx, rsp

	sub rsp, 28h
	call vmexit_handler
	add rsp, 28h

	pop rax
	pop rcx
	pop rdx
	pop rbx
	pop rbp
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15

	sub rsp, 100h
	jmp _vmresume
_vmexit_handler ENDP

_vmresume PROC
	vmresume
_vmresume ENDP

_end_initialize_vmx PROC
	cli

cpuid_loop:
	mov eax, 0
	cpuid
	jmp cpuid_loop
_end_initialize_vmx ENDP

END
