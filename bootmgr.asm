; bootmgr.asm - 简单的 EFI 引导程序
; 使用 NASM 编译: nasm -f bin bootmgr.asm -o bootmgr.efi

BITS 64

; ========== DOS Header ==========
org 0x400000  ; EFI 程序基址

DOS_HEADER:
    db 'MZ'
    dw 0x0090
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dd PE_HEADER

; ========== PE Header ==========
align 8
PE_HEADER:
    dd 'PE'                 ; PE signature
    dw 0x8664               ; Machine: x64
    dw 0x01                 ; NumberOfSections
    dd 0x00000000           ; TimeDateStamp
    dd 0x00000000           ; PointerToSymbolTable
    dd 0x00000000           ; NumberOfSymbols
    dw 0x00E0               ; SizeOfOptionalHeader
    dw 0x2002               ; Characteristics (EFI application)

; ========== Optional Header ==========
OPTIONAL_HEADER:
    dw 0x020B               ; Magic (PE32+)
    db 0x00                 ; MajorLinkerVersion
    db 0x00                 ; MinorLinkerVersion
    dd 0x00001000           ; SizeOfCode
    dd 0x00000000           ; SizeOfInitializedData
    dd 0x00000000           ; SizeOfUninitializedData
    dd 0x00001000           ; AddressOfEntryPoint
    dd 0x00001000           ; BaseOfCode
    dq 0x00000000           ; ImageBase
    dd 0x00001000           ; SectionAlignment
    dd 0x00000200           ; FileAlignment
    dw 0x0000               ; MajorOperatingSystemVersion
    dw 0x0000               ; MinorOperatingSystemVersion
    dw 0x0000               ; MajorImageVersion
    dw 0x0000               ; MinorImageVersion
    dw 0x0004               ; MajorSubsystemVersion
    dw 0x0000               ; MinorSubsystemVersion
    dd 0x00000000           ; Win32VersionValue
    dd 0x00003000           ; SizeOfImage
    dd 0x00001000           ; SizeOfHeaders
    dd 0x00000000           ; CheckSum
    dw 0x000A               ; Subsystem (EFI application)
    dw 0x0000               ; DllCharacteristics
    dq 0x00100000           ; SizeOfStackReserve
    dq 0x00001000           ; SizeOfStackCommit
    dq 0x00100000           ; SizeOfHeapReserve
    dq 0x00001000           ; SizeOfHeapCommit
    dd 0x00000000           ; LoaderFlags
    dd 0x00000010           ; NumberOfRvaAndSizes
    
    ; Data directories
    times 16 dq 0

; ========== Section Header ==========
SECTION_HEADER:
    db '.text'              ; Name
    times 8-4 db 0
    dd 0x00001000           ; VirtualSize
    dd 0x00001000           ; VirtualAddress
    dd 0x00001000           ; SizeOfRawData
    dd 0x00000400           ; PointerToRawData
    dd 0x00000000           ; PointerToRelocations
    dd 0x00000000           ; PointerToLineNumbers
    dw 0x0000               ; NumberOfRelocations
    dw 0x0000               ; NumberOfLineNumbers
    dd 0x60000020           ; Characteristics (code, execute, read)

; ========== 代码对齐 ==========
align 512
section_text_start:

; ========== EFI 入口点 ==========
; RCX = EFI_HANDLE ImageHandle
; RDX = EFI_SYSTEM_TABLE *SystemTable

global Start
Start:
    push rbp
    mov rbp, rsp
    sub rsp, 32             ; 影子空间
    
    ; 保存系统表
    mov [SystemTable], rdx
    
    ; 获取 ConOut 指针
    mov rcx, [rdx + 0x40]   ; SystemTable->ConOut
    
    ; 输出欢迎信息
    lea rdx, [welcome_msg]
    mov rax, [rcx + 0x08]   ; ConOut->OutputString
    call rax
    
    ; 输出引导信息
    mov rcx, [SystemTable]
    mov rcx, [rcx + 0x40]
    lea rdx, [boot_msg]
    mov rax, [rcx + 0x08]
    call rax
    
    ; 输出加载信息
    mov rcx, [SystemTable]
    mov rcx, [rcx + 0x40]
    lea rdx, [loading_msg]
    mov rax, [rcx + 0x08]
    call rax
    
    ; 模拟引导 Windows
    mov rcx, [SystemTable]
    mov rcx, [rcx + 0x40]
    lea rdx, [windows_msg]
    mov rax, [rcx + 0x08]
    call rax
    
    ; 等待按键
    mov rcx, [SystemTable]
    mov rcx, [rcx + 0x40]
    mov rcx, [rcx + 0x48]   ; WaitForKey
    
    ; 返回 EFI_SUCCESS
    xor rax, rax
    add rsp, 32
    pop rbp
    ret

; ========== 数据段 ==========
align 8
SystemTable dq 0

welcome_msg db 0x0D, 0x0A, '========================================', 0x0D, 0x0A
            db '     ControllerOS Boot Manager v1.0', 0x0D, 0x0A
            db '========================================', 0x0D, 0x0A, 0

boot_msg db 0x0D, 0x0A, '[INFO] Initializing boot manager...', 0x0D, 0x0A, 0

loading_msg db '[INFO] Loading ControllerOS kernel...', 0x0D, 0x0A, 0

windows_msg db '[INFO] Booting Windows from G:\ControllerOS\EFI\Microsoft\Boot\', 0x0D, 0x0A
            db '[INFO] Press any key to continue...', 0x0D, 0x0A, 0

; ========== 填充到文件大小 ==========
times 0x1000-($-section_text_start) db 0