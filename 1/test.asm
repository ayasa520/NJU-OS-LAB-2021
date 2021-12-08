; nasm -felf64 test.asm && ld test.o && ./a.out 
sys_read  equ 0
sys_write equ 1
stdout    equ 1 
stdin     equ 0
sys_exit  equ 60

section .data
    PROMPT: db "Please input x anf y :", 0xA


; .bss 放未被显式初始化的全局变量和静态变量
section .bss ;
    string: RESB 64
    bits_of_a: RESB 32
    bits_of_b: RESB 32
    bits_of_result: RESB 32

section .text
    global main
main:
    mov rbp, rsp; for correct debugging
    call print_prompt
    call get_numbers
    call my_add
    jmp exit

get_numbers:
    mov rax, sys_read
    mov rdi, stdin
    mov rdx, 64
    mov rsi, string
    syscall
    
    xor rax, rax
.get_numbers_loop:
    cmp byte[rsi+rax],0x20
    je .get_numbers_loop_break
    inc rax
    jmp .get_numbers_loop
.get_numbers_loop_break:
    push rax
    mov rdi, rax
    mov rdx, bits_of_a
    
    call strcpy
    pop rax
    add rsi, rax
    inc rsi
    xor rax, rax    
.second_loop:
    inc rax
    cmp byte[rsi+rax],0x00
    jne .second_loop    
    mov rdi, rax
    mov rdx, bits_of_b

    call strcpy
    
    ret
 ; rdi:a rsi:b
 my_add:
    mov rdi, bits_of_a
    mov rsi, bits_of_b
    xor rcx, rcx
    xor al,al
    xor ah,ah   ; 标志进位
.add_loop:
    cmp rcx, 0x20
    je .end_my_add
    cmp byte[rdi+rcx], 0x00
    jne  .add_32
    mov byte[rdi+rcx], 0x30
.add_32:
    cmp byte[rsi+rcx], 0x00
    jne .add_continue
    mov byte[rsi+rcx], 0x30
.add_continue:
    mov al,byte[rdi+rcx]
    add al,byte[rsi+rcx]
    add al,ah
    sub al, 0x30
    sub al, 0x30
    cmp al, 0x0A
    js .smaller_than_ten 
    mov ah, 0x1
    sub al, 0x0A
    jmp .continue_add
.smaller_than_ten:
    xor ah,ah
.continue_add:
    mov [bits_of_result+ecx], al
    inc rcx
    jmp .add_loop

    
.end_my_add:
    ret
    
strcpy:
    mov r8, rdi    ; rdi 存储着数字的位数(10进制), rdx 存储
.strcpy_loop:
    mov al, byte[rsi+r8-1]
    push r8
    add r8, 31
    sub r8, rdi
    neg r8
    add r8, 31
    mov byte[rdx+r8],al
    pop r8
    dec r8
    cmp r8,0x00
    jne .strcpy_loop  
    
ret




print_prompt:
    mov rdi, PROMPT
    push rdi
    call strlen
    mov rdx, rax ; rdx 字符串的长度
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    pop rsi ; 要输出的字符串的地址
    syscall
    ret 


; 获取字符串的长度 rdi 接收参数, 返回值放在 rax 里 
strlen:
    xor rax, rax ;将 rax 归零

    .strlen_loop:
    cmp byte[rdi+rax], 0xA
    je .strlen_end
    inc rax
    jmp .strlen_loop

    .strlen_end:
    inc rax ; 再加上一个换行
    ret 

exit:
    mov       rax, sys_exit          ; 60: sys_exit
    xor       rdi,rdi                ; 退出代码 0
    syscall                           ; 调用操作系统退出


    ; 32 位下这样用，64 位下要用的话,就不能使用 rax 等等而应该使用 eax
    ; ; exit(0)
    ; mov eax, 1
    ; mov ebx, 0
    ; int 80h

