// winnt.h - Windows类型定义
#ifndef _WINNT_H_
#define _WINNT_H_

#include "windef.h"

// 句柄类型
typedef PVOID               HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

// 对象属性
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes(p, n, a, r, s) \
    do { \
        (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
        (p)->RootDirectory = r; \
        (p)->Attributes = a; \
        (p)->ObjectName = n; \
        (p)->SecurityDescriptor = s; \
        (p)->SecurityQualityOfService = NULL; \
    } while(0)

// IO状态块
typedef struct _IO_STATUS_BLOCK {
    NTSTATUS Status;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#endif
