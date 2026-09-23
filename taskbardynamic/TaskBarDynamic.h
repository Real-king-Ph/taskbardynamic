#pragma once
#include "PluginInterface.h"
#include "DynamicData.h"
#include "PrimoCache.h"
#include <memory>
#include <vector>

// 任务栏动态显示插件：把主程序采集到的监控数据按滑动窗口归一化后显示在任务栏上

class DynamicWindow : public ITMPlugin {
public:
	static DynamicWindow& GetInstance();

	DynamicWindow(const DynamicWindow&) = delete;
	DynamicWindow& operator=(const DynamicWindow&) = delete;

	IPluginItem* GetItem(int index) override;
	const wchar_t* GetInfo(PluginInfoIndex index) override;

	// 主程序定时回调：推进所有显示项的滑动窗口并刷新显示文本
	void DataRequired() override;
	// 主程序推送监控数据：交给各显示项缓存
	void OnMonitorInfo(const MonitorInfo& monitor_info) override;

private:
	DynamicWindow();

	/// 按配置创建显示项，新增显示项时只需在 config 中追加一条
	template <typename T>
	void AddItem(const DynamicInfo<T>& info) {
		items_.emplace_back(std::make_unique<DynamicData<T>>(info));
	}

	/// PrimoCache 显示项：在被绘制时上报，用于惰性采样
	template <typename T>
	void AddPrimoItem(const DynamicInfo<T>& info) {
		items_.emplace_back(std::make_unique<PrimoCacheItem<T>>(info));
	}

	std::vector<std::unique_ptr<IPluginItem>> items_;
};

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif