#include "pch.h"
#include "PrimoCache.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

namespace {

/// 单次 rxpcc 调用的超时（毫秒）
constexpr DWORD kProcessTimeoutMs = 4000;

/// rxpcc 命令行参数：perf -a -u=b -s
///   perf 取性能统计；-a 全部缓存任务；-u=b 以字节显示（纯整数，便于解析）；-s 静默
constexpr wchar_t kCommandArgs[] = L"perf -a -u=b -s";

/// 采样器是否已启动（供 DllMain 判断是否需要通知停止）
std::atomic<bool> g_monitor_started{ false };

/// 进程是否已提权（rxpcc 需要管理员权限）
bool IsProcessElevated()
{
	HANDLE token = nullptr;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token) == FALSE) {
		return false;
	}

	TOKEN_ELEVATION elevation{};
	DWORD size = 0;
	const BOOL ok = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size);
	CloseHandle(token);
	return ok != FALSE && elevation.TokenIsElevated != 0;
}

bool FileExists(const std::wstring& path)
{
	const DWORD attr = GetFileAttributesW(path.c_str());
	return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::wstring ReadRegString(HKEY root, const wchar_t* subKey, const wchar_t* valueName)
{
	std::wstring result;
	HKEY key = nullptr;
	if (RegOpenKeyExW(root, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) {
		return result;
	}

	wchar_t buffer[1024]{};
	DWORD size = sizeof(buffer);
	DWORD type = 0;
	if (RegQueryValueExW(key, valueName, nullptr, &type, reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS) {
		result.assign(buffer);
	}
	RegCloseKey(key);
	return result;
}

/// PrimoCache 是否已安装（驱动会写入这个注册表键）
bool IsPrimoCacheInstalled()
{
	HKEY key = nullptr;
	const bool exists = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Romex Software\\FancyCcV",
		0, KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS;
	if (exists) {
		RegCloseKey(key);
	}
	return exists;
}

/// 定位安装目录：优先卸载信息里的 InstallLocation，其次注册表 PrefetchPath，最后默认路径
std::wstring DetectInstallDirectory()
{
	auto normalize = [](std::wstring dir) {
		while (!dir.empty() && (dir.back() == L'\\' || dir.back() == L'/')) {
			dir.pop_back();
		}
		return dir;
	};
	auto hasRxpcc = [](const std::wstring& dir) {
		return !dir.empty() && FileExists(dir + L"\\rxpcc.exe");
	};

	// 1) 卸载信息中的 InstallLocation
	constexpr wchar_t kUninstallRoot[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	HKEY root = nullptr;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, kUninstallRoot, 0, KEY_READ | KEY_WOW64_64KEY, &root) == ERROR_SUCCESS) {
		wchar_t subName[512]{};
		for (DWORD index = 0; ; ++index) {
			DWORD subSize = _countof(subName);
			if (RegEnumKeyExW(root, index, subName, &subSize, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
				break;
			}

			HKEY sub = nullptr;
			if (RegOpenKeyExW(root, subName, 0, KEY_READ | KEY_WOW64_64KEY, &sub) != ERROR_SUCCESS) {
				continue;
			}

			std::wstring installDir;
			wchar_t displayName[512]{};
			DWORD size = sizeof(displayName);
			DWORD type = 0;
			if (RegQueryValueExW(sub, L"DisplayName", nullptr, &type, reinterpret_cast<LPBYTE>(displayName), &size) == ERROR_SUCCESS
				&& wcsstr(displayName, L"PrimoCache") != nullptr) {
				wchar_t location[MAX_PATH]{};
				size = sizeof(location);
				if (RegQueryValueExW(sub, L"InstallLocation", nullptr, &type, reinterpret_cast<LPBYTE>(location), &size) == ERROR_SUCCESS) {
					installDir = normalize(location);
				}
			}
			RegCloseKey(sub);

			if (hasRxpcc(installDir)) {
				RegCloseKey(root);
				return installDir;
			}
		}
		RegCloseKey(root);
	}

	// 2) 驱动参数中的 PrefetchPath，形如 "\DosDevices\C:\Program Files\PrimoCache\prefetch\"
	std::wstring prefetch = ReadRegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Romex Software\\FancyCcV", L"PrefetchPath");
	if (!prefetch.empty()) {
		const std::wstring dosPrefix = L"\\DosDevices\\";
		if (prefetch.compare(0, dosPrefix.size(), dosPrefix) == 0) {
			prefetch.erase(0, dosPrefix.size());
		}
		const std::wstring suffix = L"\\prefetch";
		const std::size_t at = prefetch.rfind(suffix);
		if (at != std::wstring::npos) {
			prefetch.erase(at);
		}
		const std::wstring dir = normalize(prefetch);
		if (hasRxpcc(dir)) {
			return dir;
		}
	}

	// 3) 默认安装路径
	return L"C:\\Program Files\\PrimoCache";
}
/// 取字符串里第一个整数（跳过前导空白，忽略千位分隔符）
std::uint64_t ParseFirstNumber(const std::string& text)
{
	std::uint64_t value = 0;
	bool started = false;
	for (const char ch : text) {
		if (ch >= '0' && ch <= '9') {
			value = value * 10 + static_cast<std::uint64_t>(ch - '0');
			started = true;
		}
		else if (ch == ',') {
			continue;                   // 千位分隔符：数字中间也要忽略，否则 210,726,629,888 会被截断成 210
		}
		else if (started) {
			break;                      // 数字结束
		}
		else if (ch == ' ' || ch == '\t' || ch == '\r') {
			continue;
		}
		else {
			return 0;                   // 遇到其它字符说明不是数字
		}
	}
	return value;
}

/// 把「区间增量 / 秒」换算为速率（字节/秒），并限制在合理范围
/// 注意：直接做 double→uint64 转换时，若 dt 异常小则结果可能超出 uint64 范围（未定义行为），因此先钳位。
std::uint64_t ToRate(std::uint64_t delta, double seconds) noexcept
{
	if (!(seconds > 0.0)) {
		return 0;
	}

	const double rate = static_cast<double>(delta) / seconds;
	if (!(rate > 0.0)) {                    // 同时覆盖 NaN
		return 0;
	}

	constexpr double kMaxRate = 1.0e12;     // 上限 1 TB/s，超出视为异常数据
	return rate > kMaxRate ? static_cast<std::uint64_t>(kMaxRate) : static_cast<std::uint64_t>(rate);
}

/// 解析 rxpcc perf 输出；多卷时累加
bool ParseCounters(const std::string& text, PrimoCacheCounters& counters)
{
	PrimoCacheCounters result;
	bool sawTotalRead = false;

	std::size_t pos = 0;
	while (pos < text.size()) {
		std::size_t eol = text.find('\n', pos);
		if (eol == std::string::npos) {
			eol = text.size();
		}
		std::string line = text.substr(pos, eol - pos);
		pos = eol + 1;

		const std::size_t colon = line.find(':');
		if (colon == std::string::npos) {
			continue;
		}

		std::string label = line.substr(0, colon);
		const std::size_t begin = label.find_first_not_of(" \t\r");
		const std::size_t end = label.find_last_not_of(" \t\r");
		if (begin == std::string::npos) {
			continue;
		}
		label = label.substr(begin, end - begin + 1);

		const std::string value = line.substr(colon + 1);
		if (label == "Total Read") {
			result.total_read += ParseFirstNumber(value);
			sawTotalRead = true;
		}
		else if (label == "Cached Read") {
			result.cached_read += ParseFirstNumber(value);
		}

	}

	if (!sawTotalRead) {
		return false;
	}
	counters = result;
	return true;
}

/// 基于 rxpcc.exe 的数据源实现
class RxpccSource final : public IPrimoCacheSource
{
public:
	RxpccSource() : rxpcc_path_(DetectInstallDirectory() + L"\\rxpcc.exe") {}

	bool Available() const override
	{
		if (!IsProcessElevated()) {
			return false;
		}
		if (!IsPrimoCacheInstalled()) {
			return false;
		}
		return FileExists(GetRxpccPath());
	}

	const std::wstring& Reason() const override
	{
		if (!IsProcessElevated()) {
			reason_ = L"TrafficMonitor 未以管理员身份运行，无法调用 PrimoCache 命令行工具";
		}
		else if (!IsPrimoCacheInstalled()) {
			reason_ = L"未检测到已安装的 PrimoCache";
		}
		else if (!FileExists(GetRxpccPath())) {
			reason_ = L"未找到 rxpcc.exe（PrimoCache 命令行工具）";
		}
		else {
			reason_.clear();
		}
		return reason_;
	}

	bool Sample(PrimoCacheCounters& counters) override
	{
		std::string output;
		if (!RunCommand(output)) {
			return false;
		}
		return ParseCounters(output, counters);
	}

private:
	const std::wstring& GetRxpccPath() const { return rxpcc_path_; }

	bool RunCommand(std::string& output)
	{
		output.clear();

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		HANDLE readPipe = nullptr;
		HANDLE writePipe = nullptr;
		if (CreatePipe(&readPipe, &writePipe, &sa, 0) == FALSE) {
			return false;
		}
		SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

		const HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);

		STARTUPINFOW si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;
		si.hStdInput = (nul != INVALID_HANDLE_VALUE) ? nul : nullptr;

		const std::wstring exePath = GetRxpccPath();
		const std::wstring installDir = exePath.substr(0, exePath.find_last_of(L'\\'));
		std::wstring commandLine = L"\"" + exePath + L"\" " + kCommandArgs;
		std::vector<wchar_t> commandBuffer(commandLine.begin(), commandLine.end());
		commandBuffer.push_back(L'\0');

		PROCESS_INFORMATION pi{};
		const BOOL created = CreateProcessW(exePath.c_str(), commandBuffer.data(), nullptr, nullptr, TRUE,
			CREATE_NO_WINDOW, nullptr, installDir.c_str(), &si, &pi);

		CloseHandle(writePipe);
		if (nul != INVALID_HANDLE_VALUE) {
			CloseHandle(nul);
		}
		if (created == FALSE) {
			CloseHandle(readPipe);
			return false;
		}

		const ULONGLONG deadline = GetTickCount64() + kProcessTimeoutMs;
		bool finished = false;
		char buffer[1024];

		while (true) {
			DWORD available = 0;
			if (PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) == FALSE) {
				break;      // 管道已关闭
			}

			if (available > 0) {
				DWORD bytesRead = 0;
				if (ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, nullptr) == FALSE || bytesRead == 0) {
					break;
				}
				output.append(buffer, bytesRead);
				continue;
			}

			if (WaitForSingleObject(pi.hProcess, 20) == WAIT_OBJECT_0) {
				DWORD rest = 0;
				while (ReadFile(readPipe, buffer, sizeof(buffer), &rest, nullptr) != FALSE && rest > 0) {
					output.append(buffer, rest);
				}
				finished = true;
				break;
			}

			if (GetTickCount64() >= deadline) {
				break;      // 超时
			}
		}

		if (!finished) {
			TerminateProcess(pi.hProcess, 1);
			WaitForSingleObject(pi.hProcess, 1000);
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(readPipe);
		return finished;
	}

	std::wstring rxpcc_path_;    ///< 构造时一次性解析，之后只读（避免多线程惰性初始化竞态）
	mutable std::wstring reason_;
};

} // namespace

