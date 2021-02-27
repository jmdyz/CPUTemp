#include <windows.h>
#include <wchar.h>
#include "CPUTempFunc.h"
#include "../../Library/Export.h"

enum class eMeasureType
{
	MeasureTemperature,
	MeasureMaxTemperature,
	MeasureTjMax,
	MeasureLoad,
	MeasureVid,
	MeasureCpuSpeed,
	MeasureBusSpeed,
	MeasureBusMultiplier,
	MeasureCpuName,
	MeasureCoreSpeed,
	MeasureCoreBusMultiplier,
	MeasureTdp,
	MeasurePower,
};

struct MeasureData
{
	eMeasureType type;
	int index;
	void* rm;

	MeasureData() :
		type(eMeasureType::MeasureTemperature),
		index(0),
		rm(nullptr) {}
};

eMeasureType convertStringToMeasureType(LPCWSTR i_String, void* rm);

/************************************************************************
*
* 函 数 名：Initialize
* 函数功能：初始化data，使之成为MeasureData类型数据
* 输入参数：
** void** data	数据指针
** void* rm		Rainmeter运行指针
* 输出参数：void
* 返 回 值: void
*
************************************************************************/
PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	MeasureData* measure = new MeasureData;
	*data = measure;
	measure->rm = rm;
}

/************************************************************************
*
* 函 数 名：Reload
* 函数功能：读取用户配置参数重载data
* 输入参数：
** void* data		数据
** void* rm			Rainmeter运行指针
** double* maxValue	未知
* 输出参数：void
* 返 回 值: void
*
************************************************************************/
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	UNREFERENCED_PARAMETER(maxValue);
	MeasureData* measure = (MeasureData*)data;

	LPCWSTR value = RmReadString(rm, L"CPUTempType", L"Temperature");
	measure->type = convertStringToMeasureType(value, rm);
	measure->index = RmReadInt(rm, L"CPUTempIndex", 1);
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

int temp = 0;
int cache = 0;
HANDLE hMutex = nullptr;

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

		case eMeasureType::MeasureLoad:
			break;

		case eMeasureType::MeasureVid:
			break;

		case eMeasureType::MeasureCpuSpeed:
			break;

		case eMeasureType::MeasureBusSpeed:
			break;

		case eMeasureType::MeasureBusMultiplier:
			break;

		case eMeasureType::MeasureCoreSpeed:
			break;

		case eMeasureType::MeasureCoreBusMultiplier:
			break;

		case eMeasureType::MeasureTdp:
			break;

		case eMeasureType::MeasurePower:
			break;
		}
	}
	ReleaseMutex(hMutex);
	return 0L;
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

//PLUGIN_EXPORT LPCWSTR GetString(void* data)
//{
//	MeasureData* measure = (MeasureData*)data;
//	static WCHAR buffer[128];
//
//	switch (measure->type)
//	{
//	case eMeasureType::MeasureVid:
//		_snwprintf_s(buffer, _TRUNCATE, L"%.4f", 0.0);
//		break;
//
//	case eMeasureType::MeasureCpuName:
//		_snwprintf_s(buffer, _TRUNCATE, L"%S", "0");
//		break;
//
//	default:
//		return nullptr;
//	}
//
//	return buffer;
//}

PLUGIN_EXPORT void Finalize(void* data)
{
	MeasureData* measure = (MeasureData*)data;
	delete measure;
}

bool areStringsEqual(LPCWSTR i_String1, LPCWSTR i_Strting2)
{
	return _wcsicmp(i_String1, i_Strting2) == 0;
}

eMeasureType convertStringToMeasureType(LPCWSTR i_String, void* rm)
{
	eMeasureType result;

	if (areStringsEqual(i_String, L"Temperature"))
	{
		result = eMeasureType::MeasureTemperature;
	}
	else if (areStringsEqual(i_String, L"MaxTemperature"))
	{
		result = eMeasureType::MeasureMaxTemperature;
	}
	else if (areStringsEqual(i_String, L"TjMax"))
	{
		result = eMeasureType::MeasureTjMax;
	}
	else if (areStringsEqual(i_String, L"Load"))
	{
		result = eMeasureType::MeasureLoad;
	}
	else if (areStringsEqual(i_String, L"Vid"))
	{
		result = eMeasureType::MeasureVid;
	}
	else if (areStringsEqual(i_String, L"CpuSpeed"))
	{
		result = eMeasureType::MeasureCpuSpeed;
	}
	else if (areStringsEqual(i_String, L"BusSpeed"))
	{
		result = eMeasureType::MeasureBusSpeed;
	}
	else if (areStringsEqual(i_String, L"BusMultiplier"))
	{
		result = eMeasureType::MeasureBusMultiplier;
	}
	else if (areStringsEqual(i_String, L"CpuName"))
	{
		result = eMeasureType::MeasureCpuName;
	}
	else if (areStringsEqual(i_String, L"CoreSpeed"))
	{
		result = eMeasureType::MeasureCoreSpeed;
	}
	else if (areStringsEqual(i_String, L"CoreBusMultiplier"))
	{
		result = eMeasureType::MeasureCoreBusMultiplier;
	}
	else if (areStringsEqual(i_String, L"Tdp"))
	{
		result = eMeasureType::MeasureTdp;
	}
	else if (areStringsEqual(i_String, L"Power"))
	{
		result = eMeasureType::MeasurePower;
	}
	else
	{
		result = eMeasureType::MeasureTemperature;
		RmLogF(rm, LOG_WARNING, L"无效的CPUTempType: %s", i_String);
	}

	return result;
}