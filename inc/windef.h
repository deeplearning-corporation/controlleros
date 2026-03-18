// windef.h - 基础类型定义
#ifndef _WINDEF_H_
#define _WINDEF_H_

// 基本整数类型
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned long long  QWORD;
typedef long                LONG;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;

// 指针类型
typedef void*               PVOID;
typedef char*               PCHAR;
typedef short*              PSHORT;
typedef long*               PLONG;
typedef unsigned char*      PBYTE;
typedef unsigned short*     PWORD;
typedef unsigned long*      PDWORD;

// 字符串类型
typedef char                CHAR;
typedef short               SHORT;
typedef wchar_t             WCHAR;
typedef WCHAR*              PWSTR;
typedef const WCHAR*        PCWSTR;
typedef CHAR*               PSTR;
typedef const CHAR*         PCSTR;

// 布尔类型
typedef int                 BOOL;
typedef unsigned char       BOOLEAN;
#define FALSE               0
#define TRUE                1

// 返回状态
typedef LONG                NTSTATUS;
#define STATUS_SUCCESS      ((NTSTATUS)0)

// 调用约定
#define WINAPI              __stdcall
#define NTAPI               __stdcall
#define CALLBACK            __stdcall

// 基本结构
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#endif
