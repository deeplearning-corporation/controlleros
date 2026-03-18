; hal.asm - 有实际代码
.386
.model flat, stdcall
option casemap:none

.code

; DLL入口点
DllMain proc hinstDLL:DWORD, fdwReason:DWORD, lpvReserved:DWORD
    mov eax, 1      ; 返回TRUE
    ret 12
DllMain endp

; HAL函数
HalGetProcessorCount proc
    mov eax, 1      ; 返回1个处理器
    ret
HalGetProcessorCount endp

HalInitialize proc
    mov eax, 1      ; 返回TRUE
    ret
HalInitialize endp

end DllMain