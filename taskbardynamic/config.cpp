#include "pch.h"
#include "config.h"

namespace config {
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
		std::wstring s;
		std::wstring unit;

		if (value < 1024 * 1000) { // 使用 KB/s 作为单位
			unsigned long long integerPart = value / 1024;
			unsigned long long remainder = value % 1024;
			unsigned long long fractionalPart = (remainder * 100) / 1024; // 计算小数点后两位

			unit = L"KB/s";

			// 格式化输出为固定宽度4
			s = std::to_wstring(integerPart) + L".";
			if (fractionalPart < 10)
				s += L"0"; // 确保两位小数
			s += std::to_wstring(fractionalPart);

			// 截断或补零到4个字符
			if (s.length() > 4)
				s = s.substr(0, 4);
			else
				while (s.length() < 4)
					s += L"0";
		}
		else {
			unsigned long long integerPart = value / (1024 * 1024);
			unsigned long long remainder = value % (1024 * 1024);
			unsigned long long fractionalPart = (remainder * 100) / (1024 * 1024); // 计算小数点后两位

			unit = L"MB/s";

			// 格式化输出为固定宽度4
			s = std::to_wstring(integerPart) + L".";
			if (fractionalPart < 10)
				s += L"0"; // 确保两位小数
			s += std::to_wstring(fractionalPart);

			// 截断或补零到4个字符
			if (s.length() > 4)
				s = s.substr(0, 4);
			else
				while (s.length() < 4)
					s += L"0";
		}

		text = s + unit;
	}
	void SetCpuTemperatrue(std::wstring& text, int value) {
		text = std::to_wstring(value) + L"°C";
	}

	// config
	DynamicInfo<unsigned long long> upload_info_ = {
		L"upload speed",
		L"mq72ZrBHN2", // 随机字符串                
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
		SetCpuTemperatrue
	};
};