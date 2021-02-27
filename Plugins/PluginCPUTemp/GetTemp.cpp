#include "GetTemp.h"

#ifdef _DEBUG
BOOL get_bus_dev(UINT devieid, INT* BUS, INT* DEV)
{
	UINT address, value;
	for (UINT bus = 0; bus < 128; bus++)
	{
		for (UINT dev = 0; dev < 32; dev++)
		{
			for (UINT func = 0; func < 8; func++)
			{
				address = PciBusDevFunc(bus, dev, func);
				value = DriverFunc(READ_PCI_CONFIG_DWORD, 0, 0, 0, 0, address, 0x00);
				if (value == devieid)
				{
					*BUS = bus;
					*DEV = dev;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

DWORD get_temp()
{
	INT bus, dev;
	if (!get_bus_dev(0x1103, &bus, &dev))
	{
		return FALSE;
	}

	UINT slot = DeviceSlot(dev, 0x3);
	UINT IO_ADDRE = GetDevice(bus, slot, 0xE4);

	DWORD CPUTemp = 0;

	if (DriverFunc(WRITE_IO_PORT_DWORD, 0, 0, 0, 0, 0, 0, 0xCF8, IO_ADDRE)) CPUTemp = DriverFunc(READ_IO_PORT_DWORD, 0, 0, 0, 0, 0, 0, 0xCFC, IO_ADDRE);

	INT CPUInfo[4], g_Offset, K = 0;

	__cpuid(CPUInfo, 1);
	INT t = CPUInfo[0];
	INT family = ((t >> 20) & 0xFF) + ((t >> 8) & 0xF), model = ((t >> 12) & 0xF0) + ((t >> 4) & 0xF), stepping = t & 0xF;

	if (family == 0xF && !(((model == 4) && (stepping == 0)) || ((model == 5) && (stepping <= 1)))) K = 8;
	if (family > 0xF) K = 10;

	if (model >= 0x69 && model != 0xc1 && model != 0x6c && model != 0x7c) g_Offset = 49 - 21;
	else g_Offset = 49;

	CPUTemp = (CPUTemp >> 16) & 0xFF;

	if (K == 8) CPUTemp -= g_Offset;
	if (K == 10) CPUTemp /= 8;

	return CPUTemp;
}

DWORD GetAMDTemp(DWORD_PTR threadAffinityMask)
{
	DWORD		result = FALSE;
	DWORD_PTR	mask = 0;
	HANDLE		hThread = NULL;

	if (IsNT())
	{
		hThread = GetCurrentThread();
		mask = SetThreadAffinityMask(hThread, threadAffinityMask);
		if (mask == 0)
		{
			return FALSE;
		}
	}

	result = get_temp();

	if (IsNT() && hThread != NULL)
	{
		SetThreadAffinityMask(hThread, mask);
	}

	return result;
}
#endif

int _GetTjMax()
{
	if (TjMax == 0)
	{
		DWORD eax = 0, edx = 0;
		DriverFunc(RD_MSR, 0x1A2, &eax, &edx);
		TjMax = (eax & 0xff0000) >> 16;
	}
	return TjMax;
}

int _GetTemp(int _index)
{
	if (TjMax == 0) _GetTjMax();

	if (!IsNT()) return 0;

	if (IsAMD())
	{
		//return GetAMDTemp(_index);
		return 0;
	}
	else
	{
		if (!IsMsr()) return 0;
		DWORD eax = 0, edx = 0;
		DriverFunc(RD_MSR_TX, 0x19C, &eax, &edx, _index);
		return TjMax - ((eax & 0x7f0000) >> 16);
	}
}