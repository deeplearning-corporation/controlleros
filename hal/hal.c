// hal.c - ¼ò»¯°æ
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}

DWORD WINAPI HalGetProcessorCount(VOID) {
    return 1;
}