void HitRateWindow::Reset() noexcept
{
	reads_.fill(0);
	hits_.fill(0);
	pos_ = 0;
}

void HitRateWindow::Push(std::uint64_t reads, std::uint64_t hits) noexcept
{
	reads_[pos_] = reads;
	hits_[pos_] = hits;
	pos_ = (pos_ + 1) % kHitRateSamples;
}

bool HitRateWindow::GetPercent(double& percent) const noexcept
{
	std::uint64_t totalReads = 0;
	std::uint64_t totalHits = 0;
	for (std::size_t i = 0; i < kHitRateSamples; ++i) {
		totalReads += reads_[i];
		totalHits += hits_[i];
	}

	if (totalReads == 0) {
		return false;       // 窗口内没有任何读取，命中率无意义
	}

	double value = 100.0 * static_cast<double>(totalHits) / static_cast<double>(totalReads);
	value = value < 0.0 ? 0.0 : (value > 100.0 ? 100.0 : value);
	percent = value;
	return true;
}

std::unique_ptr<IPrimoCacheSource> CreatePrimoCacheSource()
{
	return std::make_unique<RxpccSource>();
}

PrimoCacheMonitor::~PrimoCacheMonitor()
{
	stop_.store(true);
	if (worker_.joinable()) {
		worker_.join();
	}
}

