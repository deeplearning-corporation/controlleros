// oobe.c
// ControllerOS OOBE 核心逻辑

#include <windows.h>
#include <stdio.h>
#include "oobe.h"

#define REG_KEY_CONTROLLEROS "SOFTWARE\\ControllerOS"
#define REG_VALUE_COMPLETED "OOBECompleted"
#define REG_VALUE_PHASE "OOBEPhase"
#define LOG_FILE "C:\\Windows\\System32\\LogFiles\\oobe.log"

// 外部函数声明
extern BOOL ShowLanguageChoose(HWND hWnd, char* pszLanguage);
extern BOOL ShowTimezoneChoose(HWND hWnd, char* pszTimezone);
extern BOOL ShowAccountCreate(HWND hWnd, char* pszUserName);

BOOL OOBE_Initialize(OOBE_STATE* pState, HINSTANCE hInstance)
{
    if (!pState) return FALSE;
    
    ZeroMemory(pState, sizeof(OOBE_STATE));
    pState->hInstance = hInstance;
    pState->currentPhase = OOBE_GetCurrentPhase();
    
    // 默认值
    strcpy_s(pState->szLanguage, 64, "zh-CN");
    strcpy_s(pState->szTimezone, 128, "China Standard Time");
    
    OOBE_WriteLog("OOBE初始化完成");
    return TRUE;
}

void OOBE_Cleanup(OOBE_STATE* pState)
{
    if (pState)
    {
        ZeroMemory(pState, sizeof(OOBE_STATE));
    }
    OOBE_WriteLog("OOBE清理完成");
}

int OOBE_Start(OOBE_STATE* pState)
{
    OOBE_WriteLog("开始OOBE流程");
    
    // 欢迎界面
    MessageBoxA(NULL, "欢迎使用 ControllerOS 操作系统\n\n点击确定开始设置", 
                "ControllerOS", MB_OK | MB_ICONINFORMATION);
    
    // 语言选择
    if (pState->currentPhase <= OOBE_PHASE_LANGUAGE)
    {
        if (!ShowLanguageChoose(NULL, pState->szLanguage))
            return 1;
        pState->currentPhase = OOBE_PHASE_LANGUAGE;
        OOBE_SavePhase(pState->currentPhase);
    }
    
    // 时区选择
    if (pState->currentPhase <= OOBE_PHASE_TIMEZONE)
    {
        if (!ShowTimezoneChoose(NULL, pState->szTimezone))
            return 1;
        pState->currentPhase = OOBE_PHASE_TIMEZONE;
        OOBE_SavePhase(pState->currentPhase);
    }
    
    // 账户创建
    if (pState->currentPhase <= OOBE_PHASE_ACCOUNT)
    {
        if (!ShowAccountCreate(NULL, pState->szUserName))
            return 1;
        pState->currentPhase = OOBE_PHASE_ACCOUNT;
        OOBE_SavePhase(pState->currentPhase);
    }
    
    // 应用设置
    OOBE_WriteLog("应用系统设置");
    
    // 保存配置到注册表
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_CONTROLLEROS, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, "Language", 0, REG_SZ, (BYTE*)pState->szLanguage,
            (DWORD)strlen(pState->szLanguage) + 1);
        RegSetValueExA(hKey, "Timezone", 0, REG_SZ, (BYTE*)pState->szTimezone,
            (DWORD)strlen(pState->szTimezone) + 1);
        RegCloseKey(hKey);
    }
    
    // 标记完成
    OOBE_MarkCompleted();
    
    MessageBoxA(NULL, "ControllerOS 设置完成！\n\n即将启动桌面", 
                "ControllerOS", MB_OK | MB_ICONINFORMATION);
    
    OOBE_WriteLog("OOBE流程完成");
    return 0;
}

OOBE_PHASE OOBE_GetCurrentPhase(void)
{
    HKEY hKey;
    DWORD dwPhase = OOBE_PHASE_WELCOME;
    DWORD dwSize = sizeof(dwPhase);
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_CONTROLLEROS, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExA(hKey, REG_VALUE_PHASE, NULL, NULL, (BYTE*)&dwPhase, &dwSize);
        RegCloseKey(hKey);
    }
    
    return (OOBE_PHASE)dwPhase;
}

void OOBE_SavePhase(OOBE_PHASE phase)
{
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_CONTROLLEROS, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        DWORD dwPhase = phase;
        RegSetValueExA(hKey, REG_VALUE_PHASE, 0, REG_DWORD, (BYTE*)&dwPhase, sizeof(dwPhase));
        RegCloseKey(hKey);
    }
}

void OOBE_MarkCompleted(void)
{
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_CONTROLLEROS, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        DWORD dwCompleted = 1;
        RegSetValueExA(hKey, REG_VALUE_COMPLETED, 0, REG_DWORD, (BYTE*)&dwCompleted, sizeof(dwCompleted));
        RegCloseKey(hKey);
    }
}

BOOL OOBE_IsCompleted(void)
{
    HKEY hKey;
    DWORD dwCompleted = 0;
    DWORD dwSize = sizeof(dwCompleted);
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_CONTROLLEROS, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExA(hKey, REG_VALUE_COMPLETED, NULL, NULL, (BYTE*)&dwCompleted, &dwSize);
        RegCloseKey(hKey);
        return (dwCompleted == 1);
    }
    
    return FALSE;
}

void OOBE_WriteLog(const char* lpszMsg)
{
    HANDLE hFile;
    DWORD dwWritten;
    SYSTEMTIME st;
    char szBuffer[512];
    
    // 确保日志目录存在
    CreateDirectoryA("C:\\Windows\\System32\\LogFiles", NULL);
    
    hFile = CreateFileA(LOG_FILE, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE)
    {
        GetLocalTime(&st);
        
        sprintf_s(szBuffer, sizeof(szBuffer), "[%04d-%02d-%02d %02d:%02d:%02d] %s\r\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, lpszMsg);
        
        WriteFile(hFile, szBuffer, (DWORD)strlen(szBuffer), &dwWritten, NULL);
        CloseHandle(hFile);
    }
}
