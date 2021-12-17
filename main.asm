; nasm x86-64
; nasm -felf64 main.asm && ld main.o && ./a.out 

sys_read  equ 0
sys_write equ 1
stdout    equ 1 
stdin     equ 0
sys_exit  equ 60

section .data
    PROMPT: db "Please input x anf y :", 0xA
    neg_a: db 0x00 ; 正负标志 1/0
    neg_b: db 0x00 ; 正负标志 1/0rr
    result_sign: db 0x00 ; 结果的正负 1/-1
    huanhang: db "", 0xa
    dash: db "-"
    zero: db "0"
; .bss 放未被显式初始化的全局变量和静态变量
section .bss ;
    string: RESB 64
    bits_of_a: RESB 32
    bits_of_b: RESB 32
    add_result_part_one: RESB 32
    add_result_part_two: RESB 32
    mul_ac: RESQ 2                      
    mul_bd: RESQ 2
    mul_abcd: RESQ 2       
    dec_num_ac: RESB 64     ;十进制 ac
    dec_num_bd: RESB 64      ; 十进制 bd
    dec_num_c: RESB 64      ; 十进制 bc+ad
    mul_result: RESB 64     ;ac*10^22+bd+(ad+bc)*10^11  最终结果倒序
;    bits_of_result: RESB 32

section .text
    global _start
_start:
    mov rbp, rsp; for correct debugging
    mov ebp, esp; for correct debugging
    mov rbp, rsp; for correct debugging

    call print_prompt
    call get_numbers    ;return r11:r12

;   call base_conversion
    push rbx
    push rcx
    push rsi
    push rdi
    call my_add ;return r11:r12
    call print_add_result 
    pop rdi
    pop rsi
    pop rcx
    pop rbx
    call my_mul
    call print_mul_result
    jmp exit
    
    
 
; 打印加法结果
print_add_result:
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
     mov rsi, -1
    mov rcx, 0xa
    mov r14, 0x1 
    cmp byte[result_sign], -1
    jne .high_loop
    
    mov rdx, 0x1
    mov rsi,result_sign 
    mov byte[rsi],0x2d 
    push rcx
    syscall
    pop rcx
    mov rsi, -1
    
.high_loop:

    cmp rcx, 0x0
    jl .low_loop_start
    
    cmp byte[add_result_part_one+rcx], 0x0
    je .high_is_print
    
    jmp .high_print
.high_is_print:
    cmp rsi, -1
    je .high_loop_zero
.high_print:

    mov rdx, 0x1
    mov rsi,add_result_part_one 
    add rsi, rcx
    add byte[rsi], '0'
    push rcx
    syscall
    pop rcx
    dec rcx
    jmp .high_loop
.high_loop_zero:
    dec rcx 
    jmp .high_loop
.low_loop_start:
    mov rcx, 0xa
.low_loop:
    cmp rcx, 0x0
    jl .end_print_add_result
    cmp byte[add_result_part_two+rcx], 0x0
    je .low_is_print

    jmp .low_print
.low_is_print:
    cmp rsi, -1
    je .low_loop_zero
.low_print:
    
    mov rdx, 0x1
    mov rsi,add_result_part_two
    add rsi, rcx
    add byte[rsi], '0'
    push rcx
    syscall
    pop rcx
    dec rcx
    jmp .low_loop
.low_loop_zero:
    dec rcx 
    jmp .low_loop
    
.end_print_add_result:
    cmp rsi , -1
    jne .true_end_print 
    mov rsi,add_result_part_two
    mov byte[rsi], '0'
    mov rdx, 0x1
    syscall 
    
.true_end_print:
    
    ret


print_mul_result:
    mov r8, 0x0
  
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    mov rdx, 0x1
    mov rsi, huanhang
    syscall
    mov rcx, 63
    
    mov al, byte[neg_a]
    mov ah, byte[neg_b]
    xor al, ah
    cmp al, 0x1
    jne .print_mul_loop
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    mov rsi, dash
    mov rdx, 0x1
    syscall
    mov rcx, 63

.print_mul_loop:
    cmp rcx, 0
    jl .end_print_mul_result
    cmp byte[mul_result+rcx],0x0
    je .is_skip
    jmp .normal_print_mul    
