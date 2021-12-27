
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_print	equ 1
_NR_sleep 	equ 2
_NR_wait    equ 3
_NR_signal 	equ 4
_NR_clear 	equ 5

INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global print
global sleep
global _wait
global _signal
global clear

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

print:
	mov eax, _NR_print
	mov ebx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret

sleep:
	mov eax, _NR_sleep
	mov ebx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret

_wait:
	mov eax, _NR_wait
	mov ebx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret

_signal:
	mov eax, _NR_signal
	mov ebx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret
clear:
	mov eax, _NR_clear
	mov ebx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret
	
