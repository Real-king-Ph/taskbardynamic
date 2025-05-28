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
	// ��СΪ4�Ķ�̬����
	// �ֱ�Ϊ 10min 5min 2min curr
	int size_;
	std::vector<std::pair<T, SYSTEMTIME>> min_;
	std::vector<std::pair<T, SYSTEMTIME>> max_;
	int deprecated_time_[4] = { 10 , 5 , 2, 1 };
	T value_{0};

	bool SetDeprecated(int index, const SYSTEMTIME& time);
	bool SetRightValue(int index);
};

