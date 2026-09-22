#pragma once
#include "pch.h"
#include "PluginInterface.h"
#include "DynamicBase.h"
#include <functional>
#include <memory>
#include <string>

/// 单个显示项的静态描述与数据回调
template<typename T>
struct DynamicInfo {
	std::wstring name_;          ///< 显示项名称
	std::wstring id_;            ///< 显示项唯一 ID，同一插件内不可重复
	std::wstring lable_;         ///< 标签文本，如 "↑:"
	std::wstring value_sample_;  ///< 数值示例文本，主程序据此估算显示宽度

	int draw_graph_{ 0 };        ///< 是否绘制资源占用图：1 绘制，0 不绘制

	std::function<T(const ITMPlugin::MonitorInfo&)> get_data_;   ///< 从监控信息中提取原始数据
	std::function<void(std::wstring& text, T value)> set_data_;  ///< 把原始数据格式化成显示文本
};

/**
 * @brief 基于 DynamicInfo 的通用 IPluginItem 实现
 *
 * 数据流：
 *   OnMonitorInfo() -> OnItemInfo(GET_ITEM_DATA) -> SetData()      缓存原始值
 *   DataRequired()  -> OnItemInfo(SET_ITEM_DATA) -> GenerateData() 推进窗口并刷新显示文本
 */
template<typename T>
class DynamicData : public IPluginItem
{
public:
	DynamicData() = default;
	explicit DynamicData(const DynamicInfo<T>& info);

	// 固定返回的静态信息
	const wchar_t* GetItemName() const override;
	const wchar_t* GetItemId() const override;
	const wchar_t* GetItemLableText() const override;
	const wchar_t* GetItemValueSampleText() const override;
	int IsDrawResourceUsageGraph() const override;

	// 动态更新的数据
	const wchar_t* GetItemValueText() const override;
	float GetResourceUsageGraphValue() const override;

	void* OnItemInfo(ItemInfoType info, void* para1, void* para2) override;

private:
	/// 缓存主程序推送的最新监控数据
	void SetData(const ITMPlugin::MonitorInfo& monitor_info);
	/// 推进滑动窗口并刷新显示文本
	void GenerateData(const SYSTEMTIME& time);

	DynamicBase<T> data_;
	DynamicInfo<T> info_;
	std::wstring value_text_;
};