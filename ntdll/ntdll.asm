; ntdll.asm - 有实际代码
.386
.model flat, stdcall
option casemap:none

.code

; DLL入口点
DllMain proc hinstDLL:DWORD, fdwReason:DWORD, lpvReserved:DWORD
    mov eax, 1      ; 返回TRUE
    ret 12
DllMain endp

; 导出函数
NtCreateFile proc
    mov eax, 0      ; STATUS_SUCCESS
    ret
NtCreateFile endp

NtClose proc
    mov eax, 0
    ret 4
NtClose endp

end DllMain