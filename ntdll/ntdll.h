// ntdll.h - ntdll 内部头文件
#ifndef _NTDLL_H_
#define _NTDLL_H_

#include <windef.h>
#include <winnt.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== 系统服务调用 ==========
NTSTATUS NTAPI NtCreateFile(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

NTSTATUS NTAPI NtClose(
    HANDLE Handle
);

NTSTATUS NTAPI NtReadFile(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    PULONG Key
);

NTSTATUS NTAPI NtWriteFile(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    PULONG Key
);

// ========== 内存管理 ==========
NTSTATUS NTAPI NtAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);

NTSTATUS NTAPI NtFreeVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    PSIZE_T RegionSize,
    ULONG FreeType
);

// ========== 进程/线程 ==========
NTSTATUS NTAPI NtCreateProcess(
    PHANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ParentProcess,
    BOOLEAN InheritObjectTable,
    HANDLE SectionHandle,
    HANDLE DebugPort,
    HANDLE ExceptionPort
);

NTSTATUS NTAPI NtCreateThread(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PCLIENT_ID ClientId,
    PCONTEXT ThreadContext,
    PINITIAL_TEB InitialTeb,
    BOOLEAN CreateSuspended
);

// ========== 同步对象 ==========
NTSTATUS NTAPI NtCreateEvent(
    PHANDLE EventHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    EVENT_TYPE EventType,
    BOOLEAN InitialState
);

NTSTATUS NTAPI NtWaitForSingleObject(
    HANDLE Handle,
    BOOLEAN Alertable,
    PLARGE_INTEGER Timeout
);

// ========== 运行时库 ==========
PVOID NTAPI RtlAllocateHeap(
    PVOID HeapHandle,
    ULONG Flags,
    SIZE_T Size
);

BOOLEAN NTAPI RtlFreeHeap(
    PVOID HeapHandle,
    ULONG Flags,
    PVOID Memory
);

VOID NTAPI RtlZeroMemory(
    PVOID Destination,
    SIZE_T Length
);

VOID NTAPI RtlCopyMemory(
    PVOID Destination,
    const VOID *Source,
    SIZE_T Length
);

VOID NTAPI RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
);

// ========== 调试 ==========
ULONG NTAPI DbgPrint(
    PCH Format,
    ...
);

VOID NTAPI DbgBreakPoint(
    VOID
);

#ifdef __cplusplus
}
#endif

#endif // _NTDLL_H_
