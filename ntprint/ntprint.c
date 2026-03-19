// ntprint.exe - 打印机后台处理程序
// 编译: cl /nologo /GS- /O2 /D "WIN32" /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" ntprint.c /Fe:ntprint.exe

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ========== 版本信息 ==========
#define NTPRINT_VERSION_MAJOR 1
#define NTPRINT_VERSION_MINOR 0
#define NTPRINT_VERSION_BUILD 2024

// ========== 重命名我们的常量，避免与winspool.h冲突 ==========
#define MY_JOB_STATUS_QUEUED       0x0001
#define MY_JOB_STATUS_PRINTING     0x0002
#define MY_JOB_STATUS_PAUSED       0x0004
#define MY_JOB_STATUS_COMPLETED    0x0008
#define MY_JOB_STATUS_ERROR        0x0010
#define MY_JOB_STATUS_CANCELLED    0x0020

#define MY_PRINTER_STATUS_READY    0x0000
#define MY_PRINTER_STATUS_PAUSED   0x0001
#define MY_PRINTER_STATUS_ERROR    0x0002
#define MY_PRINTER_STATUS_PENDING  0x0004
#define MY_PRINTER_STATUS_OFFLINE  0x0008

// ========== 打印任务结构 ==========
typedef struct _MY_PRINT_JOB {
    DWORD JobId;
    CHAR DocumentName[256];
    CHAR PrinterName[128];
    CHAR DriverName[128];
    CHAR PortName[64];
    DWORD TotalPages;
    DWORD CurrentPage;
    DWORD Status;
    DWORD Priority;
    FILETIME SubmitTime;
    FILETIME StartTime;
    FILETIME EndTime;
    struct _MY_PRINT_JOB* Next;
} MY_PRINT_JOB, *PMY_PRINT_JOB;

// ========== 打印机结构 ==========
typedef struct _MY_PRINTER_INFO {
    CHAR Name[128];
    CHAR Port[64];
    CHAR Driver[128];
    CHAR Comment[256];
    DWORD Status;
    DWORD JobCount;
    PMY_PRINT_JOB Jobs;
} MY_PRINTER_INFO, *PMY_PRINTER_INFO;

// ========== 全局变量 ==========
static MY_PRINTER_INFO g_Printers[16];
static DWORD g_PrinterCount = 0;
static DWORD g_NextJobId = 1;
static HANDLE g_hJobMutex = NULL;
static HANDLE g_hStopEvent = NULL;
static BOOL g_bRunning = TRUE;
static PMY_PRINT_JOB g_JobQueue = NULL;

// ========== 函数声明 ==========
VOID PrintBanner(VOID);
VOID PrintHelp(VOID);
BOOL InitPrintSystem(VOID);
BOOL AddMyPrinter(LPCSTR Name, LPCSTR Port, LPCSTR Driver);
BOOL RemoveMyPrinter(LPCSTR Name);
BOOL SubmitMyPrintJob(LPCSTR PrinterName, LPCSTR DocumentName);
BOOL CancelMyPrintJob(DWORD JobId);
BOOL ProcessMyPrintQueue(VOID);
BOOL ProcessMyPrintJob(PMY_PRINT_JOB Job);
VOID ListMyPrinters(VOID);
VOID ListMyJobs(LPCSTR PrinterName);
VOID CleanupMyPrintSystem(VOID);
DWORD WINAPI MyPrintWorkerThread(LPVOID lpParam);
PMY_PRINT_JOB FindMyJob(DWORD JobId);
PMY_PRINT_JOB FindNextMyJobToPrint(VOID);
VOID UpdateMyJobStatus(PMY_PRINT_JOB Job, DWORD Status);
LPCSTR GetMyJobStatusString(DWORD Status);
LPCSTR GetMyPrinterStatusString(DWORD Status);
BOOL SendToMyPort(PMY_PRINT_JOB Job);
BOOL CreateMySpoolFile(PMY_PRINT_JOB Job);
BOOL DeleteMySpoolFile(PMY_PRINT_JOB Job);
VOID GetCurrentTimeString(CHAR* Buffer, DWORD Size);
BOOL WINAPI MyConsoleHandler(DWORD dwCtrlType);

