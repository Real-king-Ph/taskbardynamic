#pragma once
#include "pch.h"
#include "PluginInterface.h"
#include "DynamicBase.h"
#include <memory>
#include <string>
#include <functional>


template<typename T>
struct DynamicInfo {
	std::wstring name_;
	std::wstring id_;
	std::wstring lable_;
	std::wstring value_sample_;

	int draw_graph_;

	std::function<T(const ITMPlugin::MonitorInfo&)> get_data_;
	std::function<void(std::wstring& text, T value)> set_data_;
};

template<typename T>
class DynamicData : public IPluginItem
{
public:
	DynamicData() {}
	DynamicData(const DynamicInfo<T>& info);

	// 固定回答的信息
	virtual const wchar_t* GetItemName() const override;
	virtual const wchar_t* GetItemId() const override;
	virtual const wchar_t* GetItemLableText() const override;
	virtual const wchar_t* GetItemValueSampleText() const override;
	virtual int IsDrawResourceUsageGraph() const override;

	// 动态更新的数据
	virtual const wchar_t* GetItemValueText() const override;
	virtual float GetResourceUsageGraphValue() const override;

	// 设置数据
	void SetData(const ITMPlugin::MonitorInfo& monitor_info);
	void GenerateData(const SYSTEMTIME& time);

private:
	DynamicBase<T> data_;
	DynamicInfo<T> info_;
	std::wstring value_text_;
};

