#include "pch.h"
#include "config.h"

#include "PrimoCache.h"

#include <atomic>
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

		// ---- PrimoCache 取值状态 ----
		// 由 get_data_ 更新、由格式化回调读取，保证同一周期内数值与标签一致
		std::atomic<bool> g_primo_valid{ false };           ///< 最近一次采样是否成功
		std::atomic<bool> g_primo_hit_rate_valid{ false };  ///< 区间内有读取时命中率才有效

		/// 取一次快照并同步状态标记
		PrimoCacheSnapshot TakePrimoSnapshot() {
			const PrimoCacheSnapshot snapshot = PrimoCacheMonitor::Instance().Snapshot();
			g_primo_valid.store(snapshot.valid);
			g_primo_hit_rate_valid.store(snapshot.hit_rate_valid);
			return snapshot;
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

	// ---- PrimoCache 取数 ----

	unsigned long long GetPrimoHitSpeed(const ITMPlugin::MonitorInfo&) {
		return TakePrimoSnapshot().hit_speed;
	}

	unsigned long long GetPrimoMissSpeed(const ITMPlugin::MonitorInfo&) {
		return TakePrimoSnapshot().miss_speed;
	}


	float GetPrimoHitRate(const ITMPlugin::MonitorInfo&) {
		return static_cast<float>(TakePrimoSnapshot().hit_rate_percent);
	}

	// ---- PrimoCache 文本格式化 ----
	void SetPrimoSpeed(std::wstring& text, unsigned long long value) {
		if (!g_primo_valid.load()) {
			text = L"--";           // 连续多次采样失败（服务停止 / 取数异常）
			return;
		}
		SetNetSpeed(text, value);   // 复用网速的自动单位格式化
	}


	void SetPrimoHitRate(std::wstring& text, float value) {
		if (!g_primo_valid.load()) {
			text = L"--";           // 连续多次采样失败
			return;
		}
		if (!g_primo_hit_rate_valid.load()) {
			text = L"--";           // 本区间没有读取，命中率无意义
			return;
		}

		wchar_t buffer[32]{};
		swprintf_s(buffer, L"%.1f%%", static_cast<double>(value));
		text = buffer;
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

	// ---- PrimoCache 显示项 ----
	// 取数来自 rxpcc 的后台采样（5 秒一次），口径均为“缓存卷收到的 I/O”

	DynamicInfo<unsigned long long> primo_hit_speed_info_ = {
		L"primocache hit read speed",   // 命中速度（由缓存 L1/L2 服务）
		L"PC_HIT_SPD",
		L"命中:",
		L"12.1MB/s",
		1,
		GetPrimoHitSpeed,
		SetPrimoSpeed
	};

	DynamicInfo<unsigned long long> primo_miss_speed_info_ = {
		L"primocache miss read speed",  // 未命中速度（落回物理磁盘）
		L"PC_MISS_SPD",
		L"未中:",

		L"345KB/s",
		1,
		GetPrimoMissSpeed,
		SetPrimoSpeed
	};


	DynamicInfo<float> primo_hit_rate_info_ = {
		L"primocache hit rate",         // 实时命中率（最近一个采样区间）
		L"PC_HIT_RATE",
		L"命中率:",
		L"99.9%",
		1,
		GetPrimoHitRate,
		SetPrimoHitRate
	};
};