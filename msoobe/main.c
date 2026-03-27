// main.c
// ControllerOS 操作系统 OOBE 主程序
// 位置: C:\ControllerOS\Windows\System32\oobe\msoobe.exe
// 编译: cl main.c oobe.c chooses.c /link user32.lib gdi32.lib advapi32.lib shell32.lib

#include <windows.h>
#include "oobe.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    OOBE_STATE state;
    
    OOBE_WriteLog("ControllerOS OOBE 启动");
    
    // 如果已完成，直接启动Windows桌面
    if (OOBE_IsCompleted())
    {
        OOBE_WriteLog("OOBE已完成，启动explorer.exe桌面");
        // 启动Windows资源管理器作为桌面
        ShellExecuteA(NULL, "open", "C:\\Windows\\explorer.exe", NULL, NULL, SW_SHOW);
        return 0;
    }
    
    // 初始化
    if (!OOBE_Initialize(&state, hInstance))
    {
        OOBE_WriteLog("OOBE初始化失败");
        return 1;
    }
    
    // 运行OOBE
    int result = OOBE_Start(&state);
    
    OOBE_Cleanup(&state);
    
    if (result == 0)
    {
        // 设置完成，启动explorer.exe
        OOBE_WriteLog("OOBE完成，启动explorer.exe");
        
        // 启动Windows资源管理器
        ShellExecuteA(NULL, "open", "C:\\Windows\\explorer.exe", NULL, NULL, SW_SHOW);
        
        // OOBE程序退出
        PostQuitMessage(0);
    }
    
    return result;
}
