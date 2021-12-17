; ----------------------------------------------------------------------------------------
; 仅使用syscall将"Hello,World"写入控制台。仅在64位Linux上运行。
; 使用 Linux 下的 1 号系统调用来输出一条信息和 60 号系统调用来退出程序。
; 编译汇编代码并运行：
;
; nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

          global    _start

          section   .text
_start:   
         ; write(1, message, 13)
          mov       rax,1                  ; 1 号系统调用是写操作
          mov       rdi,1                  ; 1 号文件系统调用是标准输出stdout
          mov       rsi,message            ; 输出字符串的地址
          mov       rdx,13                 ; 字节数（输出字符串的长度）
          syscall                           ; 调用操作系统进行写
          
          ; exit(0)
          mov       rax,60                 ; 60号系统调用是退出
          xor       rdi,rdi                ; 退出代码 0
          syscall                           ; 调用操作系统退出

          section   .data
message:  db        "Hello,World",10      ; 注意最后的换行符


