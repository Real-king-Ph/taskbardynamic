#include "pch.h"
#include "DynamicBase.h"

#include <cassert>

template <typename T>
typename DynamicBase<T>::Stamp DynamicBase<T>::ToStamp(const SYSTEMTIME& time) noexcept
{
	FILETIME file_time{};
	if (SystemTimeToFileTime(&time, &file_time) == FALSE) {
		return Stamp{};
	}

	ULARGE_INTEGER ticks{};
	ticks.LowPart = file_time.dwLowDateTime;
	ticks.HighPart = file_time.dwHighDateTime;

	// FILETIME 本身就是以 100ns 为单位的绝对时间戳，无需再做换算
	return Stamp{ static_cast<std::int64_t>(ticks.QuadPart) };
}

template <typename T>
std::chrono::minutes DynamicBase<T>::ElapsedMinutes(Stamp from, Stamp to) noexcept
{
	if (to <= from) {
		return std::chrono::minutes::zero();  // 系统时间被回拨时按 0 处理
	}
	return std::chrono::duration_cast<std::chrono::minutes>(to - from);
}

template <typename T>
void DynamicBase<T>::PutInValue(const SYSTEMTIME& time)
{
	const Stamp now = ToStamp(time);
	constexpr std::size_t current = kWindowCount - 1;

	if (!initialized_) {
		// 首次采样时各档从同一点开始，避免把默认构造的 0 时刻当成历史极值
		min_.fill(Sample{ value_, now });
		max_.fill(Sample{ value_, now });
		initialized_ = true;
		return;
	}

	// 当前值始终占据最后一档
	min_[current] = Sample{ value_, now };
	max_[current] = Sample{ value_, now };

	// 由短窗口到长窗口淘汰超时极值：
	// 必须等短窗口刷新完再处理长窗口，否则长窗口会取到短窗口尚未刷新的旧极值，
	// 导致过期的极值在窗口中多残留若干个采样周期。
	for (int i = static_cast<int>(kWindowCount) - 2; i >= 0; --i) {
		Decay(static_cast<std::size_t>(i), now);
	}

	// 再由短窗口反向收缩长窗口，维持窗口之间的单调性 min_[i] <= min_[i+1]、max_[i] >= max_[i+1]
	for (int i = static_cast<int>(kWindowCount) - 2; i >= 0; --i) {
		ShrinkTo(static_cast<std::size_t>(i));
	}
}

template <typename T>
void DynamicBase<T>::Decay(std::size_t index, Stamp now) noexcept
{
	assert(index + 1 < kWindowCount);

	const std::chrono::minutes limit{ kWindowMinutes[index] };

	if (ElapsedMinutes(min_[index].second, now) > limit) {
		min_[index] = min_[index + 1];
	}
	if (ElapsedMinutes(max_[index].second, now) > limit) {
		max_[index] = max_[index + 1];
	}
}

template <typename T>
void DynamicBase<T>::ShrinkTo(std::size_t index) noexcept
{
	const Sample& shorter_min = min_[index + 1];
	const Sample& shorter_max = max_[index + 1];

	if (min_[index].first > shorter_min.first) {
		min_[index] = shorter_min;
	}
	if (max_[index].first < shorter_max.first) {
		max_[index] = shorter_max;
	}
}

template <typename T>
float DynamicBase<T>::GenerateValue() const noexcept
{
	const double min = static_cast<double>(min_[0].first);
	const double max = static_cast<double>(max_[0].first);

	// 窗口内数值恒定（max == min）时 0/0 会得到 NaN，会使主程序绘图异常
	if (!(max > min)) {
		return 0.0f;
	}

	const double ratio = (static_cast<double>(value_) - min) / (max - min);
	return static_cast<float>(std::clamp(ratio, 0.0, 1.0));
}

template class DynamicBase<int>;
template class DynamicBase<unsigned long long>;
template class DynamicBase<float>;