.is_skip:
    cmp r8, 0x0
    je .print_mul_skip0
    jmp .normal_print_mul
.print_mul_skip0:
    dec rcx
    jmp .print_mul_loop
.normal_print_mul:
    mov r8, 0x01
    mov rsi, mul_result
    add rsi, rcx
    add byte[rsi], 0x30
    mov rdx, 0x1
    push rcx
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    syscall
    pop rcx
    dec rcx
    jmp .print_mul_loop
.end_print_mul_result:
    cmp r8, 0x0
    jne .mul_ret
    mov rdx, 0x01
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    mov rsi, zero
    syscall

.mul_ret:
    mov rsi, huanhang
    mov rax, sys_write ; rax sys_write  
    mov rdi, stdout ;stdout
    mov rdx, 0x01
    syscall
    ret    
    
    
    
; one rsi:rdi
; another rcx:rbx 
; ac, bd, (a+b)(c+d)-ac-bd=>ad+bc
; ac*10^22+bd+(ad+bc)*10^11 为最终答案
; mul 结果: 高 64 位存入 rdx, 低 64 位存入 rax

my_mul:
    cmp byte[neg_a], 1
    je .change_a
    jmp .exam_b
.change_a:
    neg rsi
    neg rdi
 
.exam_b:   
    cmp byte[neg_b], 1
    je .change_b
    jmp .mul_end_change
.change_b:
    neg rcx
    neg rbx
.mul_end_change:
    xor rdx,rdx 
    mov rax, rsi 
    mul rcx
    mov qword[mul_ac], rax ;低位
    mov qword[mul_ac+8], rdx ; 高位
    
    xor rdx,rdx
    mov rax, rdi
    mul rbx
    mov qword[mul_bd], rax ;低位
    mov qword[mul_bd+8], rdx ;高位
    
    xor rdx,rdx
    mov r8, rsi
    add r8, rdi
    mov r9, rcx
    add r9, rbx
    mov rax, r8
    mul r9
    ; mov qword[mul_abcd], rax ;低位
    ; mov qword[mul_abcd+8], rdx ;高位
    
    ; ac, *10^22, 这个直接转10进制字符串保留就行
    mov r10, qword[mul_ac+8]       ;高位 
    mov r11, qword[mul_ac]         ;低位 
    sub rax, r11
    sub rdx, r10
    
    ; bd
    mov r10, qword[mul_bd+8]       ;高位 
    mov r11, qword[mul_bd]         ;低位 
    sub rax, r11
    sub rdx, r10
    
    ; ad + bc, *10^11
    mov qword[mul_abcd], rax ;低位
    mov qword[mul_abcd+8], rdx ;高位
    
    
    mov rsi, qword[mul_ac+8] 
    mov rdi, qword[mul_ac]
    mov r13, dec_num_ac
    call bin_to_dec
    
    mov rsi, qword[mul_bd+8] 
    mov rdi, qword[mul_bd]
    mov r13, dec_num_bd
    call bin_to_dec
    
    mov rsi, qword[mul_abcd+8] 
    mov rdi, qword[mul_abcd]
    mov r13, dec_num_c
    call bin_to_dec


    xor rcx, rcx
    dec rcx ; -1
.final_add_loop:
   inc rcx  
.mul_add_bd:
    cmp rcx, 0x20
    jge .mul_add_adbc
    mov al, byte[dec_num_bd+rcx]
    add byte[mul_result+rcx], al

.mul_add_adbc:
    cmp rcx, 0xB
    jl .mul_add_ac
    cmp rcx, 0x2b
    jge .mul_add_ac
    mov al, byte[dec_num_c+rcx-11]
    add byte[mul_result+rcx], al

.mul_add_ac:
    cmp rcx, 0x16
    jl .final_add_loop
    cmp rcx, 0x36
    jge .mul_add_end
  
    mov al, byte[dec_num_ac+rcx-22]
    add byte[mul_result+rcx], al
    jmp .final_add_loop
.mul_add_end: 
.mul_carry:
    xor rcx, rcx
   
    mov rbx, 0xA