void StopPrimoCacheMonitor()
{
	if (g_monitor_started.load()) {
		PrimoCacheMonitor::Instance().RequestStop();
	}
}

PrimoCacheMonitor& PrimoCacheMonitor::Instance()
{
	static PrimoCacheMonitor instance;
	return instance;
}

bool PrimoCacheMonitor::Probe()
{
	if (worker_.joinable()) {
		return true;        // 已经启动过
	}

	source_ = CreatePrimoCacheSource();
	if (!source_ || !source_->Available()) {
		reason_ = source_ ? source_->Reason() : L"无法创建 PrimoCache 数据源";
		source_.reset();
		return false;
	}

	PrimoCacheCounters baseline;
	if (!source_->Sample(baseline)) {
		reason_ = L"PrimoCache 取数失败：请确认服务已启动且以管理员身份运行";
		source_.reset();
		return false;
	}

	previous_ = baseline;
	has_previous_ = true;
	stop_.store(false);

	PrimoCacheSnapshot initial;
	initial.valid = true;
	Publish(initial);

	worker_ = std::thread(&PrimoCacheMonitor::WorkerMain, this);
	g_monitor_started.store(true);
	return true;
}

PrimoCacheSnapshot PrimoCacheMonitor::Snapshot() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return snapshot_;
}

void PrimoCacheMonitor::Publish(const PrimoCacheSnapshot& snapshot)
{
	std::lock_guard<std::mutex> lock(mutex_);
	snapshot_ = snapshot;
}

