#include "pch.h"
#include "TaskBarDynamic.h" 
#include "config.h"

DynamicWindow& DynamicWindow::GetInstance()
{
	static DynamicWindow instance;
	return instance;
}
DynamicWindow::DynamicWindow() :
	upload_speed_(config::upload_info_),
	download_speed_(config::download_info_),
	cpu_temperature_(config::cpu_temperature_info_)
{
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
		return L"https//";
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
	upload_speed_.GenerateData(system_time);
	download_speed_.GenerateData(system_time);
	cpu_temperature_.GenerateData(system_time);
}

void DynamicWindow::OnMonitorInfo(const MonitorInfo& monitor_info) {
	upload_speed_.SetData(monitor_info);
	download_speed_.SetData(monitor_info);
	cpu_temperature_.SetData(monitor_info);
}



IPluginItem* DynamicWindow::GetItem(int index) {
	switch (index) {
	case 0:
		return &upload_speed_;
	case 1:
		return &download_speed_;
	case 2:
		return &cpu_temperature_;
	default:
		break;
	}
	return nullptr;
}