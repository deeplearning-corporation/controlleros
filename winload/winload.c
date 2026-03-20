// winload.c - ControllerOS Boot Loader (NATIVE子系统)
// 编译: cl /nologo /c /GS- /O2 winload.c && link /nologo /subsystem:native /entry:_start /out:winload.exe winload.obj

// ========== ControllerOS版本 ==========
#define CONTROLLEROS_VERSION_MAJOR 1
#define CONTROLLEROS_VERSION_MINOR 0
#define CONTROLLEROS_VERSION_BUILD 2024

// ========== VGA文本模式 ==========
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define COLOR_WHITE 0x0F
#define COLOR_GREEN 0x0A
#define COLOR_RED   0x0C
#define COLOR_YELLOW 0x0E
#define COLOR_BLUE   0x09
#define COLOR_CYAN   0x0B

// ========== 内核路径 ==========
#define KERNEL_PATH "C:\\Windows\\System32\\ntoskrnl.exe"
#define KERNEL_BASE_ADDRESS 0x100000
#define KERNEL_MAX_SIZE 0x400000

// ========== 引导配置 ==========
#define MAX_BOOT_ENTRIES 8
#define MAX_PATH 260

typedef struct _BOOT_ENTRY {
    char Name[64];
    char Path[MAX_PATH];
    char Options[128];
    int Default;
} BOOT_ENTRY, *PBOOT_ENTRY;

typedef struct _BOOT_CONFIG {
    int Version;
    int Timeout;
    int DefaultEntry;
    int EntryCount;
    BOOT_ENTRY Entries[MAX_BOOT_ENTRIES];
} BOOT_CONFIG, *PBOOT_CONFIG;

typedef struct _LOADER_PARAMETERS {
    char BootDevice[MAX_PATH];
    char SystemRoot[MAX_PATH];
    char KernelPath[MAX_PATH];
    char HalPath[MAX_PATH];
    unsigned long KernelBase;
    unsigned long KernelSize;
    int BootFlags;
    int BootEntry;
    void* KernelEntryPoint;
} LOADER_PARAMETERS, *PLOADER_PARAMETERS;

static unsigned short* g_VideoMemory = (unsigned short*)VGA_ADDRESS;
static BOOT_CONFIG g_BootConfig = {0};
static LOADER_PARAMETERS g_LoaderParams = {0};
static int g_CurrentChoice = 0;

// ========== 自定义内存函数 ==========
void my_memset(void* dest, int val, int len) {
    unsigned char* p = (unsigned char*)dest;
    while (len--) {
        *p++ = (unsigned char)val;
    }
}

// ========== 自定义字符串函数 ==========
int my_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = 0;
}

// ========== 自定义数字转字符串 ==========
void my_itoa(int value, char* buffer, int radix) {
    char* p = buffer;
    char* p1;
    char* p2;
    unsigned int val;
    int is_negative = 0;
    
    if (radix == 10 && value < 0) {
        is_negative = 1;
        val = -value;
    } else {
        val = value;
    }
    
    do {
        int digit = val % radix;
        *p++ = (digit < 10) ? '0' + digit : 'A' + digit - 10;
        val /= radix;
    } while (val);
    
    if (is_negative) {
        *p++ = '-';
    }
    *p = 0;
    
    p1 = buffer;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
}

void my_uitoa(unsigned int value, char* buffer, int radix) {
    char* p = buffer;
    char* p1;
    char* p2;
    
    do {
        int digit = value % radix;
        *p++ = (digit < 10) ? '0' + digit : 'A' + digit - 10;
        value /= radix;
    } while (value);
    *p = 0;
    
    p1 = buffer;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
}