// ========== 主函数 ==========
int main(int argc, char* argv[]) {
    // 设置控制台标题
    SetConsoleTitleA("ControllerOS Print Spooler");
    
    // 显示标题
    PrintBanner();
    
    // 设置控制台事件处理
    SetConsoleCtrlHandler(MyConsoleHandler, TRUE);
    
    // 检查命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "/help") == 0 || 
            strcmp(argv[1], "-h") == 0 || 
            strcmp(argv[1], "--help") == 0) {
            PrintHelp();
            return 0;
        }
        else if (strcmp(argv[1], "/add") == 0 && argc >= 5) {
            InitPrintSystem();
            AddMyPrinter(argv[2], argv[3], argv[4]);
            return 0;
        }
        else if (strcmp(argv[1], "/remove") == 0 && argc >= 3) {
            InitPrintSystem();
            RemoveMyPrinter(argv[2]);
            return 0;
        }
        else if (strcmp(argv[1], "/list") == 0) {
            InitPrintSystem();
            ListMyPrinters();
            return 0;
        }
        else if (strcmp(argv[1], "/jobs") == 0 && argc >= 3) {
            InitPrintSystem();
            ListMyJobs(argv[2]);
            return 0;
        }
        else if (strcmp(argv[1], "/print") == 0 && argc >= 4) {
            InitPrintSystem();
            SubmitMyPrintJob(argv[2], argv[3]);
            return 0;
        }
        else if (strcmp(argv[1], "/cancel") == 0 && argc >= 3) {
            InitPrintSystem();
            CancelMyPrintJob(atoi(argv[2]));
            return 0;
        }
        else if (strcmp(argv[1], "/install") == 0) {
            printf("Installing print service...\n");
            InitPrintSystem();
            AddMyPrinter("Default Printer", "LPT1:", "Generic / Text Only");
            AddMyPrinter("PDF Writer", "FILE:", "Microsoft Print to PDF");
            printf("Default printers installed.\n");
            return 0;
        }
    }
    
    // 启动打印服务
    printf("Starting print spooler service...\n");
    
    if (!InitPrintSystem()) {
        printf("Failed to initialize print system!\n");
        return 1;
    }
    
    // 创建默认打印机
    AddMyPrinter("Default Printer", "LPT1:", "Generic / Text Only");
    AddMyPrinter("PDF Writer", "FILE:", "Microsoft Print to PDF");
    
    // 创建互斥体
    g_hJobMutex = CreateMutex(NULL, FALSE, NULL);
    if (!g_hJobMutex) {
        printf("Failed to create mutex\n");
        CleanupMyPrintSystem();
        return 1;
    }
    
    // 创建停止事件
    g_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_hStopEvent) {
        printf("Failed to create event\n");
        CleanupMyPrintSystem();
        return 1;
    }
    
    // 创建工作线程
    HANDLE hThread = CreateThread(NULL, 0, MyPrintWorkerThread, NULL, 0, NULL);
    if (!hThread) {
        printf("Failed to create worker thread\n");
        CleanupMyPrintSystem();
        return 1;
    }
    
    printf("\nPrint spooler running. Press Ctrl+C to stop...\n");
    printf("Use 'ntprint /help' for command line options\n\n");
    
    // 等待停止事件
    WaitForSingleObject(g_hStopEvent, INFINITE);
    
    printf("\nShutting down print spooler...\n");
    g_bRunning = FALSE;
    
    // 等待线程结束
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    
    // 清理
    CleanupMyPrintSystem();
    printf("Print spooler stopped.\n");
    
    return 0;
}

