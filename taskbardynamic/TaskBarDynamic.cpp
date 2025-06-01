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
	using ull = unsigned long long;
	using DDull = DynamicData<ull>;
	using DDint = DynamicData<int>;

	items_.emplace_back(std::make_unique<DDull>(config::upload_info_));
	items_.emplace_back(std::make_unique<DDull>(config::download_info_));
	items_.emplace_back(std::make_unique<DDint>(config::cpu_temperature_info_));
}

ITMPlugin* TMPluginGetInstance() {
	return &DynamicWindow::GetInstance();
}

const wchar_t* DynamicWindow::GetInfo(PluginInfoIndex index) {
	switch (index) {
	case TMI_NAME:
		return L"任务栏滚动图自适应";
	case TMI_DESCRIPTION:
		return L"任务栏滚动图自适应";
	case TMI_AUTHOR:
		return L"Real-King-ph";
	case TMI_COPYRIGHT:
		return L"cpy";
	case TMI_VERSION:
		return L"v1.0";
	case TMI_URL:
		return L"https://github.com/Real-king-Ph/taskbardynamic";
	case TMI_MAX:
		return L"3";
	default:
		break;
	}
	return L"";
}

void DynamicWindow::DataRequired() {
	static SYSTEMTIME system_time;
	GetLocalTime(&system_time);

	for (auto& item : items_) {
		item->OnItemInfo(IPluginItem::SET_ITEM_DATA, &system_time, nullptr);
	}
}

void DynamicWindow::OnMonitorInfo(const MonitorInfo& monitor_info) {
	for (auto& item : items_) {
		item->OnItemInfo(IPluginItem::GET_ITEM_DATA, const_cast<MonitorInfo*>(&monitor_info), nullptr);
	}
}



IPluginItem* DynamicWindow::GetItem(int index) {
	if (index >= 0 && index < items_.size()) {
		return items_[index].get();
	}

	return nullptr;
}