#include "pch.h"
#include "TaskBarDynamic.h"
#include "config.h"

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
}

ITMPlugin* TMPluginGetInstance() {
	return &DynamicWindow::GetInstance();
}

const wchar_t* DynamicWindow::GetInfo(PluginInfoIndex index) {
	switch (index) {
	case TMI_NAME:
		return L"任务栏滚动图自适应";
	case TMI_DESCRIPTION:
		return L"在任务栏上显示实时上传/下载速度与 CPU 温度，并按窗口内的历史极值自适应绘制资源占用图";
	case TMI_AUTHOR:
		return L"Real-King-ph";
	case TMI_COPYRIGHT:
		return L"cpy";
	case TMI_VERSION:
		return L"v1.1";
	case TMI_URL:
		return L"https://github.com/Real-king-Ph/taskbardynamic";
	default:
		break;  // TMI_MAX 是枚举哨兵，主程序不会查询
	}
	return L"";
}

void DynamicWindow::DataRequired() {
	SYSTEMTIME system_time{};
	GetLocalTime(&system_time);

	for (const auto& item : items_) {
		item->OnItemInfo(IPluginItem::SET_ITEM_DATA, &system_time, nullptr);
	}
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