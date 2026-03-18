// ============================================
// ControllerOS NTOSKRNL - 带BSOD功能
// 编译: cl /nologo /c /GS- /O2 ntoskrnl.c && link /nologo /subsystem:native /entry:KiSystemStartup ntoskrnl.obj /out:ntoskrnl.exe
// ============================================

// ========== 基础类型定义 ==========
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef void VOID;
typedef void* PVOID;
typedef long NTSTATUS;
typedef int BOOLEAN;
#define FALSE 0
#define TRUE 1
#define STATUS_SUCCESS ((NTSTATUS)0)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001)

// ========== VGA文本模式 ==========
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// ========== BSOD颜色 ==========
#define COLOR_BLUE 0x1F
#define COLOR_WHITE 0x0F

// ========== 内核数据结构 ==========
typedef struct _KERNEL_DATA {
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG BuildNumber;
    ULONG ProcessorCount;
    ULONG TickCount;
    ULONG BugCheckCode;
} KERNEL_DATA, *PKERNEL_DATA;

// ========== 全局变量 ==========
static KERNEL_DATA g_KeData = {0};
static USHORT* g_VideoMemory = (USHORT*)VGA_ADDRESS;

// ========== BSOD错误代码 ==========
#define BUGCODE_UNKNOWN             0x00000000
#define BUGCODE_HAL_FAILURE          0x00000001
#define BUGCODE_MM_FAILURE           0x00000002
#define BUGCODE_OB_FAILURE           0x00000003
#define BUGCODE_PS_FAILURE           0x00000004
#define BUGCODE_SE_FAILURE           0x00000005
#define BUGCODE_UNEXPECTED           0x0000000E
#define BUGCODE_CONTROLLEROS_PANIC   0xDEADBEEF

// ========== 字符串常量 ==========
static const char* g_BugMessages[] = {
    "UNKNOWN_ERROR",
    "HAL_INITIALIZATION_FAILED",
    "MEMORY_MANAGER_FAILED",
    "OBJECT_MANAGER_FAILED",
    "PROCESS_MANAGER_FAILED",
    "SECURITY_MANAGER_FAILED",
    "UNEXPECTED_KERNEL_MODE_TRAP",
    "CONTROLLEROS_SYSTEM_PANIC"
};

// ========== 屏幕操作函数 ==========
VOID KeClearScreen(VOID) {
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        g_VideoMemory[i] = (COLOR_BLUE << 8) | ' ';
    }
}

VOID KePrintString(int row, int col, const char* str) {
    int i = 0;
    int pos = row * VGA_WIDTH + col;
    
    while (str[i] != 0 && pos < VGA_WIDTH * VGA_HEIGHT) {
        g_VideoMemory[pos] = (COLOR_WHITE << 8) | str[i];
        i++;
        pos++;
    }
}

VOID KePrintHex(int row, int col, ULONG value) {
    char buffer[11];
    char hex[] = "0123456789ABCDEF";
    int i;
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (i = 0; i < 8; i++) {
        buffer[9 - i] = hex[value & 0xF];
        value >>= 4;
    }
    buffer[10] = 0;
    
    KePrintString(row, col, buffer);
}

// ========== 蓝屏死机 ==========
VOID KeBugCheck(ULONG BugCheckCode) {
    const char* msg;
    int index = 0;
    
    // 保存错误代码
    g_KeData.BugCheckCode = BugCheckCode;
    
    // 清屏为蓝色
    KeClearScreen();
    
    // 根据错误代码获取消息
    switch (BugCheckCode) {
        case BUGCODE_HAL_FAILURE:
            index = 1;
            break;
        case BUGCODE_MM_FAILURE:
            index = 2;
            break;
        case BUGCODE_OB_FAILURE:
            index = 3;
            break;
        case BUGCODE_PS_FAILURE:
            index = 4;
            break;
        case BUGCODE_SE_FAILURE:
            index = 5;
            break;
        case BUGCODE_UNEXPECTED:
            index = 6;
            break;
        case BUGCODE_CONTROLLEROS_PANIC:
            index = 7;
            break;
        default:
            index = 0;
            break;
    }
    msg = g_BugMessages[index];
    
    // 显示BSOD信息
    KePrintString(2, 5, "*** CONTROLLEROS SYSTEM SERVICE EXCEPTION ***");
    KePrintString(4, 5, "A problem has been detected and system has been halted.");
    KePrintString(5, 5, "to prevent damage to your computer.");
    
    KePrintString(7, 5, "Technical information:");
    KePrintString(8, 5, "*** BUGCODE: ");
    KePrintHex(8, 19, BugCheckCode);
    
    KePrintString(10, 5, "*** MESSAGE: ");
    KePrintString(10, 18, msg);
    
    KePrintString(12, 5, "*** CONTROLLEROS VERSION: ");
    KePrintHex(12, 30, g_KeData.MajorVersion);
    KePrintString(12, 34, ".");
    KePrintHex(12, 35, g_KeData.MinorVersion);
    KePrintString(12, 39, ".");
    KePrintHex(12, 40, g_KeData.BuildNumber);
    
    KePrintString(14, 5, "*** PROCESSOR COUNT: ");
    KePrintHex(14, 26, g_KeData.ProcessorCount);
    
    KePrintString(16, 5, "*** TICK COUNT: ");
    KePrintHex(16, 21, g_KeData.TickCount);
    
    KePrintString(18, 5, "*** SYSTEM HALTED - RESTART YOUR COMPUTER ***");
    
    // 无限循环，保持BSOD显示
    while (1) {
        // 死循环，永不返回
    }
}

