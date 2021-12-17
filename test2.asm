 
SYS_EXIT  equ 1
SYS_WRITE equ 4
STDIN     equ 0
STDOUT    equ 1
; Hello World Program - asmtutor.com ; Compile with: nasm -f elf helloworld.asm ; Link with (64 bit systems require elf_i386 option): ld -m elf_i386 helloworld.o -o helloworld ; Run with: ./helloworld 
SECTION .data 
    msg db 'Hello World!', 0Ah ; assign msg variable with your message string 
SECTION .text 
    global _start 


_start: mov edx, 13 ; number of bytes to write - one for each letter plus 0Ah (line feed character)
   mov eax, SYS_WRITE        
   mov ebx, STDOUT        
   mov ecx, msg       
   mov edx, len
   int 0x80   

   mov eax, SYS_WRITE        
   mov ebx, STDOUT        
   mov ecx, msg       
   mov edx, len
   int 0x80   
