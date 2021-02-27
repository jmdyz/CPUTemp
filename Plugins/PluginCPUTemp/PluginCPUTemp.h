#pragma once
#include <Windows.h>
#include <wchar.h>
#include "../../Library/Export.h"
#include "RunningEnvironment.h"
#include "Driver.h"
#include "DriverFunc.h"

enum class eMeasureType
{
	MeasureTemperature,
	MeasureMaxTemperature,
	MeasureTjMax,
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

int TjMax = 0;
int temp = 0;
int cache = 0;
HANDLE hMutex = nullptr;
Driver* pDriver = nullptr;
HANDLE gHandle = nullptr;

PLUGIN_EXPORT void Initialize(void** data, void* rm);
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue);
PLUGIN_EXPORT double Update(void* data);
PLUGIN_EXPORT void Finalize(void* data);

ULONGLONG _GetTickCount64();
DWORD WINAPI Thread(LPVOID data);

DWORD GetCoreCount();
UINT GetTjMax();
UINT GetTemp(BYTE index);
UINT GetHighestTemp();