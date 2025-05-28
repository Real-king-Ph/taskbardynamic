#pragma once

#include "DynamicData.h"
#include "PluginInterface.h"
#include <string>


namespace config {
	// get_data_
	unsigned long long GetUpSpeed(const ITMPlugin::MonitorInfo& monitor_info);
	unsigned long long GetDownSpeed(const ITMPlugin::MonitorInfo& monitor_info);
	int GetCpuTemperature(const ITMPlugin::MonitorInfo& monitor_info);

	// set_data_
	void SetNetSpeed(std::wstring& text, unsigned long long value);
	void SetCpuTemperatrue(std::wstring& text, int value);

	// config
	extern DynamicInfo<unsigned long long> upload_info_;
	extern DynamicInfo<unsigned long long> download_info_;
	extern DynamicInfo<int> cpu_temperature_info_;
};