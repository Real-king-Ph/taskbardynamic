#include "pch.h"
#include "DynamicData.h"

template<typename T>
DynamicData<T>::DynamicData(const DynamicInfo<T>& info) : info_(info) {}

template<typename T>
const wchar_t* DynamicData<T>::GetItemName() const {
	return info_.name_.c_str();
}

template<typename T>
const wchar_t* DynamicData<T>::GetItemId() const {
	return info_.id_.c_str();
}

template<typename T>
const wchar_t* DynamicData<T>::GetItemLableText() const {
	return info_.lable_.c_str();
}

template<typename T>
const wchar_t* DynamicData<T>::GetItemValueSampleText() const {
	return info_.value_sample_.c_str();
}

template<typename T>
const wchar_t* DynamicData<T>::GetItemValueText() const {
	return value_text_.c_str();
}

template<typename T>
int DynamicData<T>::IsDrawResourceUsageGraph() const {
	return info_.draw_graph_;
}

template<typename T>
float DynamicData<T>::GetResourceUsageGraphValue() const {
	return data_.GenerateValue();
}

template<typename T>
void DynamicData<T>::SetData(const ITMPlugin::MonitorInfo& monitor_info) {
	T value = info_.get_data_(monitor_info);
	data_.SetValue(value);
}

template<typename T>
void DynamicData<T>::GenerateData(const SYSTEMTIME& time) {
	data_.PutInValue(time);
	info_.set_data_(value_text_, data_.GetValue());
}

template<typename T>
void* DynamicData<T>::OnItemInfo(ItemInfoType info, void* para1, void* para2) {
	switch (info)
	{
	case IPluginItem::GET_ITEM_DATA:
		if (para1 != nullptr)
		{
			ITMPlugin::MonitorInfo* monitor_info = static_cast<ITMPlugin::MonitorInfo*>(para1);
			SetData(*monitor_info);
		}
		break;
	case IPluginItem::SET_ITEM_DATA:
		if (para1 != nullptr)
		{
			SYSTEMTIME* time = static_cast<SYSTEMTIME*>(para1);
			GenerateData(*time);
		}
		break;
	default:
		break;
	}
	return 0;
}

template class DynamicData<int>;
template class DynamicData<unsigned long long>;
template class DynamicData<float>;