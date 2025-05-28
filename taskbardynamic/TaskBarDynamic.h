#pragma once
#include "PluginInterface.h"
#include "DynamicData.h"
#include <vector>

// 上传/下载

class DynamicWindow : public ITMPlugin {
public:
	static DynamicWindow& GetInstance();
	virtual IPluginItem* GetItem(int index) override;

	virtual const wchar_t* GetInfo(PluginInfoIndex index) override;

	// 动态生成数据
	virtual void DataRequired() override;
	virtual void OnMonitorInfo(const MonitorInfo& monitor_info);

private:
	DynamicWindow();

	DynamicData<unsigned long long> upload_speed_;
	DynamicData<unsigned long long> download_speed_;
	DynamicData<int>cpu_temperature_;
};

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
