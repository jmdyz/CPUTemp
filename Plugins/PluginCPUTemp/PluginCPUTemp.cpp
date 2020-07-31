/* Copyright (C) 2011 Rainmeter Project Developers
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License; either version 2 of the License, or (at your option) any later
 * version. If a copy of the GPL was not distributed with this file, You can
 * obtain one at <https://www.gnu.org/licenses/gpl-2.0.html>. */

#include <windows.h>
#include <wchar.h>
#include "CPUTempProxy.h"
#include "../../Library/Export.h"	// Rainmeter's exported functions

enum eMeasureType
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
		type(MeasureTemperature),
		index(0),
		rm(nullptr) {}
};



eMeasureType convertStringToMeasureType(LPCWSTR i_String, void* rm);
bool areStringsEqual(LPCWSTR i_String1, LPCWSTR i_Strting2);
int getHighestTemp(void* rm);

CoreTempProxy proxy;

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	MeasureData* measure = new MeasureData;
	*data = measure;

	measure->rm = rm;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	UNREFERENCED_PARAMETER(maxValue);
	MeasureData* measure = (MeasureData*)data;

	LPCWSTR value = RmReadString(rm, L"CoreTempType", L"Temperature");
	measure->type = convertStringToMeasureType(value, rm);

	measure->index = RmReadInt(rm, L"CoreTempIndex", 1);

	//RmLogF(rm, LOG_DEBUG, L"Reload:%i", measure->type);
}

PLUGIN_EXPORT double Update(void* data)
{
	
	MeasureData* measure = (MeasureData*)data;
	double result = 0;

	//RmLogF(measure->rm, LOG_DEBUG, L"Update");

	//if (proxy.GetData())
	{
		//RmLogF(measure->rm, LOG_DEBUG, L"GetData");

		switch (measure->type)
		{
		case MeasureTemperature:
			proxy._GetTemp(measure->index);
			result = proxy.GetTemp(measure->index);
			break;

		case MeasureMaxTemperature:
			result = getHighestTemp(measure->rm);
			//RmLogF(measure->rm, LOG_DEBUG, L"MeasureMaxTemperature:%i", result);
			break;

		case MeasureTjMax:
			result = proxy.GetTjMax();
			break;

		case MeasureLoad:
			result = proxy.GetCoreLoad(measure->index);
			break;

		case MeasureVid:
			result = proxy.GetVID();
			break;

		case MeasureCpuSpeed:
			result = proxy.GetCPUSpeed();
			break;

		case MeasureBusSpeed:
			result = proxy.GetFSBSpeed();
			break;

		case MeasureBusMultiplier:
			result = proxy.GetMultiplier();
			break;

		case MeasureCoreSpeed:
			result = (double)proxy.GetMultiplier(measure->index) * proxy.GetFSBSpeed();
			break;

		case MeasureCoreBusMultiplier:
			result = proxy.GetMultiplier(measure->index);
			break;

		case MeasureTdp:
			result = proxy.GetTdp(measure->index);
			break;

		case MeasurePower:
			result = proxy.GetPower(measure->index);
			break;
		}
	}

	return result;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	MeasureData* measure = (MeasureData*)data;
	static WCHAR buffer[128];

	switch (measure->type)
	{
	case MeasureVid:
		_snwprintf_s(buffer, _TRUNCATE, L"%.4f", proxy.GetVID());
		break;

	case MeasureCpuName:
		_snwprintf_s(buffer, _TRUNCATE, L"%S", proxy.GetCPUName());
		break;

	default:
		return nullptr;
	}

	return buffer;
}

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
		result = MeasureTemperature;
	}
	else if (areStringsEqual(i_String, L"MaxTemperature"))
	{
		result = MeasureMaxTemperature;
	}
	else if (areStringsEqual(i_String, L"TjMax"))
	{
		result = MeasureTjMax;
	}
	else if (areStringsEqual(i_String, L"Load"))
	{
		result = MeasureLoad;
	}
	else if (areStringsEqual(i_String, L"Vid"))
	{
		result = MeasureVid;
	}
	else if (areStringsEqual(i_String, L"CpuSpeed"))
	{
		result = MeasureCpuSpeed;
	}
	else if (areStringsEqual(i_String, L"BusSpeed"))
	{
		result = MeasureBusSpeed;
	}
	else if (areStringsEqual(i_String, L"BusMultiplier"))
	{
		result = MeasureBusMultiplier;
	}
	else if (areStringsEqual(i_String, L"CpuName"))
	{
		result = MeasureCpuName;
	}
	else if (areStringsEqual(i_String, L"CoreSpeed"))
	{
		result = MeasureCoreSpeed;
	}
	else if (areStringsEqual(i_String, L"CoreBusMultiplier"))
	{
		result = MeasureCoreBusMultiplier;
	}
	else if (areStringsEqual(i_String, L"Tdp"))
	{
		result = MeasureTdp;
	}
	else if (areStringsEqual(i_String, L"Power"))
	{
		result = MeasurePower;
	}
	else
	{
		result = MeasureTemperature;
		RmLogF(rm, LOG_WARNING, L"Invalid CoreTempType: %s", i_String);
	}

	return result;
}



int getHighestTemp(void* rm)
{
	//RmLogF(rm, LOG_DEBUG, L"getHighestTemp");

	int temp = -255;
	UINT coreCount = proxy.GetCoreCount();

	//RmLogF(rm, LOG_DEBUG, L"��������%i", coreCount);

	for (UINT i = 1; i <= coreCount; ++i)
	{
		//RmLogF(rm, LOG_DEBUG, L"����#%i", i);
		proxy._GetTemp(i);
		const int getTemp = proxy.GetTemp(i);
		if (temp < getTemp)
		{
			temp = getTemp;
		}
	}

	return temp;
}
