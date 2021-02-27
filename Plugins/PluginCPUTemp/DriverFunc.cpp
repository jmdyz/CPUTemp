#include "DriverFunc.h"

BOOL WINAPI Rdmsr(DWORD index, PDWORD eax, PDWORD edx, HANDLE gHandle)
{
	if (eax == NULL || edx == NULL) return FALSE;

	DWORD	returnedLength = 0;
	BOOL	result = FALSE;
	BYTE	outBuf[8] = { 0 };

	result = DeviceIoControl(
		gHandle,				// ��Ҫִ�в������豸��������豸ͨ���Ǿ�Ŀ¼���ļ�������ʹ�� CreateFile �����򿪻�ȡ�豸���������ļ���ע
		IOCTL_OLS_READ_MSR,		// �����Ŀ��ƴ��룬��ֵ��ʶҪִ�е��ض������Լ�ִ�иò������豸�����ͣ��йؿ��ƴ�����б���ο���ע��
		&index,					// ����ѡ��ָ�����뻺������ָ�롣
		sizeof(index),			// ���뻺�������ֽ�Ϊ��λ�Ĵ�С����λΪ�ֽڡ�
		&outBuf,				// ����ѡ��ָ�������������ָ�롣
		sizeof(outBuf),			// ������������ֽ�Ϊ��λ�Ĵ�С����λΪ�ֽڡ�
		&returnedLength,		// ����ѡ��ָ��һ��������ָ�룬�ñ������մ洢������������е����ݵĴ�С��
		NULL					// ����ѡ��ָ��OVERLAPPED�ṹ��ָ�롣
	);

	if (result)
	{
		memcpy(eax, outBuf, 4);
		memcpy(edx, outBuf + 4, 4);
		return TRUE;
	}
	else return FALSE;
}

BOOL WINAPI RdmsrTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask, HANDLE gHandle)
{
	BOOL		result = FALSE;
	DWORD_PTR	mask = 0;
	HANDLE		hThread = NULL;

	hThread = GetCurrentThread();
	mask = SetThreadAffinityMask(hThread, threadAffinityMask);
	if (mask == 0) return FALSE;

	result = Rdmsr(index, eax, edx, gHandle);

	if (hThread) SetThreadAffinityMask(hThread, mask);

	return result;
}

BOOL pciConfigRead(DWORD pciAddress, DWORD regAddress, PBYTE value, DWORD size, PDWORD error, HANDLE gHandle)
{
	if (value == NULL || (size == 2 && (regAddress & 1) != 0) || (size == 4 && (regAddress & 3) != 0))
	{
		return FALSE;
	}

	DWORD	returnedLength = 0;
	BOOL	result = FALSE;
	OLS_READ_PCI_CONFIG_INPUT inBuf{};

	inBuf.PciAddress = pciAddress;
	inBuf.PciOffset = regAddress;

	result = DeviceIoControl(
		gHandle,
		IOCTL_OLS_READ_PCI_CONFIG,
		&inBuf,
		sizeof(inBuf),
		value,
		size,
		&returnedLength,
		NULL
	);

	if (error != NULL)
	{
		*error = GetLastError();
	}

	if (result) return TRUE;
	else return FALSE;
}

DWORD WINAPI ReadPciConfigDword(DWORD pciAddress, BYTE regAddress, HANDLE gHandle)
{
	DWORD ret = 0;
	if (pciConfigRead(pciAddress, regAddress, (PBYTE)&ret, sizeof(ret), NULL, gHandle))
	{
		return ret;
	}
	else
	{
		return 0xFFFFFFFF;
	}
}

DWORD WINAPI ReadIoPortDword(WORD port, HANDLE gHandle)
{
	DWORD	returnedLength = 0;
	BOOL	result = FALSE;
	DWORD	port4 = port;
	DWORD	value = 0;

	result = DeviceIoControl(
		gHandle,
		IOCTL_OLS_READ_IO_PORT_DWORD,
		&port4,
		sizeof(port4),	// required 4 bytes
		&value,
		sizeof(value),
		&returnedLength,
		NULL
	);

	return value;
}

BOOL WINAPI WriteIoPortDword(WORD port, DWORD value, HANDLE gHandle)
{
	DWORD	returnedLength = 0;
	BOOL	result = FALSE;
	DWORD   length = 0;
	OLS_WRITE_IO_PORT_INPUT inBuf{};

	inBuf.LongData = value;
	inBuf.PortNumber = port;
	length = offsetof(OLS_WRITE_IO_PORT_INPUT, CharData) + sizeof(inBuf.LongData);

	result = DeviceIoControl(
		gHandle,
		IOCTL_OLS_WRITE_IO_PORT_DWORD,
		&inBuf,
		length,
		NULL,
		0,
		&returnedLength,
		NULL
	);

	if (result) return TRUE;
	else return FALSE;
}

BOOL DriverFunc(INT func, DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask, DWORD pciAddress, BYTE regAddress, WORD port, DWORD value)
{
	TCHAR gDriverPath[MAX_PATH];
	RuningEn(gDriverPath);
	Driver* pDriver = new Driver(OLS_DRIVER_ID, gDriverPath);
	HANDLE gHandle = pDriver->GetHandle();
	if (gHandle == INVALID_HANDLE_VALUE) return FALSE;

	switch (func)
	{
	case RD_MSR:
		Rdmsr(index, eax, edx, gHandle);
		break;
	case RD_MSR_TX:
		RdmsrTx(index, eax, edx, threadAffinityMask, gHandle);
		break;
	case READ_PCI_CONFIG_DWORD:
		ReadPciConfigDword(pciAddress, regAddress, gHandle);
		break;
	case READ_IO_PORT_DWORD:
		ReadIoPortDword(port, gHandle);
		break;
	case WRITE_IO_PORT_DWORD:
		WriteIoPortDword(port, value, gHandle);
		break;
	default:
		break;
	}
	delete pDriver;
	return TRUE;
}