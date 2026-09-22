#include "pch.h"
#include "config.h"

#include <cstdio>

namespace config {
	namespace {
		constexpr double kKilo = 1024.0;
		constexpr double kMega = kKilo * 1024.0;
		constexpr double kGiga = kMega * 1024.0;

		/// 把数值部分统一格式化成 4 个字符：999 / 99.9 / 9.99
		std::wstring FormatValue(double value) {
			wchar_t buffer[32]{};

			if (value >= 100.0) {
				swprintf_s(buffer, L"%.0f", value);
			}
			else if (value >= 10.0) {
				swprintf_s(buffer, L"%.1f", value);
			}
			else {
				swprintf_s(buffer, L"%.2f", value);
			}

			return buffer;
		}
	}

	// get_data_
	unsigned long long GetUpSpeed(const ITMPlugin::MonitorInfo& monitor_info) {
		return monitor_info.up_speed;
	}
	unsigned long long GetDownSpeed(const ITMPlugin::MonitorInfo& monitor_info) {
		return monitor_info.down_speed;
	}
	int GetCpuTemperature(const ITMPlugin::MonitorInfo& monitor_info) {
		return monitor_info.cpu_temperature;
	}

	// set_data_
	void SetNetSpeed(std::wstring& text, unsigned long long value) {
		const double bytes = static_cast<double>(value);

		// 按数量级自动选择单位，避免大流量时数值部分被截断成无意义的字符串
		double scaled = bytes;
		const wchar_t* unit = L"B/s";

		if (bytes >= kGiga) {
			scaled = bytes / kGiga;
			unit = L"GB/s";
		}
		else if (bytes >= kMega) {
			scaled = bytes / kMega;
			unit = L"MB/s";
		}
		else if (bytes >= kKilo) {
			scaled = bytes / kKilo;
			unit = L"KB/s";
		}

		text = FormatValue(scaled) + unit;
	}

	void SetCpuTemperature(std::wstring& text, int value) {
		text = std::to_wstring(value) + L"°C";
	}

	// config
	DynamicInfo<unsigned long long> upload_info_ = {
		L"upload speed",
		L"mq72ZrBHN2",  // 显示项 ID，修改后主程序会认为是新的显示项
		L"↑:",
		L"12.1MB/s",
		1,
		GetUpSpeed,
		SetNetSpeed
	};

	DynamicInfo<unsigned long long> download_info_ = {
		L"download speed",
		L"DE7etVv1nR",
		L"↓:",
		L"12.1MB/s",
		1,
		GetDownSpeed,
		SetNetSpeed
	};

	DynamicInfo<int> cpu_temperature_info_ = {
		L"cpu temperature",
		L"ST5sLiDY3f",
		L"CPU:",
		L"17°C",
		1,
		GetCpuTemperature,
		SetCpuTemperature
	};
};