.my_carry_loop:
    xor rax, rax 
    cmp rcx, 0x40
    jge .my_mul_end
    mov al, byte[mul_result+rcx]
    div bl ; ah 余数 al商
    add byte[mul_result+rcx+1], al
    mov byte[mul_result+rcx], ah
    inc rcx
    jmp .my_carry_loop
.my_mul_end:
    ret 
; a rsi:rdi
; b rcx:rbx 
my_add:
    mov r11, rsi
    mov r12, rdi
    
    add r11, rcx
    add r12, rbx
    
    cmp r11, 0x0    ; 高位寄存器相加结果判
    
   
    jl .is_diff     ; 小于零, 跳走
    push rax
    mov al, 0x1
    mov byte[result_sign], al  ;不小于零, 置为1
    pop rax
    cmp r11, 0x0
    je .high_is_zero
    
.is_diff:  
    mov r14, 0x1
    mov r15, 0x1
    
    cmp r12, 0x0
    jnl .is_diff_2
    sub r14, 0x2
 .is_diff_2:
    cmp r11, 0x0
    jnl .is_diff_3
    sub r15, 0x2
.is_diff_3:
    push rax
    mov rax, r14
    mul r15
    cmp rax, 0x0
    jl  .diff_sign  
    pop rax
    jmp .same_sign
  
 .diff_sign:
    pop rax
    cmp r11, 0x0
    jl .less_than_zero
    jmp .greater_than_zero
 ; 最终结果小于零
 .less_than_zero:
    push rax
    mov al, 0xff 
    mov byte[result_sign],al
    pop rax
    inc r11
    mov r13, 0x174876E7FF 
    neg r12
    add r12, r13
    inc r12
    neg r11 ;结果符号已经储存, 所以此处取绝对值
 ; 最终结果大于零
    jmp .end_add
 .greater_than_zero:

    push rax
    mov al, 0x1
    mov byte[result_sign], al
    pop rax
    
    dec r11
    mov r13, 0x174876E7FF 
    add r12, r13 ; 此时两个寄存器的值均不为负
    inc r12
    jmp .end_add
.same_sign:
    cmp r11, 0x0
    jl .same_sign_neg
    jmp .end_add
.same_sign_neg:
    neg r11
    neg r12
    mov byte[result_sign],0xffffffffffffffff
    jmp .end_add
 
.high_is_zero:
    cmp r12, 0x0
    jl .high_is_zero_l0
    push rax
    mov al, 0x1
    mov byte[result_sign], al
    pop rax
    jmp .end_add
.high_is_zero_l0:
    push rax
    mov al, 0xff
    mov byte[result_sign], al
    pop rax
    neg r12 ;符号由全局变量保存, 寄存器取绝对值
    
   
.end_add:
    mov r13, 0x174876E800
    cmp r12, r13
    jl .true_end_add
    sub r12, r13
    add r11, 0x1

.true_end_add:
    mov rsi, r11
    mov rdi, add_result_part_one
    call    int_to_str
    mov rsi, r12
    mov rdi, add_result_part_two
    call    int_to_str
    ret
       
    

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
      
     cmp byte[rsi+rax],0xA
    jne .second_loop    
    mov rdi, rax
    mov rdx, bits_of_b

        call strcpy
    
    ;十进制数字低十一位放入寄存器， a:RDI, b:RBX, r8 此处计数
    xor r8, r8
    xor rdi, rdi 
    xor rbx, rbx 
    xor rsi, rsi
    xor rcx, rcx
    xor rdx, rdx
 .low_0xB_bits:
    xor rax, rax
    cmp r8, 0xB
    je .high_bits
    mov dl, byte[bits_of_a+r8]
    mov al, byte[bits_of_b+r8]
    
    cmp dl, 0x2d
    je .a_is_neg
 .another_examine:
    cmp al , 0x2d
    je .b_is_neg
    jmp .end_neg
.a_is_neg:
 
    mov dl, 0x1 
    mov byte[neg_a], dl ;a 是负数
 
    xor dl, dl
    jmp .another_examine
.b_is_neg:
 
    mov al, 0x1 
    mov byte[neg_b], al ;a 是负数
 
    xor al, al
    jmp .end_neg
    
