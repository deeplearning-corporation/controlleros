// winload.c - ControllerOS UEFI Boot Loader (直接启动ntoskrnl)
// 编译: cl /nologo /c /GS- /O2 winload.c && link /nologo /subsystem:native /entry:_start /out:winload.efi winload.obj

// ========== ControllerOS版本 ==========
#define CONTROLLEROS_VERSION_MAJOR 1
#define CONTROLLEROS_VERSION_MINOR 0
#define CONTROLLEROS_VERSION_BUILD 2024

// ========== 内核路径 ==========
#define KERNEL_PATH "C:\\Windows\\System32\\ntoskrnl.exe"
#define KERNEL_BASE_ADDRESS 0x100000

// ========== VGA文本模式 ==========
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define COLOR_WHITE 0x0F
#define COLOR_GREEN 0x0A
#define COLOR_RED   0x0C
#define COLOR_YELLOW 0x0E
#define COLOR_CYAN   0x0B

static unsigned short* g_VideoMemory = (unsigned short*)VGA_ADDRESS;

// ========== 屏幕输出 ==========
void PrintAt(int row, int col, const char* str) {
    int pos = row * VGA_WIDTH + col;
    int i = 0;
    while (str[i] && pos < VGA_WIDTH * VGA_HEIGHT) {
        g_VideoMemory[pos] = (COLOR_WHITE << 8) | str[i];
        i++;
        pos++;
    }
}

void PrintColor(int row, int col, const char* str, int color) {
    int pos = row * VGA_WIDTH + col;
    int i = 0;
    while (str[i] && pos < VGA_WIDTH * VGA_HEIGHT) {
        g_VideoMemory[pos] = (color << 8) | str[i];
        i++;
        pos++;
    }
}

void PrintHex(int row, int col, unsigned long value) {
    char buffer[20];
    char hex[] = "0123456789ABCDEF";
    int i;
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (i = 0; i < 8; i++) {
        buffer[9 - i] = hex[value & 0xF];
        value >>= 4;
    }
    buffer[10] = 0;
    
    PrintAt(row, col, buffer);
}

void ClearScreen() {
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        g_VideoMemory[i] = (COLOR_WHITE << 8) | ' ';
    }
}

// ========== 字符串函数 ==========
int mystrlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void mystrcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = 0;
}

void mymemset(void* dest, int val, int len) {
    unsigned char* p = (unsigned char*)dest;
    while (len--) {
        *p++ = (unsigned char)val;
    }
}

// ========== 加载器参数 ==========
typedef struct _LOADER_PARAMETERS {
    char BootDevice[260];
    char SystemRoot[260];
    char KernelPath[260];
    char HalPath[260];
    unsigned long KernelBase;
    unsigned long KernelSize;
    int BootFlags;
    int BootEntry;
    void* KernelEntryPoint;
} LOADER_PARAMETERS;

static LOADER_PARAMETERS g_LoaderParams = {0};

// ========== 延迟 ==========
void Delay(int ms) {
    int i, j;
    for (i = 0; i < ms * 100; i++) {
        for (j = 0; j < 10000; j++);
    }
}

// ========== 蜂鸣 ==========
void Beep() {
    __asm {
        mov al, 0xB6
        out 0x43, al
        mov ax, 1193
        out 0x42, al
        mov al, ah
        out 0x42, al
        in al, 0x61
        or al, 3
        out 0x61, al
        mov ecx, 5000000
    delay:
        loop delay
        in al, 0x61
        and al, 0xFC
        out 0x61, al
    }
}

// ========== 等待按键 ==========
void WaitForKey() {
    __asm {
        xor ax, ax
        int 0x16
    }
}

// ========== 加载内核到内存 ==========
void* LoadKernelToMemory(const char* kernelPath, unsigned long* kernelSize) {
    PrintAt(23, 5, "Loading kernel: ");
    PrintAt(23, 19, kernelPath);
    
    // 模拟加载内核到内存
    *kernelSize = 0x200000;
    
    PrintAt(24, 5, "Kernel loaded at: ");
    PrintHex(24, 23, KERNEL_BASE_ADDRESS);
    PrintAt(24, 33, " Size: ");
    PrintHex(24, 40, *kernelSize);
    
    return (void*)KERNEL_BASE_ADDRESS;
}

