#include "pch.h"
#include "TaskBarDynamic.h"
#include "config.h"
#include "PrimoCache.h"

DynamicWindow& DynamicWindow::GetInstance()
{
	static DynamicWindow instance;
	return instance;
}

DynamicWindow::DynamicWindow()
{
	// 显示项的顺序即 GetItem(index) 的顺序，新增显示项只需在这里追加一行
	AddItem(config::upload_info_);
	AddItem(config::download_info_);
	AddItem(config::cpu_temperature_info_);

	// PrimoCache 显示项只在“已安装 PrimoCache + 具备管理员权限 + 能取到数据”时注册；
	// 任一条件不满足时这些项不会出现在主程序的显示项列表里（等于不生效）。
	primoAvailable_ = PrimoCacheMonitor::Instance().Probe();
	if (primoAvailable_) {
		AddPrimoItem(config::primo_hit_speed_info_);
		AddPrimoItem(config::primo_miss_speed_info_);
		AddPrimoItem(config::primo_hit_rate_info_);
	}

	RefreshPrimoTooltip();
}

ITMPlugin* TMPluginGetInstance() {
	return &DynamicWindow::GetInstance();
}

const wchar_t* DynamicWindow::GetInfo(PluginInfoIndex index) {
	switch (index) {
	case TMI_NAME:
		return L"任务栏滚动图自适应";
	case TMI_DESCRIPTION:
		return L"在任务栏上显示实时上传/下载速度、CPU 温度与 PrimoCache 命中/未命中速度及命中率，并按窗口内的历史极值绘制资源占用图";
	case TMI_AUTHOR:
		return L"Real-King-ph";
	case TMI_COPYRIGHT:
		return L"Copyright (C) 2025-2026 Real-King-ph";
	case TMI_VERSION:
		return L"v1.2.2";
	case TMI_URL:
		return L"https://github.com/Real-king-Ph/taskbardynamic";
	default:
		break;  // TMI_MAX 是枚举哨兵，主程序不会查询
	}
	return L"";
}

const wchar_t* DynamicWindow::GetTooltipInfo() {
	PrimoCacheMonitor::Instance().NotifyQueried();
	return primoTooltipText_.c_str();
}

void DynamicWindow::DataRequired() {
	for (const auto& item : items_) {
		item->OnItemInfo(IPluginItem::SET_ITEM_DATA, nullptr, nullptr);
	}

	RefreshPrimoTooltip();
}

void DynamicWindow::RefreshPrimoTooltip() {
	if (!primoAvailable_) {
		std::wstring reason = PrimoCacheMonitor::Instance().Reason();
		if (reason.empty()) {
			reason = L"PrimoCache 不可用";
		}
		primoTooltipText_ = L"PrimoCache\n状态      不可用\n原因      " + reason;
		return;
	}

	const PrimoCacheSnapshot snapshot = PrimoCacheMonitor::Instance().Snapshot();
	if (!snapshot.valid) {
		primoTooltipText_ = L"PrimoCache\n状态      采样失败";
		return;
	}

	if (!snapshot.rate_valid) {
		primoTooltipText_ = L"PrimoCache\n状态      等待采样";
		return;
	}

	std::wstring text = L"PrimoCache\n总读取    " + config::FormatSpeed(snapshot.read_speed) + L"\n";
	if (snapshot.hit_rate_valid) {
		const double miss_rate = 100.0 - snapshot.hit_rate_percent;
		text += L"未中率    " + config::FormatPercent(miss_rate) + L"\n状态      正常";
	}
	else {
		text += L"未中率    --\n状态      正常（近期无读取）";
	}

	primoTooltipText_ = text;
}

void DynamicWindow::OnMonitorInfo(const MonitorInfo& monitor_info) {
	for (const auto& item : items_) {
		// 接口以 void* 传递参数，DynamicData 内部按 const 处理，不会修改主程序的数据
		item->OnItemInfo(IPluginItem::GET_ITEM_DATA, const_cast<MonitorInfo*>(&monitor_info), nullptr);
	}
}

IPluginItem* DynamicWindow::GetItem(int index) {
	if (index >= 0 && index < static_cast<int>(items_.size())) {
		return items_[index].get();
	}

	return nullptr;
}
