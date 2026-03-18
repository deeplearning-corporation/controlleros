; start.asm
BITS 32

; 注意：在32位Windows下，C函数名有下划线前缀
extern _KiSystemStartup

; 导出符号 - 注意这里要用下划线
global _start

section .text

_start:
    ; 调用C函数
    call _KiSystemStartup
    
    ; 死循环
halt_loop:
    cli
    hlt
    jmp halt_loop