// ========== 打印标题 ==========
VOID PrintBanner(VOID) {
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║     ControllerOS NTPrint v%d.%d.%d             ║\n", 
           NTPRINT_VERSION_MAJOR, NTPRINT_VERSION_MINOR, NTPRINT_VERSION_BUILD);
    printf("║     Windows Printer Spooler Service            ║\n");
    printf("║     Copyright (c) ControllerOS                 ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ========== 打印帮助 ==========
VOID PrintHelp(VOID) {
    printf("Usage: ntprint [options]\n\n");
    printf("Options:\n");
    printf("  (no options)       Run as service\n");
    printf("  /install           Install default printers\n");
    printf("  /add <name> <port> <driver>   Add printer\n");
    printf("  /remove <name>                 Remove printer\n");
    printf("  /list                          List printers\n");
    printf("  /print <printer> <file>        Print file\n");
    printf("  /jobs <printer>                 List jobs\n");
    printf("  /cancel <jobid>                 Cancel job\n");
    printf("  /help                           Show help\n\n");
    printf("Examples:\n");
    printf("  ntprint\n");
    printf("  ntprint /add \"HP LaserJet\" \"USB001\" \"HP PCL6\"\n");
    printf("  ntprint /print \"Default Printer\" \"test.txt\"\n");
    printf("  ntprint /list\n");
}

// ========== 初始化打印系统 ==========
BOOL InitPrintSystem(VOID) {
    memset(g_Printers, 0, sizeof(g_Printers));
    g_PrinterCount = 0;
    g_NextJobId = 1;
    g_JobQueue = NULL;
    
    // 创建假脱机目录
    CreateDirectoryA("C:\\Windows\\Temp\\Spool", NULL);
    CreateDirectoryA("C:\\Windows\\System32\\spool\\PRINTERS", NULL);
    
    return TRUE;
}

// ========== 添加打印机 ==========
BOOL AddMyPrinter(LPCSTR Name, LPCSTR Port, LPCSTR Driver) {
    if (g_PrinterCount >= 16) {
        printf("Error: Maximum printer limit reached\n");
        return FALSE;
    }
    
    // 检查是否已存在
    for (DWORD i = 0; i < g_PrinterCount; i++) {
        if (strcmp(g_Printers[i].Name, Name) == 0) {
            printf("Error: Printer '%s' already exists\n", Name);
            return FALSE;
        }
    }
    
    PMY_PRINTER_INFO pPrinter = &g_Printers[g_PrinterCount];
    
    strcpy_s(pPrinter->Name, sizeof(pPrinter->Name), Name);
    strcpy_s(pPrinter->Port, sizeof(pPrinter->Port), Port);
    strcpy_s(pPrinter->Driver, sizeof(pPrinter->Driver), Driver);
    sprintf_s(pPrinter->Comment, sizeof(pPrinter->Comment), 
              "ControllerOS Printer on %s", Port);
    
    pPrinter->Status = MY_PRINTER_STATUS_READY;
    pPrinter->JobCount = 0;
    pPrinter->Jobs = NULL;
    
    g_PrinterCount++;
    
    printf("Printer added: %s [%s] Driver: %s\n", Name, Port, Driver);
    return TRUE;
}

// ========== 移除打印机 ==========
BOOL RemoveMyPrinter(LPCSTR Name) {
    for (DWORD i = 0; i < g_PrinterCount; i++) {
        if (strcmp(g_Printers[i].Name, Name) == 0) {
            // 取消所有等待的作业
            if (g_hJobMutex) WaitForSingleObject(g_hJobMutex, INFINITE);
            
            PMY_PRINT_JOB pJob = g_Printers[i].Jobs;
            while (pJob) {
                PMY_PRINT_JOB pNext = pJob->Next;
                DeleteMySpoolFile(pJob);
                free(pJob);
                pJob = pNext;
            }
            
            // 移动后续打印机
            for (DWORD j = i; j < g_PrinterCount - 1; j++) {
                g_Printers[j] = g_Printers[j + 1];
            }
            g_PrinterCount--;
            
            if (g_hJobMutex) ReleaseMutex(g_hJobMutex);
            
            printf("Printer removed: %s\n", Name);
            return TRUE;
        }
    }
    
    printf("Error: Printer '%s' not found\n", Name);
    return FALSE;
}

// ========== 提交打印作业 ==========
BOOL SubmitMyPrintJob(LPCSTR PrinterName, LPCSTR DocumentName) {
    // 查找打印机
    PMY_PRINTER_INFO pPrinter = NULL;
    for (DWORD i = 0; i < g_PrinterCount; i++) {
        if (strcmp(g_Printers[i].Name, PrinterName) == 0) {
            pPrinter = &g_Printers[i];
            break;
        }
    }
    
    if (!pPrinter) {
        printf("Error: Printer '%s' not found\n", PrinterName);
        return FALSE;
    }
    
    // 创建作业
    PMY_PRINT_JOB pJob = (PMY_PRINT_JOB)malloc(sizeof(MY_PRINT_JOB));
    if (!pJob) {
        printf("Error: Out of memory\n");
        return FALSE;
    }
    
    memset(pJob, 0, sizeof(MY_PRINT_JOB));
    
    pJob->JobId = g_NextJobId++;
    strcpy_s(pJob->DocumentName, sizeof(pJob->DocumentName), DocumentName);
    strcpy_s(pJob->PrinterName, sizeof(pJob->PrinterName), PrinterName);
    strcpy_s(pJob->DriverName, sizeof(pJob->DriverName), pPrinter->Driver);
    strcpy_s(pJob->PortName, sizeof(pJob->PortName), pPrinter->Port);
    
    pJob->Status = MY_JOB_STATUS_QUEUED;
    pJob->Priority = 1;
    pJob->TotalPages = (rand() % 5) + 1;  // 随机页数
    pJob->CurrentPage = 0;
    
    GetSystemTimeAsFileTime(&pJob->SubmitTime);
    
    // 添加到队列
    if (g_hJobMutex) WaitForSingleObject(g_hJobMutex, INFINITE);
    
    pJob->Next = g_JobQueue;
    g_JobQueue = pJob;
    
    pJob->Next = pPrinter->Jobs;
    pPrinter->Jobs = pJob;
    pPrinter->JobCount++;
    
    if (g_hJobMutex) ReleaseMutex(g_hJobMutex);
    
    CHAR timeStr[64];
    GetCurrentTimeString(timeStr, sizeof(timeStr));
    
    printf("[%s] Job #%d submitted: %s to %s\n", 
           timeStr, pJob->JobId, DocumentName, PrinterName);
    
    return TRUE;
}

// ========== 取消打印作业 ==========
BOOL CancelMyPrintJob(DWORD JobId) {
    if (!g_hJobMutex) return FALSE;
    
    WaitForSingleObject(g_hJobMutex, INFINITE);
    
    PMY_PRINT_JOB pJob = FindMyJob(JobId);
    if (!pJob) {
        ReleaseMutex(g_hJobMutex);
        printf("Error: Job #%d not found\n", JobId);
        return FALSE;
    }
    
    if (pJob->Status == MY_JOB_STATUS_COMPLETED) {
        printf("Cannot cancel completed job #%d\n", JobId);
        ReleaseMutex(g_hJobMutex);
        return FALSE;
    }
    
    pJob->Status = MY_JOB_STATUS_CANCELLED;
    DeleteMySpoolFile(pJob);
    
    CHAR timeStr[64];
    GetCurrentTimeString(timeStr, sizeof(timeStr));
    
    printf("[%s] Job #%d cancelled\n", timeStr, JobId);
    
    ReleaseMutex(g_hJobMutex);
    return TRUE;
}

// ========== 查找作业 ==========
PMY_PRINT_JOB FindMyJob(DWORD JobId) {
    PMY_PRINT_JOB pJob = g_JobQueue;
    while (pJob) {
        if (pJob->JobId == JobId) {
            return pJob;
        }
        pJob = pJob->Next;
    }
    return NULL;
}

// ========== 查找下一个要打印的作业 ==========
PMY_PRINT_JOB FindNextMyJobToPrint(VOID) {
    PMY_PRINT_JOB pJob = g_JobQueue;
    PMY_PRINT_JOB pBest = NULL;
    
    while (pJob) {
        if (pJob->Status == MY_JOB_STATUS_QUEUED) {
            if (!pBest || pJob->Priority > pBest->Priority) {
                pBest = pJob;
            }
        }
        pJob = pJob->Next;
    }
    
    return pBest;
}

// ========== 处理打印队列 ==========
BOOL ProcessMyPrintQueue(VOID) {
    PMY_PRINT_JOB pJob = FindNextMyJobToPrint();
    
    if (pJob && g_hJobMutex) {
        WaitForSingleObject(g_hJobMutex, INFINITE);
        pJob->Status = MY_JOB_STATUS_PRINTING;
        GetSystemTimeAsFileTime(&pJob->StartTime);
        ReleaseMutex(g_hJobMutex);
        
        // 处理作业
        ProcessMyPrintJob(pJob);
    }
    
    return TRUE;
}

// ========== 处理打印作业 ==========
BOOL ProcessMyPrintJob(PMY_PRINT_JOB Job) {
    CHAR timeStr[64];
    GetCurrentTimeString(timeStr, sizeof(timeStr));
    
    printf("[%s] Printing job #%d: %s\n", 
           timeStr, Job->JobId, Job->DocumentName);
    
    // 创建假脱机文件
    if (!CreateMySpoolFile(Job)) {
        Job->Status = MY_JOB_STATUS_ERROR;
        return FALSE;
    }
    
    // 模拟打印过程
    for (DWORD page = 1; page <= Job->TotalPages; page++) {
        if (!g_bRunning || Job->Status == MY_JOB_STATUS_CANCELLED) {
            printf("  Printing cancelled\n");
            DeleteMySpoolFile(Job);
            return FALSE;
        }
        
        Job->CurrentPage = page;
        printf("  Printing page %d of %d...\n", page, Job->TotalPages);
        
        // 模拟打印时间
        Sleep(500);
        
        // 发送到端口
        if (!SendToMyPort(Job)) {
            Job->Status = MY_JOB_STATUS_ERROR;
            DeleteMySpoolFile(Job);
            return FALSE;
        }
    }
    
    // 完成
    GetSystemTimeAsFileTime(&Job->EndTime);
    GetCurrentTimeString(timeStr, sizeof(timeStr));
    
    printf("[%s] Job #%d completed successfully\n", timeStr, Job->JobId);
    
    Job->Status = MY_JOB_STATUS_COMPLETED;
    DeleteMySpoolFile(Job);
    
    return TRUE;
}

// ========== 发送到端口 ==========
BOOL SendToMyPort(PMY_PRINT_JOB Job) {
    if (strcmp(Job->PortName, "FILE:") == 0) {
        // PDF 输出
        printf("    Output to: C:\\Users\\Public\\Documents\\%s.pdf\n", 
               Job->DocumentName);
    }
    else if (strstr(Job->PortName, "LPT") != NULL) {
        printf("    Sending to parallel port %s\n", Job->PortName);
    }
    else if (strstr(Job->PortName, "COM") != NULL) {
        printf("    Sending to serial port %s\n", Job->PortName);
    }
    else if (strstr(Job->PortName, "USB") != NULL) {
        printf("    Sending to USB port %s\n", Job->PortName);
    }
    else {
        printf("    Sending to %s\n", Job->PortName);
    }
    
    return TRUE;
}

// ========== 创建假脱机文件 ==========
BOOL CreateMySpoolFile(PMY_PRINT_JOB Job) {
    char spoolPath[MAX_PATH];
    sprintf_s(spoolPath, sizeof(spoolPath), 
              "C:\\Windows\\System32\\spool\\PRINTERS\\%05d.spl", 
              Job->JobId);
    
    HANDLE hFile = CreateFileA(spoolPath,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("    Failed to create spool file\n");
        return FALSE;
    }
    
    // 写入假脱机数据
    char buffer[1024];
    DWORD written;
    
    sprintf_s(buffer, sizeof(buffer),
              "Job ID: %d\r\n"
              "Document: %s\r\n"
              "Printer: %s\r\n"
              "Driver: %s\r\n"
              "Port: %s\r\n"
              "Pages: %d\r\n"
              "Status: %s\r\n",
              Job->JobId,
              Job->DocumentName,
              Job->PrinterName,
              Job->DriverName,
              Job->PortName,
              Job->TotalPages,
              GetMyJobStatusString(Job->Status));
    
    WriteFile(hFile, buffer, (DWORD)strlen(buffer), &written, NULL);
    CloseHandle(hFile);
    
    return TRUE;
}

// ========== 删除假脱机文件 ==========
BOOL DeleteMySpoolFile(PMY_PRINT_JOB Job) {
    char spoolPath[MAX_PATH];
    sprintf_s(spoolPath, sizeof(spoolPath), 
              "C:\\Windows\\System32\\spool\\PRINTERS\\%05d.spl", 
              Job->JobId);
    
    DeleteFileA(spoolPath);
    return TRUE;
}

// ========== 列出所有打印机 ==========
VOID ListMyPrinters(VOID) {
    printf("\nInstalled Printers:\n");
    printf("===================\n\n");
    
    if (g_PrinterCount == 0) {
        printf("No printers installed.\n");
        printf("Use 'ntprint /add <name> <port> <driver>' to add a printer.\n");
        return;
    }
    
    for (DWORD i = 0; i < g_PrinterCount; i++) {
        printf("Printer: %s\n", g_Printers[i].Name);
        printf("  Port:   %s\n", g_Printers[i].Port);
        printf("  Driver: %s\n", g_Printers[i].Driver);
        printf("  Status: %s\n", GetMyPrinterStatusString(g_Printers[i].Status));
        printf("  Jobs:   %d\n", g_Printers[i].JobCount);
        printf("\n");
    }
}

// ========== 列出打印作业 ==========
VOID ListMyJobs(LPCSTR PrinterName) {
    // 查找打印机
    PMY_PRINTER_INFO pPrinter = NULL;
    for (DWORD i = 0; i < g_PrinterCount; i++) {
        if (strcmp(g_Printers[i].Name, PrinterName) == 0) {
            pPrinter = &g_Printers[i];
            break;
        }
    }
    
    if (!pPrinter) {
        printf("Error: Printer '%s' not found\n", PrinterName);
        return;
    }
    
    printf("\nPrint Jobs for %s:\n", PrinterName);
    printf("=====================\n\n");
    
    if (pPrinter->JobCount == 0) {
        printf("No jobs in queue.\n");
        return;
    }
    
    PMY_PRINT_JOB pJob = pPrinter->Jobs;
    while (pJob) {
        printf("Job #%d\n", pJob->JobId);
        printf("  Document: %s\n", pJob->DocumentName);
        printf("  Pages:    %d\n", pJob->TotalPages);
        printf("  Status:   %s\n", GetMyJobStatusString(pJob->Status));
        printf("  Priority: %d\n", pJob->Priority);
        printf("\n");
        
        pJob = pJob->Next;
    }
}

// ========== 获取作业状态字符串 ==========
LPCSTR GetMyJobStatusString(DWORD Status) {
    switch (Status) {
        case MY_JOB_STATUS_QUEUED:    return "Queued";
        case MY_JOB_STATUS_PRINTING:  return "Printing";
        case MY_JOB_STATUS_PAUSED:    return "Paused";
        case MY_JOB_STATUS_COMPLETED: return "Completed";
        case MY_JOB_STATUS_ERROR:     return "Error";
        case MY_JOB_STATUS_CANCELLED: return "Cancelled";
        default:                      return "Unknown";
    }
}

// ========== 获取打印机状态字符串 ==========
LPCSTR GetMyPrinterStatusString(DWORD Status) {
    switch (Status) {
        case MY_PRINTER_STATUS_READY:   return "Ready";
        case MY_PRINTER_STATUS_PAUSED:  return "Paused";
        case MY_PRINTER_STATUS_ERROR:   return "Error";
        case MY_PRINTER_STATUS_PENDING: return "Pending";
        case MY_PRINTER_STATUS_OFFLINE: return "Offline";
        default:                        return "Unknown";
    }
}

// ========== 获取当前时间字符串 ==========
VOID GetCurrentTimeString(CHAR* Buffer, DWORD Size) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf_s(Buffer, Size, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
}

// ========== 打印工作线程 ==========
DWORD WINAPI MyPrintWorkerThread(LPVOID lpParam) {
    UNREFERENCED_PARAMETER(lpParam);
    
    while (g_bRunning) {
        ProcessMyPrintQueue();
        Sleep(1000);  // 每秒检查一次
    }
    
    return 0;
}

// ========== 清理打印系统 ==========
VOID CleanupMyPrintSystem(VOID) {
    if (g_hJobMutex) WaitForSingleObject(g_hJobMutex, INFINITE);
    
    // 清理所有作业
    PMY_PRINT_JOB pJob = g_JobQueue;
    while (pJob) {
        PMY_PRINT_JOB pNext = pJob->Next;
        DeleteMySpoolFile(pJob);
        free(pJob);
        pJob = pNext;
    }
    g_JobQueue = NULL;
    
    if (g_hJobMutex) {
        ReleaseMutex(g_hJobMutex);
        CloseHandle(g_hJobMutex);
        g_hJobMutex = NULL;
    }
    
    if (g_hStopEvent) {
        CloseHandle(g_hStopEvent);
        g_hStopEvent = NULL;
    }
    
    memset(g_Printers, 0, sizeof(g_Printers));
    g_PrinterCount = 0;
}

// ========== 控制台事件处理 ==========
BOOL WINAPI MyConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
        printf("\nReceived shutdown signal...\n");
        g_bRunning = FALSE;
        if (g_hStopEvent) {
            SetEvent(g_hStopEvent);
        }
        return TRUE;
    }
    return FALSE;
}
