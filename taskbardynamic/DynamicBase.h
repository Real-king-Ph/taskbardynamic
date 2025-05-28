#pragma once
#include <vector>


template <typename T>
class DynamicBase
{
public:
	DynamicBase();

	void SetValue(const T& value);
	void PutInValue(const SYSTEMTIME& time);
	float GenerateValue() const;
	T GetValue() const { return value_; }

private:
	// 大小为4的动态数组
	// 分别为 1h 30min 5min curr
	int size_;
	std::vector<std::pair<T, SYSTEMTIME>> min_;
	std::vector<std::pair<T, SYSTEMTIME>> max_;
	int deprecated_time_[4] = { 10 , 5 , 2, 1 };
	T value_;

	bool SetDeprecated(int index, const SYSTEMTIME& time);
	bool SetRightValue(int index);
};