// ========== 查找内核入口点 ==========
void* FindKernelEntryPoint(void* kernelBase) {
    // 内核入口点 KiSystemStartup
    unsigned long entryPoint = (unsigned long)kernelBase + 0x1000;
    
    PrintAt(25, 5, "Entry point: ");
    PrintHex(25, 18, entryPoint);
    
    return (void*)entryPoint;
}

// ========== 构建参数块 ==========
void BuildLoaderParams() {
    mymemset(&g_LoaderParams, 0, sizeof(LOADER_PARAMETERS));
    
    mystrcpy(g_LoaderParams.BootDevice, "multi(0)disk(0)rdisk(0)partition(1)");
    mystrcpy(g_LoaderParams.SystemRoot, "C:\\Windows");
    mystrcpy(g_LoaderParams.KernelPath, KERNEL_PATH);
    mystrcpy(g_LoaderParams.HalPath, "C:\\Windows\\System32\\hal.dll");
    g_LoaderParams.BootFlags = 0;
    g_LoaderParams.BootEntry = 0;
    
    // 加载内核
    void* kernelBase = LoadKernelToMemory(g_LoaderParams.KernelPath, &g_LoaderParams.KernelSize);
    g_LoaderParams.KernelBase = (unsigned long)kernelBase;
    
    // 查找入口点
    g_LoaderParams.KernelEntryPoint = FindKernelEntryPoint(kernelBase);
}

// ========== 启动内核 ==========
void StartKernel(LOADER_PARAMETERS* params) {
    typedef void (*KERNEL_ENTRY)(LOADER_PARAMETERS*);
    
    ClearScreen();
    PrintColor(8, 25, "========================================", COLOR_GREEN);
    PrintColor(9, 30, "Starting ControllerOS...", COLOR_GREEN);
    PrintColor(10, 25, "========================================", COLOR_GREEN);
    
    PrintAt(12, 5, "Kernel: ");
    PrintAt(12, 13, params->KernelPath);
    
    PrintAt(13, 5, "Base:   ");
    PrintHex(13, 13, params->KernelBase);
    
    PrintAt(14, 5, "Entry:  ");
    PrintHex(14, 13, (unsigned long)params->KernelEntryPoint);
    
    PrintAt(15, 5, "System: ");
    PrintAt(15, 13, params->SystemRoot);
    
    PrintColor(17, 25, "Transferring control to kernel...", COLOR_YELLOW);
    
    Beep();
    Delay(1000);
    
    // 跳转到内核
    KERNEL_ENTRY entry = (KERNEL_ENTRY)params->KernelEntryPoint;
    entry(params);
    
    // 如果返回，出错
    PrintColor(20, 20, "ERROR: Kernel returned!", COLOR_RED);
    while(1);
}

// ========== 显示启动信息 ==========
void ShowBootInfo() {
    ClearScreen();
    
    PrintColor(0, 25, "========================================", COLOR_CYAN);
    PrintColor(1, 25, "    ControllerOS UEFI Boot Loader", COLOR_GREEN);
    PrintColor(2, 25, "========================================", COLOR_CYAN);
    
    PrintAt(4, 5, "Kernel: ");
    PrintAt(4, 13, KERNEL_PATH);
    
    PrintAt(5, 5, "Base:   ");
    PrintHex(5, 13, KERNEL_BASE_ADDRESS);
    
    PrintAt(7, 5, "Press any key to start ControllerOS...");
    
    WaitForKey();
}

// ========== 错误处理 ==========
void BootError(const char* message) {
    ClearScreen();
    PrintColor(10, 20, "********************************", COLOR_RED);
    PrintColor(11, 20, "*  CONTROLLEROS BOOT ERROR   *", COLOR_RED);
    PrintColor(12, 20, "********************************", COLOR_RED);
    PrintAt(14, 20, message);
    PrintAt(16, 20, "Press any key to restart...");
    WaitForKey();
    
    // 重启
    __asm {
        mov al, 0xFE
        out 0x64, al
    }
    while(1);
}

// ========== 主函数 ==========
void BootMain() {
    // 显示启动信息
    ShowBootInfo();
    
    // 构建参数
    BuildLoaderParams();
    
    // 启动内核
    StartKernel(&g_LoaderParams);
    
    BootError("Kernel returned control!");
}

// ========== 入口点 ==========
void _start() {
    BootMain();
    while(1);
}
