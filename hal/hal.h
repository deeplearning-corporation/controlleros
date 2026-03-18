// hal.h - HAL 头文件
#ifndef _HAL_H_
#define _HAL_H_

#include <windef.h>
#include <winnt.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== 处理器信息 ==========
typedef struct _PROCESSOR_INFO {
    ULONG ProcessorId;
    ULONG Features;
    ULONG Stepping;
    ULONG Model;
    ULONG Family;
    ULONG Type;
    ULONG ExtendedFeatures;
    CHAR VendorString[16];
    CHAR BrandString[48];
} PROCESSOR_INFO, *PPROCESSOR_INFO;

// ========== 中断信息 ==========
typedef struct _INTERRUPT_INFO {
    ULONG Vector;
    ULONG Irql;
    ULONG Affinity;
    ULONG Type;
    ULONG Flags;
} INTERRUPT_INFO, *PINTERRUPT_INFO;

// ========== DMA信息 ==========
typedef struct _DMA_INFO {
    ULONG Alignment;
    ULONG Width;
    ULONG Speed;
    ULONG Port;
    ULONG Channel;
    ULONG TransferWidth;
    ULONG MaxTransferSize;
} DMA_INFO, *PDMA_INFO;

// ========== 固件类型 ==========
typedef enum _FIRMWARE_TYPE {
    FirmwareTypeUnknown,
    FirmwareTypeBios,
    FirmwareTypeEfi,
    FirmwareTypeUefi,
    FirmwareTypeMax
} FIRMWARE_TYPE, *PFIRMWARE_TYPE;

// ========== HAL函数 ==========

// 处理器相关
ULONG NTAPI HalGetProcessorCount(VOID);
ULONG NTAPI HalGetProcessorFeatures(VOID);
VOID NTAPI HalGetProcessorInfo(PPROCESSOR_INFO Info);
VOID NTAPI HalProcessorIdle(VOID);
VOID NTAPI HalRequestIpi(ULONG TargetProcessor);

// 定时器相关
ULONGLONG NTAPI HalReadTsc(VOID);
ULONGLONG NTAPI HalReadPmc(ULONG Counter);
ULONGLONG NTAPI HalQueryPerformanceCounter(VOID);
VOID NTAPI HalStallExecution(ULONG Microseconds);
VOID NTAPI HalCalibratePerformanceCounter(VOID);

// 中断相关
ULONG NTAPI HalGetInterruptVector(ULONG Irql, ULONG Processor);
VOID NTAPI HalEnableInterrupts(VOID);
VOID NTAPI HalDisableInterrupts(VOID);
BOOLEAN NTAPI HalGetInterruptInformation(ULONG Vector, PINTERRUPT_INFO Info);

// DMA相关
ULONG NTAPI HalGetDmaAlignment(VOID);
ULONG NTAPI HalGetDmaRequirement(VOID);
PVOID NTAPI HalAllocateMapRegisters(ULONG Channel, ULONG Count);
VOID NTAPI HalFreeMapRegisters(PVOID MapRegisterBase);

// PCI相关
NTSTATUS NTAPI HalGetPciConfiguration(ULONG Bus, ULONG Device, ULONG Function, PVOID Buffer, ULONG Length);
NTSTATUS NTAPI HalSetPciConfiguration(ULONG Bus, ULONG Device, ULONG Function, PVOID Buffer, ULONG Length);
ULONG NTAPI HalGetPciIntLine(ULONG Bus, ULONG Device, ULONG Function);
VOID NTAPI HalSetPciIntLine(ULONG Bus, ULONG Device, ULONG Function, ULONG Line);

// 固件相关
FIRMWARE_TYPE NTAPI HalGetFirmwareType(VOID);
NTSTATUS NTAPI HalGetFirmwareEnvironmentVariable(PCWSTR Name, PCWSTR Vendor, PVOID Buffer, ULONG Size);
NTSTATUS NTAPI HalSetFirmwareEnvironmentVariable(PCWSTR Name, PCWSTR Vendor, PVOID Buffer, ULONG Size);
VOID NTAPI HalReturnToFirmware(ULONG Routine);

// 系统信息
NTSTATUS NTAPI HalGetSystemInformation(ULONG Class, PVOID Buffer, ULONG Length);
NTSTATUS NTAPI HalSetSystemInformation(ULONG Class, PVOID Buffer, ULONG Length);

// 初始化
BOOLEAN NTAPI HalInitializeProcessor(ULONG ProcessorNumber);
BOOLEAN NTAPI HalStartNextProcessor(ULONG ProcessorNumber);
VOID NTAPI HalInitialize(VOID);

#ifdef __cplusplus
}
#endif

#endif // _HAL_H_
