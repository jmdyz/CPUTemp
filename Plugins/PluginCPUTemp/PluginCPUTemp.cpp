#include "PluginCPUTemp.h"

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	MeasureData* measure = new MeasureData;
	*data = measure;
	measure->rm = rm;

	TCHAR gDriverPath[MAX_PATH];
	RuningEn(gDriverPath);
	pDriver = new Driver(OLS_DRIVER_ID, gDriverPath);
	gHandle = pDriver->GetHandle();
	if (gHandle == INVALID_HANDLE_VALUE) return;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	UNREFERENCED_PARAMETER(maxValue);
	MeasureData* measure = (MeasureData*)data;

	LPCWSTR value = RmReadString(rm, L"CPUTempType", L"Temperature");
	if (!_wcsicmp(value, L"Temperature"))
	{
		measure->type = eMeasureType::MeasureTemperature;
	}
	else if (!_wcsicmp(value, L"MaxTemperature"))
	{
		measure->type = eMeasureType::MeasureMaxTemperature;
	}
	else if (!_wcsicmp(value, L"TjMax"))
	{
		measure->type = eMeasureType::MeasureTjMax;
	}
	else
	{
		measure->type = eMeasureType::MeasureTemperature;
		RmLogF(rm, LOG_WARNING, L"ÎÞÐ§µÄCPUTempType: %s", value);
	}
	measure->index = RmReadInt(rm, L"CPUTempIndex", 1);
}

PLUGIN_EXPORT double Update(void* data)
{
	MeasureData* measure = (MeasureData*)data;
	static ULONGLONG oldTime = _GetTickCount64();
	double result = 0;
	ULONGLONG time = _GetTickCount64();

	if (time - oldTime >= 50)
	{
		HANDLE hTread = CreateThread(NULL, 0, Thread, data, 0, NULL);
		if (hTread) CloseHandle(hTread);
		WaitForSingleObject(hMutex, 50);
		result = temp;
		ReleaseMutex(hMutex);
		oldTime = time;
	}

	return result;
}

PLUGIN_EXPORT void Finalize(void* data)
{
	MeasureData* measure = (MeasureData*)data;
	delete measure;
	delete pDriver;
}

ULONGLONG _GetTickCount64()
{
	typedef ULONGLONG(WINAPI* FPGETTICKCOUNT64)();
	static FPGETTICKCOUNT64 c_GetTickCount64 = (FPGETTICKCOUNT64)GetProcAddress(GetModuleHandle(L"kernel32"), "GetTickCount64");

	if (c_GetTickCount64)
	{
		return c_GetTickCount64();
	}
	else
	{
		static ULONGLONG lastTicks = 0;
		ULONGLONG ticks = GetTickCount();
		while (ticks < lastTicks) ticks += 0x100000000;
		lastTicks = ticks;
		return ticks;
	}
}

DWORD WINAPI Thread(LPVOID data)
{
	MeasureData* measure = (MeasureData*)data;
	WaitForSingleObject(hMutex, INFINITE);
	if (temp - cache <= 20 || cache - temp <= 20 || temp == 0 || cache == 0)
	{
		switch (measure->type)
		{
		case eMeasureType::MeasureTemperature:
		{
			cache = temp;
			temp = GetHighestTemp();
			break;
		}
		case eMeasureType::MeasureMaxTemperature:
		{
			cache = temp;
			temp = GetTemp(measure->index);
			break;
		}
		case eMeasureType::MeasureTjMax:
			temp = GetTjMax();
			break;
		}
	}
	ReleaseMutex(hMutex);
	return 0L;
}

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
		Rdmsr(0x1A2, &eax, &edx, gHandle);
		TjMax = (eax & 0xff0000) >> 16;
	}
	return TjMax;
}

UINT GetTemp(BYTE index)
{
	if (TjMax == 0) GetTjMax();

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
		RdmsrTx(0x19C, &eax, &edx, index, gHandle);
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