// ========== HAL初始化 ==========
NTSTATUS HalInitialize(VOID) {
    // 模拟HAL初始化
    g_KeData.ProcessorCount = 1;
    
    // 测试用：可以在这里触发BSOD
    // KeBugCheck(BUGCODE_HAL_FAILURE);
    
    return STATUS_SUCCESS;
}

// ========== 内存管理器初始化 ==========
NTSTATUS MmInitialize(VOID) {
    // 测试用：可以在这里触发BSOD
    // KeBugCheck(BUGCODE_MM_FAILURE);
    
    return STATUS_SUCCESS;
}

// ========== 对象管理器初始化 ==========
NTSTATUS ObInitialize(VOID) {
    // 测试用：可以在这里触发BSOD
    // KeBugCheck(BUGCODE_OB_FAILURE);
    
    return STATUS_SUCCESS;
}

// ========== 进程管理器初始化 ==========
NTSTATUS PsInitialize(VOID) {
    // 测试用：可以在这里触发BSOD
    // KeBugCheck(BUGCODE_PS_FAILURE);
    
    return STATUS_SUCCESS;
}

// ========== 安全管理器初始化 ==========
NTSTATUS SeInitialize(VOID) {
    // 测试用：可以在这里触发BSOD
    // KeBugCheck(BUGCODE_SE_FAILURE);
    
    return STATUS_SUCCESS;
}

// ========== 内核主循环 ==========
VOID KiKernelMainLoop(VOID) {
    while (1) {
        g_KeData.TickCount++;
        
        // 测试用：触发BSOD
        if (g_KeData.TickCount == 1000) {
            KeBugCheck(BUGCODE_CONTROLLEROS_PANIC);
        }
        
        // 空循环延时
        int i;
        for (i = 0; i < 1000000; i++);
    }
}

// ========== 系统启动入口 ==========
VOID KiSystemStartup(VOID) {
    // 清屏
    KeClearScreen();
    
    // 显示启动信息
    KePrintString(0, 0, "ControllerOS Starting...");
    
    // 初始化HAL
    if (HalInitialize() != STATUS_SUCCESS) {
        KeBugCheck(BUGCODE_HAL_FAILURE);
    }
    KePrintString(1, 0, "[OK] HAL Initialized");
    
    // 初始化内存管理器
    if (MmInitialize() != STATUS_SUCCESS) {
        KeBugCheck(BUGCODE_MM_FAILURE);
    }
    KePrintString(2, 0, "[OK] Memory Manager Initialized");
    
    // 初始化对象管理器
    if (ObInitialize() != STATUS_SUCCESS) {
        KeBugCheck(BUGCODE_OB_FAILURE);
    }
    KePrintString(3, 0, "[OK] Object Manager Initialized");
    
    // 初始化进程管理器
    if (PsInitialize() != STATUS_SUCCESS) {
        KeBugCheck(BUGCODE_PS_FAILURE);
    }
    KePrintString(4, 0, "[OK] Process Manager Initialized");
    
    // 初始化安全管理器
    if (SeInitialize() != STATUS_SUCCESS) {
        KeBugCheck(BUGCODE_SE_FAILURE);
    }
    KePrintString(5, 0, "[OK] Security Manager Initialized");
    
    // 设置内核版本
    g_KeData.MajorVersion = 1;
    g_KeData.MinorVersion = 0;
    g_KeData.BuildNumber = 2024;
    g_KeData.TickCount = 0;
    
    KePrintString(6, 0, "[OK] ControllerOS Version 1.0.2024");
    KePrintString(7, 0, "Press any key to continue...");
    
    // 进入内核主循环 - 最终会触发BSOD
    KiKernelMainLoop();
}
