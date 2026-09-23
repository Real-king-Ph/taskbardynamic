#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "DynamicData.h"

/// PrimoCache 采样参数
constexpr int kSampleIntervalMs = 5000;     ///< 采样间隔（毫秒）：单次 rxpcc 调用实测约 44ms 墙钟 / 16ms CPU
constexpr int kHitRateWindowMs = 30000;     ///< 命中率统计窗口（毫秒）：取窗口内的累计比值，避免单次 I/O 导致 0%/100% 跳变
constexpr std::size_t kHitRateSamples = static_cast<std::size_t>(kHitRateWindowMs / kSampleIntervalMs);   ///< 窗口内的采样数

/// 一次采样得到的原始累计计数（自 PrimoCache 统计起始时间起算，单位：字节）
struct PrimoCacheCounters {
	std::uint64_t total_read{ 0 };   ///< Total Read      —— 该卷收到的读请求吞吐
	std::uint64_t cached_read{ 0 };  ///< Cached Read     —— 由缓存(L1+L2)服务的读取
};

/**
 * @brief PrimoCache 数据源抽象（方案 C：接口先定，实现可替换）
 *
 * 现有实现 RxpccSource 通过命令行工具 rxpcc.exe 取数。
 * 若将来改为直接与驱动通信（\\.\FancyCcV + IOCTL），只需再实现一个 IPrimoCacheSource，
 * 上层的采样、速率计算与显示逻辑都不用改。
 */
class IPrimoCacheSource
{
public:
	virtual ~IPrimoCacheSource() = default;

	/// 数据源是否可用（已安装 PrimoCache、权限足够、能找到 rxpcc.exe 等）
	virtual bool Available() const = 0;

	/// Available() 为 false 时的原因说明
	virtual const std::wstring& Reason() const = 0;

	/// 采样一次累计计数；失败返回 false
	virtual bool Sample(PrimoCacheCounters& counters) = 0;
};

/// 创建基于 rxpcc.exe 的数据源
std::unique_ptr<IPrimoCacheSource> CreatePrimoCacheSource();

/// 采样结果快照（速率单位为字节/秒，命中率为百分比）
struct PrimoCacheSnapshot {
	bool          valid{ false };            ///< 最近一次采样是否成功
	bool          rate_valid{ false };       ///< 是否已能算出速率（至少完成两次采样）
	std::uint64_t read_speed{ 0 };           ///< 读取速度      Δ(Total Read)/Δt
	std::uint64_t hit_speed{ 0 };            ///< 命中速度      Δ(Cached Read)/Δt
	std::uint64_t miss_speed{ 0 };           ///< 未命中速度    Δ(Total Read - Cached Read)/Δt
	bool          hit_rate_valid{ false };   ///< 区间内有读取时才有意义
	double        hit_rate_percent{ 0.0 };   ///< 实时命中率    Δ(Cached Read)/Δ(Total Read)
};

/**
 * @brief 命中率滚动窗口（方案 A）
 *
 * 命中率取「窗口内累计命中 / 窗口内累计读取」，而不是单个采样区间的比值，
 * 这样在读取量很少的区间也不会出现 0% / 100% 的跳变。
 */
class HitRateWindow
{
public:
	/// 清空窗口（进入空闲、或重新建立基准时调用）
	void Reset() noexcept;

	/// 记录一个采样区间的增量（读取量、命中量）
	void Push(std::uint64_t reads, std::uint64_t hits) noexcept;

	/// 取窗口内的命中率（百分比）；返回 false 表示窗口内没有任何读取
	bool GetPercent(double& percent) const noexcept;

private:
	std::array<std::uint64_t, kHitRateSamples> reads_{};
	std::array<std::uint64_t, kHitRateSamples> hits_{};
	std::size_t pos_{ 0 };
};

/**
 * @brief PrimoCache 采样器（单例）
 *
 * 后台线程按固定间隔调用数据源，UI 线程只读快照，绝不阻塞主程序。
 */
class PrimoCacheMonitor
{
public:
	/// 析构时停止并回收采样线程，避免 std::thread 处于 joinable 状态触发 std::terminate
	~PrimoCacheMonitor();

	static PrimoCacheMonitor& Instance();

	/// 探测环境并启动后台采样；返回 false 时对应显示项不应注册
	bool Probe();

	/// Probe() 失败原因
	const std::wstring& Reason() const { return reason_; }

	/// 采样间隔（毫秒）
	static constexpr int kIntervalMs = kSampleIntervalMs;

	/// 连续失败达到该次数才把显示置为无效（单次失败视为抖动，沿用上一次的值）
	static constexpr int kFailureThreshold = 2;

	/// 超过该时间没有任何显示项被绘制，就停止采样（惰性采样）
	static constexpr int kIdleTimeoutMs = 30000;

	/// 空闲状态的轮询间隔（毫秒）
	static constexpr int kIdlePollMs = 1000;

	/// 取最近一次快照（线程安全）
	PrimoCacheSnapshot Snapshot() const;

	/// 请求后台线程退出（DLL 卸载时调用，尽力而为）
	void RequestStop();

	/// 由显示项在被主程序绘制时上报：表示"当前有用例在显示"，用于惰性采样
	void NotifyQueried();

private:
	PrimoCacheMonitor() = default;
	PrimoCacheMonitor(const PrimoCacheMonitor&) = delete;
	PrimoCacheMonitor& operator=(const PrimoCacheMonitor&) = delete;

	void WorkerMain();
	void WorkerMainImpl();
	/// 分片睡眠，返回 true 表示收到停止请求
	bool SleepInterruptible(int milliseconds);
	/// 记录一次采样失败（连续失败达到阈值时把快照置为无效 → 显示 --）
	void MarkFailure();
	void Publish(const PrimoCacheSnapshot& snapshot);

	std::unique_ptr<IPrimoCacheSource> source_;
	std::wstring reason_;
	PrimoCacheCounters previous_;        ///< 上一次采样（仅后台线程访问）
	bool has_previous_{ false };
	int consecutive_failures_{ 0 };      ///< 连续采样失败次数（仅后台线程访问）
	std::atomic<std::uint64_t> last_query_ms_{ 0 };  ///< 最近一次被主程序查询的时间（惰性采样判据）
	HitRateWindow hit_rate_window_;                  ///< 命中率滚动窗口（仅后台线程访问）
	std::thread worker_;
	std::atomic<bool> stop_{ false };
	mutable std::mutex mutex_;
	PrimoCacheSnapshot snapshot_;
};

/// 进程退出时通知采样线程停止（DllMain 中调用，不阻塞、不等待）
void StopPrimoCacheMonitor();

/**
 * @brief PrimoCache 显示项
 *
 * 主程序只会对**正在显示**的项调用 GetItemValueText() / GetResourceUsageGraphValue()，
 * 因此在这两个接口里上报“被查询”，采样器据此判断是否需要持续采样（惰性采样）。
 */
template <typename T>
class PrimoCacheItem : public DynamicData<T>
{
public:
	explicit PrimoCacheItem(const DynamicInfo<T>& info) : DynamicData<T>(info) {}

	const wchar_t* GetItemValueText() const override
	{
		PrimoCacheMonitor::Instance().NotifyQueried();
		return DynamicData<T>::GetItemValueText();
	}

	float GetResourceUsageGraphValue() const override
	{
		PrimoCacheMonitor::Instance().NotifyQueried();
		return DynamicData<T>::GetResourceUsageGraphValue();
	}
};