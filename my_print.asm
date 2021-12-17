; nasm -f elf64 my_print.asm -o func.o&&gcc -m64 test.cpp func.o -o test -no-pie && ./test
sys_read  equ 0
sys_write equ 1
stdout    equ 1 
stdin     equ 0
sys_exit  equ 60
global _print

section .data
    color_red:   db       `\033[31m`,0, 0
    .len            equ $ - color_red
    color_default:  db  `\033[0m`, 0, 0
    .len            equ $ - color_default

section .text


; 传参顺序: rdi，rsi，rdx，rcx，r8，r9
; void print(char* s, int lenl);
_print:
    
    mov rdx, rsi
    mov rsi, rdi

    call print
    
    
    ret
; rsi: 字符串地址
; rdx: 字符串长度
;
print:
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    syscall
    push rsi
    push rdx
    mov rax, sys_write
    mov rdi, stdout
    mov rsi, color_default
    mov rdx, color_default.len
    syscall
    pop rdx
    pop rsi
    ret
