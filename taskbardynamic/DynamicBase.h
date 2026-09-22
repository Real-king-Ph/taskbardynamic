#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <ratio>
#include <utility>
#include <windows.h>

/**
 * @brief 滑动窗口极值统计
 *
 * 维护 4 档时间窗口内的极小值 / 极大值，索引由长到短：
 *   0 -> 10min    1 -> 5min    2 -> 2min    3 -> 当前采样值
 *
 * 每一档同时记录“取得该极值的时刻”：
 *   - Decay()    极值超时后向更短的窗口回退；
 *   - ShrinkTo() 再由短窗口反向收缩长窗口，保证
 *                min_[i] <= min_[i+1] 且 max_[i] >= max_[i+1]。
 *
 * GenerateValue() 使用最长窗口把当前值归一化到 0.0 ~ 1.0，供任务栏资源占用图使用。
 *
 * @tparam T 采样值类型，需支持大小比较与默认构造（本项目使用 int / unsigned long long / float）
 */
template <typename T>
class DynamicBase
{
public:
	/// 与 FILETIME 相同的时间刻度（100ns），仅用于计算采样间隔
	using Stamp = std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>;

	/// 窗口档数：10min / 5min / 2min / 当前值
	static constexpr std::size_t kWindowCount = 4;
	/// 各档的超时分钟数，最后一档为当前值不参与淘汰
	static constexpr std::array<int, kWindowCount - 1> kWindowMinutes{ 10, 5, 2 };

	DynamicBase() = default;

	/// 记录本次采样值（不推进窗口，需再调用 PutInValue）
	void SetValue(const T& value) noexcept { value_ = value; }
	/// 获取最近一次采样值
	T GetValue() const noexcept { return value_; }

	/// 以采样时刻推进一次滑动窗口
	void PutInValue(const SYSTEMTIME& time);

	/// 当前值在最长窗口中的归一化位置，范围 0.0 ~ 1.0
	float GenerateValue() const noexcept;

private:
	using Sample = std::pair<T, Stamp>;

	/// SYSTEMTIME -> FILETIME 刻度，转换失败时返回 0
	static Stamp ToStamp(const SYSTEMTIME& time) noexcept;
	/// to - from 的分钟数，系统时间被回拨时返回 0
	static std::chrono::minutes ElapsedMinutes(Stamp from, Stamp to) noexcept;
	/// 极值超时后向更短的窗口回退
	void Decay(std::size_t index, Stamp now) noexcept;
	/// 用更短窗口的极值收缩当前窗口
	void ShrinkTo(std::size_t index) noexcept;

	std::array<Sample, kWindowCount> min_{};
	std::array<Sample, kWindowCount> max_{};
	T value_{};
	bool initialized_{ false };  ///< 首次采样要把各档一起初始化，否则会残留 0 时刻的伪极值
};