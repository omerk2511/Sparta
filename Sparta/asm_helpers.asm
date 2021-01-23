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

_invept PROC
	invept rcx, oword ptr [rdx]

	jz invept_error
	jc invept_error

	mov rax, 1		; true - success
	ret

invept_error:
	xor rax, rax	; false - failure
	ret
_invept ENDP

_vmexit_handler PROC
	push r15
	mov r15, cr2
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
	push rsp

	mov rcx, rsp

	sub rsp, 28h
	call vmexit_handler
	add rsp, 30h

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
	mov cr2, r15
	pop r15

	sub rsp, 100h
	jmp _vmresume
_vmexit_handler ENDP

_vmresume PROC
	vmresume
_vmresume ENDP

_restore_guest PROC
	cli

	mov rcx, rsp
	sub rcx, 0ff8h

	movaps  xmm0, [rcx+1a0h]
    movaps  xmm1, [rcx+1b0h]
    movaps  xmm2, [rcx+1c0h]
    movaps  xmm3, [rcx+1d0h]
    movaps  xmm4, [rcx+1e0h]
    movaps  xmm5, [rcx+1f0h]
    movaps  xmm6, [rcx+200h]
    movaps  xmm7, [rcx+210h]
    movaps  xmm8, [rcx+220h]
    movaps  xmm9, [rcx+230h]
    movaps  xmm10, [rcx+240h]
    movaps  xmm11, [rcx+250h]
    movaps  xmm12, [rcx+260h]
    movaps  xmm13, [rcx+270h]
    movaps  xmm14, [rcx+280h]
    movaps  xmm15, [rcx+290h]
    ldmxcsr [rcx+34h]

    mov rax, [rcx+78h]
    mov rdx, [rcx+88h]
    mov r8, [rcx+0b8h]
    mov r9, [rcx+0c0h]
    mov r10, [rcx+0c8h]
    mov r11, [rcx+0d0h]

    mov rbx, [rcx+90h]
    mov rsi, [rcx+0a8h]
    mov rdi, [rcx+0b0h]
    mov rbp, [rcx+0a0h]
    mov r12, [rcx+0d8h]
    mov r13, [rcx+0e0h]
    mov r14, [rcx+0e8h]
    mov r15, [rcx+0f0h]

    push [rcx+44h]
    popfq
    mov rsp, [rcx+98h]
    push [rcx+0f8h]
    mov rcx, [rcx+80h]

	sti

    ret
_restore_guest ENDP

_invd PROC
	invd
	ret
_invd ENDP

END
