section .text
global main
main:
    mov ebp, esp; for correct debugging
   ;; mov rbp, rsp; for correct debugging
   mov ebp, esp; for correct debugging
   ; mov rbp, rsp; for correct debugging
    call showbanner
    mov esi, numstr1
    call readfun
    mov esi, numstr2
    call readfun
    
    ;mov esi, numstr1
    ;call show
    ;call println
    ;mov esi, numstr2
    ;call show
    ;call println
    
    mov esi, numstr1
    mov edi, numstr5
    call copynumstr
    
    mov esi, numstr1
    mov edi, numstr2
    call plus
    mov esi, numstr1
    call show
    call println
    
    mov esi, numstr5
    mov edi, numstr2
    call mulall
    mov esi, numstr4
    call show
    call println
     
    ; exit
    mov eax, 1
    mov ebx, 0
    int 80h
    
    
    
; get input number from keyboard
; until space(ascii 32 0x20) or enter(ascii 10 0x0A) is inputed
; INPUT:
; ESI = the pointer to where to restore the input number digits (MUST BE 40 Bytes).
readfun:
    pusha
    mov edi, esi
    add esi, 39 ; move to the last char
readloop:
    ; read and store one by one
    mov eax, 3
    mov ebx, 0
    mov ecx, esi
    mov edx, 1
    int 80h
    ; if there is space or \n jump out
    mov al, byte[esi]
    cmp al, 32
    je putahead
    cmp al, 10
    je putahead
    ; continue to read
    sub esi, 1
    jmp readloop
putahead:
    ; put the num to the start of numstr
    add esi, 1
    mov eax, edi
    add eax, 40 ; the end of numstr
nextByte:
    mov bl, byte[esi]
    mov byte[edi],bl
    add edi,1
    add esi,1
    cmp esi,eax
    je putzero
    jmp nextByte
putzero:
    ; put other digits zero
    mov byte[edi],48
    add edi,1
    cmp edi, eax
    je readexit
    jmp putzero
readexit:
    popa
    ret
    
    
; plus only one digit of a number
; INPUT:
; ESI = the pointer to num1's one digit
; EDI = the pointer to num2's one digit
; CL = the carry num from last plus
; OUTPUT:
; BYTE[ESI] = the result of this digit
; CL = the carry num for next plus
plusdigit:
    mov al, byte[esi]
    mov bl, byte[edi]
    add al, bl
    add al, cl
    sub al, 0x60
    cmp al, 10
    jnb carryone
    mov cl, 0
    jmp plusdigitexit
carryone:
    sub al, 10
    mov cl, 1
plusdigitexit:
    add al, 0x30
    mov byte[esi], al
    ret


; plus two 40digit number
; INPUT:
; ESI = the pointer to num1
; EDI = the pointer to num2
; OUTPUT:
; num1 will be result
plus:
    mov edx, esi
    add edx, 40
    mov cl, 0
plusnextdigit:
    call plusdigit
    add esi, 1
    add edi, 1
    cmp esi, edx
    je plusexit
    jmp plusnextdigit
plusexit:
    ret
    
    

; mul one digit and put the result to memory
; INPUT:
; ESI = the pointer to num1
; EDI = the pointer to num2
; ECX = which digit of num1 (from 0 to 19)
; EDX = which digit of num2 (from 0 to 19)
; OUTPUT:
; numstr3 = the result of just two bit
mulonedigit:
    pusha
    call clearnum3
    mov ebx, esi
    add ebx, ecx
    mov al, byte[ebx]
    mov ebx, edi
    add ebx, edx
    mov ah, byte[ebx]
    sub ah, 0x30
    sub al, 0x30
    mul ah
    mov bl, 10
    div bl
    
    mov ebx, numstr3
    add ebx, ecx
    add ebx, edx
    add ah,0x30
    add al,0x30
    mov byte[ebx], ah
    add ebx,1
    mov byte[ebx], al
    popa
    ret
    
    
    
    
; get mul of two 20 digit num
; INPUT:
; ESI = the pointer to num1
; EDI = the pointer to num2
; OUTPUT:
; numstr4 = result
mulall:
    pusha
    mov ecx, 0
num1loop:
    mov edx, 0
num2loop:
    call mulonedigit
    pusha
    mov esi, numstr4
    mov edi, numstr3
    call plus
    popa
    
    add edx, 1
    cmp edx, 20
    jne num2loop

    add ecx, 1
    cmp ecx, 20
    jne num1loop
    popa
    ret
    
    

clearnum3:
    pusha
    mov eax, numstr3
    mov ebx, numstr3
    add ebx, 40
clearonebigit:
    mov byte[eax], 0x30
    add eax,1
    cmp eax, ebx
    jne clearonebigit
    popa
    ret
    
    

; Print the memory to help debug.
; INPUT:
; ESI = The pointer to where to be printed. Print the next 40 Bytes.
print:
    pusha
    mov ecx, esi
    mov eax, 4
    mov ebx, 1
    mov edx, 40
    int 80h
    popa
    ret

; print \n (ascii 10)
println:
    pusha
    mov ecx, newline
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    popa
    ret
    
; show the result
; INPUT:
; ESI = The pointer to where to be printed. Print the num in next 40 bytes.
show:
    pusha
    mov edi, esi
    add edi, 39
findbegin:
    mov bl, byte[edi]
    cmp bl, 0x30
    jne showrest
    sub edi, 1
    cmp edi,esi
    jne findbegin
showrest:
    sub esi, 1
shownextdigit:
    mov ecx, edi
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    sub edi, 1
    cmp edi,esi
    jne shownextdigit
    popa
    ret
    
; copy a 40 bigit numstr to another place
; INPUT:
; ESI = the source num
; EDI = the destination address
copynumstr:
    pusha
    mov eax, esi
    add eax, 40
copyonedigit:
    mov bl, byte[esi]
    mov byte[edi], bl
    add esi, 1
    add edi, 1
    cmp esi, eax
    jne copyonedigit
    popa
    ret
    
showbanner:
    pusha
    mov ecx, banner
    mov eax, 4
    mov ebx, 1
    mov edx, 22
    int 80h
    popa
    ret



section .data
numstr1:
    db "0000000000000000000000000000000000000000" ; 40 x 0 . Every num use 40 bytes(chars) to restore.
numstr2:
    db "0000000000000000000000000000000000000000" ; 40 x 0 . Every num use 40 bytes(chars) to restore.
numstr3: ; USED FRO MULALL FUNCTION. WARNING: DO NOT USE THIS SPACE AT OTHER PLACE! 
    db "0000000000000000000000000000000000000000" ; 40 x 0 . Every num use 40 bytes(chars) to restore.
numstr4:
    db "0000000000000000000000000000000000000000" ; 40 x 0 . Every num use 40 bytes(chars) to restore.
numstr5:
    db "0000000000000000000000000000000000000000" ; 40 x 0 . Every num use 40 bytes(chars) to restore.
newline:
    db 10
banner:
    db "please input x and y:", 10
