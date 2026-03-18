// ntdll.c - ¼ò»¯°æ
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}

NTSTATUS WINAPI NtCreateFile(
    PHANDLE FileHandle,
    DWORD DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    DWORD FileAttributes,
    DWORD ShareAccess,
    DWORD CreateDisposition,
    DWORD CreateOptions,
    PVOID EaBuffer,
    DWORD EaLength
) {
    return STATUS_SUCCESS;
}