void PrimoCacheMonitor::RequestStop()
{
	stop_.store(true);
}

void PrimoCacheMonitor::WorkerMain()
{
	try {
	WorkerMainImpl();
	}
	catch (...) {
		// 采样线程内不允许异常逃逸，否则会触发 std::terminate
		PrimoCacheSnapshot failure;
		Publish(failure);
	}
}

void PrimoCacheMonitor::NotifyQueried()
{
	last_query_ms_.store(GetTickCount64());
}

bool PrimoCacheMonitor::SleepInterruptible(int milliseconds)
{
	for (int elapsed = 0; elapsed < milliseconds; elapsed += 50) {
		if (stop_.load()) {
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return stop_.load();
}

void PrimoCacheMonitor::MarkFailure()
{
	++consecutive_failures_;

	// 单次失败视为抖动：不更新快照，沿用上一次的显示值；
	// 连续失败达到阈值才置为无效（上层显示 --）。
	if (consecutive_failures_ >= kFailureThreshold) {
		PrimoCacheSnapshot stale = Snapshot();
		stale.valid = false;
		stale.rate_valid = false;
		stale.hit_rate_valid = false;
		Publish(stale);
	}
}

void PrimoCacheMonitor::WorkerMainImpl()
{
	LARGE_INTEGER frequency{};
	QueryPerformanceFrequency(&frequency);
	LARGE_INTEGER previousTick{};
	bool active = false;        // 是否处于“有显示项在用”的持续采样状态

	while (!stop_.load()) {
		// 惰性判据：主程序只对正在显示的项调用取值接口
		const ULONGLONG now_ms = GetTickCount64();
		const ULONGLONG last_query = last_query_ms_.load();
		const bool in_use = last_query != 0
			&& (now_ms - last_query) <= static_cast<ULONGLONG>(kIdleTimeoutMs);

		if (!in_use) {
			// 没有任何 PrimoCache 显示项在绘制：完全停止采样（不启动 rxpcc），并丢弃基准
			if (active) {
				// 清除上一次的速率值，避免下次恢复显示时短暂显示空闲前的旧值。
				PrimoCacheSnapshot idle;
				idle.valid = true;
				Publish(idle);
			}
			active = false;
			has_previous_ = false;
			consecutive_failures_ = 0;
			hit_rate_window_.Reset();
			if (SleepInterruptible(kIdlePollMs)) {
				break;
			}
			continue;
		}

		if (!active) {
			// 刚恢复显示：先采一次作为基准（不发布速率），
			// 否则会把空闲期间累计的增长当成“速率”。
			PrimoCacheSnapshot not_ready;
			not_ready.valid = true;
			Publish(not_ready);

			PrimoCacheCounters baseline;
			if (source_->Sample(baseline)) {
				previous_ = baseline;
				has_previous_ = true;
				consecutive_failures_ = 0;
				QueryPerformanceCounter(&previousTick);
			}
			else {
				MarkFailure();
			}
			active = true;
			continue;       // 下一轮循环等待 kIntervalMs 后再采第二次，即可算出速率
		}

		if (SleepInterruptible(kIntervalMs)) {
			break;
		}

		PrimoCacheCounters current;
		if (!source_->Sample(current)) {
			MarkFailure();
			continue;       // 保留时间基准，下次成功采样时按实际间隔计算速率
		}
		consecutive_failures_ = 0;

		LARGE_INTEGER now{};
		QueryPerformanceCounter(&now);
		const double seconds = static_cast<double>(now.QuadPart - previousTick.QuadPart)
			/ static_cast<double>(frequency.QuadPart);

		PrimoCacheSnapshot snapshot;
		snapshot.valid = true;

		if (has_previous_ && seconds > 0.0) {
			const std::uint64_t deltaRead = current.total_read >= previous_.total_read
				? current.total_read - previous_.total_read : 0;
			const std::uint64_t deltaHit = current.cached_read >= previous_.cached_read
				? current.cached_read - previous_.cached_read : 0;

			const std::uint64_t deltaMiss = deltaRead >= deltaHit ? deltaRead - deltaHit : 0;

			snapshot.rate_valid = true;
			snapshot.read_speed = ToRate(deltaRead, seconds);
			snapshot.hit_speed = ToRate(deltaHit, seconds);
			snapshot.miss_speed = ToRate(deltaMiss, seconds);

			// 命中率：取最近 kHitRateWindowMs（30 秒）内的累计比值，避免小样本跳变
			hit_rate_window_.Push(deltaRead, deltaHit);
			double percent = 0.0;
			if (hit_rate_window_.GetPercent(percent)) {
				snapshot.hit_rate_valid = true;
				snapshot.hit_rate_percent = percent;
			}
		}

		previous_ = current;
		has_previous_ = true;
		previousTick = now;
		Publish(snapshot);
	}
}
