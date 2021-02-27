#include "CPUTempFunc.h"

DWORD GetCoreCount()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

UINT GetTjMax()
{
	if (TjMax == 0)
	{
		DWORD eax = 0, edx = 0;
		DriverFunc(RD_MSR, 0x1A2, &eax, &edx);
		TjMax = (eax & 0xff0000) >> 16;
	}
	return TjMax;
}

UINT GetTemp(BYTE index)
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
		DriverFunc(RD_MSR_TX, 0x19C, &eax, &edx, index);
		return TjMax - ((eax & 0x7f0000) >> 16);
	}
}

UINT GetHighestTemp()
{
	UINT MaxTemp = 0;
	DWORD coreCount = GetCoreCount();

	for (BYTE i = 1; i <= coreCount; i++)
	{
		UINT Temp = GetTemp(i);
		if (MaxTemp < Temp) MaxTemp = Temp;
	}
	return MaxTemp;
}