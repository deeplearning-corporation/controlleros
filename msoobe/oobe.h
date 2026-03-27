// oobe.h
// ControllerOS 操作系统 OOBE 头文件

#ifndef _OOBE_H_
#define _OOBE_H_

#include <windows.h>
#include <stdio.h>

// OOBE阶段
typedef enum {
    OOBE_PHASE_WELCOME = 0,
    OOBE_PHASE_LANGUAGE,
    OOBE_PHASE_TIMEZONE,
    OOBE_PHASE_ACCOUNT,
    OOBE_PHASE_COMPLETE
} OOBE_PHASE;

// OOBE状态
typedef struct {
    HINSTANCE hInstance;
    OOBE_PHASE currentPhase;
    char szLanguage[64];
    char szTimezone[128];
    char szUserName[256];
} OOBE_STATE;

// 函数声明
BOOL OOBE_Initialize(OOBE_STATE* pState, HINSTANCE hInstance);
void OOBE_Cleanup(OOBE_STATE* pState);
int OOBE_Start(OOBE_STATE* pState);
OOBE_PHASE OOBE_GetCurrentPhase(void);
void OOBE_SavePhase(OOBE_PHASE phase);
void OOBE_MarkCompleted(void);
BOOL OOBE_IsCompleted(void);
void OOBE_WriteLog(const char* lpszMsg);

#endif