.end_neg:
    cmp dl, 0
    je .b_low_sub
    sub dl, 0x30
    
.b_low_sub:
    cmp al, 0
    je .end_sub_48      
    sub al, 0x30
    
.end_sub_48:
    mov  r9,rax ; 因为 rax 之后会用到，传给 r9 保存
    push rdi
    push rsi
    push rdx
    
    mov rdi, 0xA
    mov rsi, r8
    call pow
    
    pop rdx
    pop rsi
    pop rdi
 
    push rax
    mul rdx
    add rdi, rax
    
    pop rax
    mul r9   
    add rbx, rax
    inc r8
    jmp .low_0xB_bits
; 十进制数字高十一位放在 a: rsi, b: rcx


.high_bits:
    xor r8, r8
.high_bits_loop:
    xor rax, rax
    cmp r8, 0xB
    je .end_high_bits
    mov dl, byte[bits_of_a+r8+11]
    mov al, byte[bits_of_b+r8+11]
    cmp dl, 0x2d
    je .a_h_is_neg
.examine_b:
    cmp al , 0x2d
    je .b_h_is_neg
    jmp .end_h_neg
.a_h_is_neg:
 
    mov dl, 0x1 
    mov byte[neg_a], dl ;a 是负数
 
    xor dl, dl
    jmp .examine_b
.b_h_is_neg:
 
    mov al, 0x1 
    mov byte[neg_b], al ;a 是负数
 
    xor al, al
    jmp .end_h_neg
    
.end_h_neg:
    cmp dl, 0
    je .b_high_sub
    sub dl, 0x30
    
.b_high_sub:
    cmp al, 0
    je .end_sub_high      
    sub al, 0x30
    
.end_sub_high:
    mov  r9,rax ; 因为 rax 之后会用到，传给 r9 保存
    push rdi
    push rsi
    push rdx
    
    mov rdi, 0xA
    mov rsi, r8
    call pow
    
    pop rdx
    pop rsi
    pop rdi
 
     push rax
    mul rdx
    add rsi, rax
    
    pop rax
    mul r9   
    add rcx, rax
    inc r8
    jmp .high_bits_loop
    
; a rsi:rdi
; b rcx:rbx 
.end_high_bits:
    push rax
    mov al, 0x1
    cmp byte[neg_a], al  ; a<0?
    pop rax
    jne .adjust_b
    neg rsi
    neg rdi
.adjust_b:
    push rax
    mov al, 0x1
    cmp byte[neg_b], al ; b<0?
    pop rax
    jne .end_adjust
    neg rcx
    neg rbx
.end_adjust:
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
    cmp byte[rdi+rax] , 0x0
    je .strlen_end
    inc rax
    jmp .strlen_loop

    .strlen_end:
    inc rax ; 再加上一个换行
    ret 
    
; rdi 的 rsi 次方, rax 返回
pow:
    mov rax,1
    
.pow_loop:
    cmp rsi, 0x0
    je .pow_end
    mul rdi
    dec rsi
    jmp .pow_loop
.pow_end:
    ret

; rdi 目的地址, rsi 要存的寄存器
int_to_str:
 mov rcx, 0x0
 mov rax, rsi
 mov r8, 0xa
.i2s_loop:
xor rdx,rdx
 div r8
 mov byte[rdi+rcx], dl
 inc rcx
 cmp rax, 0x0
 jne .i2s_loop
 ret
 
 
    

;进制转换,倒序存储在全局变量里,r13 存来源, rdx 余数, rax 商 数字参数放在 rsi:rdi
bin_to_dec:
    mov r8, 0xA
    mov r9, 0x0
    xor rdx, rdx
.base_conversion_loop:
    xor rdx, rdx
    mov rax, rsi
    div r8
    mov rsi, rax
    
    mov rax, rdi
    div r8
    mov rdi, rax

    mov byte[r13+r9],dl    ; 每轮循环完毕的余数(rdx)就是一个十进制位    
    inc r9
    cmp rsi, 0x0
    jne .base_conversion_loop
    cmp rdi, 0x0
    jne .base_conversion_loop
    
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

