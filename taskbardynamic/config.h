#pragma once

#include "DynamicData.h"
#include "PluginInterface.h"
#include <string>

namespace config {
	// get_data_：从主程序的监控信息中取出原始数据
	unsigned long long GetUpSpeed(const ITMPlugin::MonitorInfo& monitor_info);
	unsigned long long GetDownSpeed(const ITMPlugin::MonitorInfo& monitor_info);
	int GetCpuTemperature(const ITMPlugin::MonitorInfo& monitor_info);

	// set_data_：把原始数据格式化成任务栏上显示的文本
	void SetNetSpeed(std::wstring& text, unsigned long long value);
	void SetCpuTemperature(std::wstring& text, int value);

	// 显示项配置：新增一个显示项只需在下面追加一条 DynamicInfo
	extern DynamicInfo<unsigned long long> upload_info_;
	extern DynamicInfo<unsigned long long> download_info_;
	extern DynamicInfo<int> cpu_temperature_info_;
};