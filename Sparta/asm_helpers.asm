public _get_es_selector
public _get_cs_selector
public _get_ss_selector
public _get_ds_selector
public _get_fs_selector
public _get_gs_selector
public _get_ldtr_selector
public _get_tr_selector

.code

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

END
