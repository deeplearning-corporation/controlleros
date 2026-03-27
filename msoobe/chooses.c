// chooses.c
// ControllerOS OOBE 选择界面

#include <windows.h>
#include <stdio.h>
#include "oobe.h"

// 语言选择对话框过程
INT_PTR CALLBACK LanguageDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char* pszResult = (char*)GetWindowLongPtrA(hDlg, GWLP_USERDATA);
    
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            SetWindowTextA(hDlg, "ControllerOS - 选择语言");
            SetWindowLongPtrA(hDlg, GWLP_USERDATA, lParam);
            pszResult = (char*)lParam;
            
            HWND hList = GetDlgItem(hDlg, 1001);
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"中文(简体)");
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"English");
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"日本語");
            SendMessageA(hList, CB_SETCURSEL, 0, 0);
            
            SetDlgItemTextA(hDlg, 1002, "请选择您的语言:");
            SetDlgItemTextA(hDlg, 1003, "下一步");
            return TRUE;
        }
        
    case WM_COMMAND:
        if (LOWORD(wParam) == 1003)
        {
            HWND hList = GetDlgItem(hDlg, 1001);
            int nSel = (int)SendMessage(hList, CB_GETCURSEL, 0, 0);
            
            switch(nSel)
            {
            case 0: strcpy_s(pszResult, 64, "zh-CN"); break;
            case 1: strcpy_s(pszResult, 64, "en-US"); break;
            case 2: strcpy_s(pszResult, 64, "ja-JP"); break;
            default: strcpy_s(pszResult, 64, "zh-CN"); break;
            }
            
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    
    return FALSE;
}

BOOL ShowLanguageChoose(HWND hWnd, char* pszLanguage)
{
    INT_PTR result = DialogBoxParamA(GetModuleHandleA(NULL), 
        MAKEINTRESOURCEA(1000), hWnd, LanguageDlgProc, (LPARAM)pszLanguage);
    return (result == IDOK);
}

// 时区选择对话框过程
INT_PTR CALLBACK TimezoneDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char* pszResult = (char*)GetWindowLongPtrA(hDlg, GWLP_USERDATA);
    
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            SetWindowTextA(hDlg, "ControllerOS - 选择时区");
            SetWindowLongPtrA(hDlg, GWLP_USERDATA, lParam);
            pszResult = (char*)lParam;
            
            HWND hList = GetDlgItem(hDlg, 2001);
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"(UTC+08:00) 北京, 上海, 香港");
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"(UTC+09:00) 东京, 首尔");
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"(UTC-05:00) 纽约, 华盛顿");
            SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)"(UTC+00:00) 伦敦, 都柏林");
            SendMessageA(hList, CB_SETCURSEL, 0, 0);
            
            SetDlgItemTextA(hDlg, 2002, "请选择您的时区:");
            SetDlgItemTextA(hDlg, 2003, "下一步");
            return TRUE;
        }
        
    case WM_COMMAND:
        if (LOWORD(wParam) == 2003)
        {
            HWND hList = GetDlgItem(hDlg, 2001);
            int nSel = (int)SendMessage(hList, CB_GETCURSEL, 0, 0);
            
            switch(nSel)
            {
            case 0: strcpy_s(pszResult, 128, "China Standard Time"); break;
            case 1: strcpy_s(pszResult, 128, "Tokyo Standard Time"); break;
            case 2: strcpy_s(pszResult, 128, "Eastern Standard Time"); break;
            case 3: strcpy_s(pszResult, 128, "GMT Standard Time"); break;
            default: strcpy_s(pszResult, 128, "China Standard Time"); break;
            }
            
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    
    return FALSE;
}

BOOL ShowTimezoneChoose(HWND hWnd, char* pszTimezone)
{
    INT_PTR result = DialogBoxParamA(GetModuleHandleA(NULL), 
        MAKEINTRESOURCEA(2000), hWnd, TimezoneDlgProc, (LPARAM)pszTimezone);
    return (result == IDOK);
}

// 账户创建对话框过程
INT_PTR CALLBACK AccountDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char* pszResult = (char*)GetWindowLongPtrA(hDlg, GWLP_USERDATA);
    
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            SetWindowTextA(hDlg, "ControllerOS - 创建用户账户");
            SetWindowLongPtrA(hDlg, GWLP_USERDATA, lParam);
            pszResult = (char*)lParam;
            
            SetDlgItemTextA(hDlg, 3001, "用户名:");
            SetDlgItemTextA(hDlg, 3002, "密码(可选):");
            SetDlgItemTextA(hDlg, 3003, "确认密码:");
            SetDlgItemTextA(hDlg, 3004, "创建账户");
            
            // 默认用户名
            SetDlgItemTextA(hDlg, 3005, "Controller");
            return TRUE;
        }
        
    case WM_COMMAND:
        if (LOWORD(wParam) == 3004)
        {
            GetDlgItemTextA(hDlg, 3005, pszResult, 256);
            
            if (strlen(pszResult) == 0)
            {
                MessageBoxA(hDlg, "请输入用户名", "错误", MB_OK | MB_ICONERROR);
                return TRUE;
            }
            
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    
    return FALSE;
}

BOOL ShowAccountCreate(HWND hWnd, char* pszUserName)
{
    INT_PTR result = DialogBoxParamA(GetModuleHandleA(NULL), 
        MAKEINTRESOURCEA(3000), hWnd, AccountDlgProc, (LPARAM)pszUserName);
    return (result == IDOK);
}