// ========== 简化版sprintf ==========
void my_sprintf(char* buffer, const char* format, ...) {
    char* p = buffer;
    const char* f = format;
    char* arg = (char*)&format + sizeof(format);
    
    while (*f) {
        if (*f == '%') {
            f++;
            if (*f == 'd') {
                int val = *(int*)arg;
                arg += sizeof(int);
                char num[20];
                my_itoa(val, num, 10);
                char* np = num;
                while (*np) *p++ = *np++;
            } else if (*f == 'x') {
                unsigned int val = *(unsigned int*)arg;
                arg += sizeof(unsigned int);
                char num[20];
                my_uitoa(val, num, 16);
                char* np = num;
                while (*np) *p++ = *np++;
            } else if (*f == 's') {
                const char* str = *(const char**)arg;
                arg += sizeof(const char*);
                while (*str) *p++ = *str++;
            } else {
                *p++ = '%';
                *p++ = *f;
            }
        } else {
            *p++ = *f;
        }
        f++;
    }
    *p = 0;
}

// ========== 屏幕输出 ==========
void ClearScreen() {
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        g_VideoMemory[i] = (COLOR_WHITE << 8) | ' ';
    }
}

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

// ========== 蜂鸣 ==========
void BeepSimple() {
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

// ========== 按键函数 ==========
int GetKey() {
    int key;
    __asm {
        xor ax, ax
        int 0x16
        mov key, eax
    }
    return key;
}

int CheckKey() {
    int result;
    __asm {
        mov ah, 1
        int 0x16
        jz no_key
        mov eax, 1
        mov result, eax
        jmp done
    no_key:
        mov result, 0
    done:
    }
    return result;
}

// ========== 延时 ==========
void Delay(int ms) {
    int i, j;
    for (i = 0; i < ms * 100; i++) {
        for (j = 0; j < 10000; j++);
    }
}

// ========== 初始化引导配置 ==========
void InitBootConfig() {
    int i;
    
    g_BootConfig.Version = 1;
    g_BootConfig.Timeout = 5;
    g_BootConfig.DefaultEntry = 0;
    g_BootConfig.EntryCount = 4;
    
    // 清空所有条目
    for (i = 0; i < MAX_BOOT_ENTRIES; i++) {
        my_memset(g_BootConfig.Entries[i].Name, 0, 64);
        my_memset(g_BootConfig.Entries[i].Path, 0, MAX_PATH);
        my_memset(g_BootConfig.Entries[i].Options, 0, 128);
        g_BootConfig.Entries[i].Default = 0;
    }
    
    // 启动项 0: ControllerOS (正常模式)
    my_strcpy(g_BootConfig.Entries[0].Name, "ControllerOS 1.0");
    my_strcpy(g_BootConfig.Entries[0].Path, KERNEL_PATH);
    my_strcpy(g_BootConfig.Entries[0].Options, "/kernel=ntoskrnl.exe /hal=hal.dll");
    g_BootConfig.Entries[0].Default = 1;
    
    // 启动项 1: ControllerOS (安全模式)
    my_strcpy(g_BootConfig.Entries[1].Name, "ControllerOS (Safe Mode)");
    my_strcpy(g_BootConfig.Entries[1].Path, KERNEL_PATH);
    my_strcpy(g_BootConfig.Entries[1].Options, "/safeboot:minimal /sos");
    g_BootConfig.Entries[1].Default = 0;
    
    // 启动项 2: ControllerOS (调试模式)
    my_strcpy(g_BootConfig.Entries[2].Name, "ControllerOS (Debug Mode)");
    my_strcpy(g_BootConfig.Entries[2].Path, KERNEL_PATH);
    my_strcpy(g_BootConfig.Entries[2].Options, "/debug /debugport=com1 /baudrate=115200");
    g_BootConfig.Entries[2].Default = 0;
    
    // 启动项 3: 启动另一个系统
    my_strcpy(g_BootConfig.Entries[3].Name, "Windows (Alternative)");
    my_strcpy(g_BootConfig.Entries[3].Path, "C:\\Windows\\System32\\ntoskrnl.exe");
    my_strcpy(g_BootConfig.Entries[3].Options, "");
    g_BootConfig.Entries[3].Default = 0;
}

// ========== 显示启动菜单 ==========
void DisplayBootMenu() {
    int i;
    char buffer[128];
    
    ClearScreen();
    
    // 标题
    PrintColor(0, 25, "========================================", COLOR_CYAN);
    PrintColor(1, 25, "    ControllerOS Boot Manager", COLOR_GREEN);
    PrintColor(2, 25, "========================================", COLOR_CYAN);
    
    // 版本
    my_sprintf(buffer, "Version %d.%d.%d", 
               CONTROLLEROS_VERSION_MAJOR,
               CONTROLLEROS_VERSION_MINOR,
               CONTROLLEROS_VERSION_BUILD);
    PrintColor(3, 30, buffer, COLOR_YELLOW);
    
    PrintAt(5, 5, "Choose an operating system to start:");
    
    // 列出启动项
    for (i = 0; i < g_BootConfig.EntryCount; i++) {
        char prefix[4];
        if (i == g_CurrentChoice) {
            my_strcpy(prefix, "-> ");
            PrintColor(7 + i, 5, prefix, COLOR_GREEN);
        } else {
            my_strcpy(prefix, "   ");
            PrintAt(7 + i, 5, prefix);
        }
        
        my_sprintf(buffer, "%d. %s", i + 1, g_BootConfig.Entries[i].Name);
        
        if (i == g_BootConfig.DefaultEntry) {
            PrintColor(7 + i, 8, buffer, COLOR_YELLOW);
        } else {
            PrintAt(7 + i, 8, buffer);
        }
    }
    
    // 显示内核路径
    PrintColor(7 + g_BootConfig.EntryCount + 2, 5, "Kernel:", COLOR_CYAN);
    PrintAt(7 + g_BootConfig.EntryCount + 2, 12, KERNEL_PATH);
    
    // 倒计时
    my_sprintf(buffer, "Seconds until default: %d", g_BootConfig.Timeout);
    PrintAt(7 + g_BootConfig.EntryCount + 4, 5, buffer);
    
    PrintAt(7 + g_BootConfig.EntryCount + 6, 5, 
            "Use arrow keys to select, Enter to confirm.");
}

// ========== 等待用户选择 ==========
int WaitForChoice() {
    int timeout = g_BootConfig.Timeout;
    int choice = g_BootConfig.DefaultEntry;
    char buffer[128];
    
    while (timeout > 0) {
        // 更新倒计时
        my_sprintf(buffer, "Seconds until default: %d  ", timeout);
        PrintAt(7 + g_BootConfig.EntryCount + 4, 5, buffer);
        
        // 检查按键
        if (CheckKey()) {
            int key = GetKey();
            
            // 上箭头 (0x4800)
            if (key == 0x4800 && choice > 0) {
                choice--;
                g_CurrentChoice = choice;
                DisplayBootMenu();
            }
            // 下箭头 (0x5000)
            else if (key == 0x5000 && choice < g_BootConfig.EntryCount - 1) {
                choice++;
                g_CurrentChoice = choice;
                DisplayBootMenu();
            }
            // 回车 (0x1C0D)
            else if (key == 0x1C0D) {
                return choice;
            }
            // 数字键 1-8
            else if ((key & 0xFF) >= '1' && (key & 0xFF) <= '8') {
                int num = (key & 0xFF) - '1';
                if (num < g_BootConfig.EntryCount) {
                    return num;
                }
            }
        }
        
        // 延时1秒
        Delay(1000);
        timeout--;
    }
    
    return g_BootConfig.DefaultEntry;
}

// ========== 加载内核到内存 ==========
void* LoadKernelToMemory(const char* kernelPath, unsigned long* kernelSize) {
    PrintAt(23, 5, "Loading kernel: ");
    PrintAt(23, 19, kernelPath);
    
    // 模拟加载内核到内存
    // 实际应该读取文件并加载到 KERNEL_BASE_ADDRESS
    
    *kernelSize = 0x200000;  // 2MB
    
    PrintAt(24, 5, "Kernel loaded at: ");
    PrintHex(24, 23, KERNEL_BASE_ADDRESS);
    PrintAt(24, 33, " Size: ");
    PrintHex(24, 40, *kernelSize);
    
    return (void*)KERNEL_BASE_ADDRESS;
}

// ========== 查找内核入口点 ==========
void* FindKernelEntryPoint(void* kernelBase) {
    // 内核入口点通常是 KiSystemStartup
    // 在PE文件中是入口点，这里简化为基址+偏移
    unsigned long entryPoint = (unsigned long)kernelBase + 0x1000;
    
    PrintAt(25, 5, "Entry point: ");
    PrintHex(25, 18, entryPoint);
    
    return (void*)entryPoint;
}

// ========== 构建参数块 ==========
void BuildLoaderParams(PLOADER_PARAMETERS params, int choice) {
    my_memset(params, 0, sizeof(LOADER_PARAMETERS));
    
    my_strcpy(params->BootDevice, "multi(0)disk(0)rdisk(0)partition(1)");
    my_strcpy(params->SystemRoot, "C:\\Windows");
    my_strcpy(params->KernelPath, g_BootConfig.Entries[choice].Path);
    my_strcpy(params->HalPath, "C:\\Windows\\System32\\hal.dll");
    params->BootFlags = 0;
    params->BootEntry = choice;
    
    // 加载内核到内存
    void* kernelBase = LoadKernelToMemory(params->KernelPath, &params->KernelSize);
    params->KernelBase = (unsigned long)kernelBase;
    
    // 查找内核入口点
    params->KernelEntryPoint = FindKernelEntryPoint(kernelBase);
}

// ========== 转移到内核 ==========
void TransferToKernel(PLOADER_PARAMETERS params) {
    typedef void (*KERNEL_ENTRY)(PLOADER_PARAMETERS);
    
    ClearScreen();
    PrintColor(8, 25, "========================================", COLOR_GREEN);
    PrintColor(9, 28, "Starting ControllerOS...", COLOR_GREEN);
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
    
    BeepSimple();
    Delay(1000);
    
    // 真正跳转到内核
    KERNEL_ENTRY entry = (KERNEL_ENTRY)params->KernelEntryPoint;
    entry(params);
    
    // 如果内核返回，说明出错
    PrintColor(20, 20, "ERROR: Kernel returned control!", COLOR_RED);
    while(1);
}

// ========== 引导错误 ==========
void BootError(const char* message, int code) {
    char buffer[256];
    
    ClearScreen();
    PrintColor(10, 20, "********************************", COLOR_RED);
    PrintColor(11, 20, "*  CONTROLLEROS BOOT ERROR   *", COLOR_RED);
    PrintColor(12, 20, "********************************", COLOR_RED);
    
    PrintAt(14, 20, message);
    
    my_sprintf(buffer, "Error Code: 0x%08X", code);
    PrintAt(16, 20, buffer);
    
    PrintAt(18, 20, "Press any key to restart...");
    GetKey();
    
    // 重启
    __asm {
        mov al, 0xFE
        out 0x64, al
    }
    
    while(1);
}

// ========== 主引导函数 ==========
void BootMain() {
    int choice;
    
    // 初始化
    InitBootConfig();
    
    // 显示启动菜单
    DisplayBootMenu();
    
    // 等待用户选择
    choice = WaitForChoice();
    
    if (choice < 0 || choice >= g_BootConfig.EntryCount) {
        BootError("Invalid boot selection", 0xC0000001);
        return;
    }
    
    g_CurrentChoice = choice;
    
    // 构建参数
    BuildLoaderParams(&g_LoaderParams, choice);
    
    // 转移到内核
    TransferToKernel(&g_LoaderParams);
    
    // 如果返回，出错
    BootError("Kernel returned control to boot loader", 0xC0000002);
}

// ========== NATIVE入口点 ==========
void _start() {
    BootMain();
    while(1);
}
