#pragma once

#include "Driver.h"
#include "running_environment.h"

#define RD_MSR					1
#define RD_MSR_TX				2
#define READ_PCI_CONFIG_DWORD	3
#define READ_IO_PORT_DWORD		4
#define WRITE_IO_PORT_DWORD		5

#define IOCTL_OLS_READ_MSR				CTL_CODE(OLS_TYPE, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OLS_READ_PCI_CONFIG		CTL_CODE(OLS_TYPE, 0x851, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_OLS_READ_IO_PORT_DWORD	CTL_CODE(OLS_TYPE, 0x835, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_OLS_WRITE_IO_PORT_DWORD	CTL_CODE(OLS_TYPE, 0x838, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct  _OLS_READ_PCI_CONFIG_INPUT {
	ULONG PciAddress;
	ULONG PciOffset;
}OLS_READ_PCI_CONFIG_INPUT;
typedef struct  _OLS_WRITE_IO_PORT_INPUT {
	ULONG	PortNumber;
	union {
		ULONG   LongData;
		USHORT  ShortData;
		UCHAR   CharData;
	};
}OLS_WRITE_IO_PORT_INPUT;

BOOL WINAPI Rdmsr(DWORD index, PDWORD eax, PDWORD edx, HANDLE gHandle);
BOOL WINAPI RdmsrTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask, HANDLE gHandle);
BOOL pciConfigRead(DWORD pciAddress, DWORD regAddress, PBYTE value, DWORD size, PDWORD error, HANDLE gHandle);
DWORD WINAPI ReadPciConfigDword(DWORD pciAddress, BYTE regAddress, HANDLE gHandle);
DWORD WINAPI ReadIoPortDword(WORD port, HANDLE gHandle);
BOOL WINAPI WriteIoPortDword(WORD port, DWORD value, HANDLE gHandle);
BOOL DriverFunc(
	INT func,
	DWORD index = 0,
	PDWORD eax = NULL,
	PDWORD edx = NULL,
	DWORD_PTR threadAffinityMask = 1,
	DWORD pciAddress = 0,
	BYTE regAddress = 0,
	WORD port = 0,
	DWORD value = 0);