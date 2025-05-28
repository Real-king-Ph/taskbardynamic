#include "pch.h"
#include "DynamicBase.h"

template<typename T>
DynamicBase<T>::DynamicBase() :size_(4) {
	min_.resize(size_);
	max_.resize(size_);
}

template<typename T>
void DynamicBase<T>::SetValue(const T& value) {
	value_ = value;
}

template<typename T>
void DynamicBase<T>::PutInValue(const SYSTEMTIME& time) {
	T value = value_;
	min_[size_ - 1] = { value, time };
	max_[size_ - 1] = { value, time };

	for (int i = 0; i < size_ - 1; ++i) {
		SetDeprecated(i, time);
	}

	for (int i = size_ - 2; i >= 0; i--) {
		SetRightValue(i);
	}
}

template<typename T>
float DynamicBase<T>::GenerateValue() const {
	auto min = min_[0].first;
	auto max = max_[0].first;

	return 1.0 * (value_ - min) / (max - min);
}

template<typename T>
bool DynamicBase<T>::SetDeprecated(int index, const SYSTEMTIME& time) {
	bool f = false;

	auto get_minute = [](const SYSTEMTIME& time) -> int {
		int ret = 0;
		ret += (time.wYear - 2000) * 525600; // 365 days * 24 hours * 60 minutes
		ret += time.wMonth * 43800; // 30 days * 24 hours * 60 minutes
		ret += time.wDay * 1440;   // 24 hours * 60 minutes
		ret += time.wHour * 60;     // 60 minutes
		ret += time.wMinute;        // current minute

		return ret;
		};

	auto min_time = get_minute(min_[index].second);
	auto max_time = get_minute(max_[index].second);
	auto now_time = get_minute(time);

	if (now_time - min_time > deprecated_time_[index]) {
		min_[index] = min_[index + 1];
		f = true;
	}

	if (now_time - max_time > deprecated_time_[index]) {
		max_[index] = max_[index + 1];
		f = true;
	}
	return f;
}

template<typename T>
bool DynamicBase<T>::SetRightValue(int index) {
	bool f = false;

	auto [shoter_min, shoter_min_time] = min_[index + 1];
	auto [shoter_max, shoter_max_time] = max_[index + 1];

	if (min_[index].first > shoter_min || max_[index].first < shoter_max) {
		min_[index] = { shoter_min, shoter_min_time };
		f = true;
	}
	if (max_[index].first < shoter_max || min_[index].first > shoter_min) {
		max_[index] = { shoter_max, shoter_max_time };
		f = true;
	}

	return f;
}

template class DynamicBase<int>;
template class DynamicBase<unsigned long long>;
template class DynamicBase<float>;