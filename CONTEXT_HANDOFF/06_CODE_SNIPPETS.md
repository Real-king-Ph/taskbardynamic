# 关键原文与代码片段

> 上级入口：[00_INDEX.md](00_INDEX.md)
> 用途：拆分自旧 `_HANDOFF.md` 第 11 章。
> 权威 ID：P-xxx 精确片段
> 不存放：大段历史、决策理由和普通技术概览。
> 更新时间：2026-09-23
> 迁移说明：旧章节编号 (11)；完整映射见 `00_INDEX.md`。

## 11. 关键原文与代码片段

- 片段 ID：P-001
- 来源：用户原话（旧会话，2026-09-23，[S-001]）
- 用途：新对话必须知道用户对功能与流程的原始要求（避免再次偏离）
- 原文（按时间顺序摘录）：
  - 「帮我扫描一下这个项目」
  - 「帮我优化一下这份代码，并且生成文档给我」
  - 「尝试进行运行一次，使用当前运行的Traffic Monitor」
  - 「任务栏窗口都变成了乱码，帮我修复」
  - 「先把插件新改动 commit ，然后push到远端。同步一下最新的 dll 到远端。」
  - 「新增 release V1.1」／「删除 V1.0 Release」／「同时提供一个 x32 的版本。」
  - 「我想要用在插件里显示几个信息 1. 读取速度 2. 命中的速度 3. 未命中的速度 4. 实时命中率。」
  - 「先不要实现，给我几个方案，我再敲定细节」
  - 「查找一下 primocache 有没有提供一些方便读取的SDK。采用方案C，如果本机没有管理员权限以及不存在 primocache 的时候这个插件选项不生效。以及窗口时间调长，我需要知道性能消耗，如果过大则窗口时间拉长。」
  - 「1. 是这个意思。我想增加一个性功能，当写入>读取时则显示写入，反之显示读取。 2. 间隔5s，我想在任务栏图标也变成 5s 一个柱子， 3. 是否显示由我自己控制。」
  - 「先在本地更新一下插件，测试一下功能是否全部能用且没有 bug」
  - 「删除 4 我不需要这个数据，同时动态改变标签也不生效，这个功能删去。」
  - 「先生成一份计划，把所有可以手动操作的动作都交给我，过会就要进入到 DeepSeek 高峰期了，我在谷时再启动你」
  - 「不需要读取速度，也不需要严格同位。」
  - 「2次以上失败才进行显示 -- ，不要显示 NA」
  - 「1 可以采用惰性采样」
  - 「插件默认这些选项都不是开启的。本地修改我怕自己来」
  - 「你不用修改了，待会我自己修改，另外命中和未命中修改一下名称，让其简洁对称。」
  - 「记得把测试过程中产生的不需要commit 的所有文件删除，包括NEXT-STEPS」
  - 「检测是否存在bug」
  - 「目前还有一个体验上的问题。我的命中率不是连续的，怎么设计让这个命中率更加平滑的变化。」／「我选择方案A，但先别执行。」
  - 「先依次执行，再发布之前要执行一次代码检查，是否存在漏洞，如果存在漏洞则需要修复后重复运行之前的步骤。」
  - 「后续如果需要我手动UAC的操作先不要进行，我要睡觉去了，等我回来再统一执行需要我手操的任务。」
  - 「执行 A1」
  - 「这个插件还有没有什么值得优化的地方？」（**未获回答**）
  - 「README 文档修复一下」（**未完成**）

- 片段 ID：P-002
- 来源：[S-006]（`taskbardynamic/PrimoCache.h` L13-L18）
- 用途：调整采样/平滑行为时直接改这里
- 原文：
```
/// PrimoCache 采样参数
constexpr int kSampleIntervalMs = 5000;     ///< 采样间隔（毫秒）：单次 rxpcc 调用实测约 44ms 墙钟 / 16ms CPU
constexpr int kHitRateWindowMs = 30000;     ///< 命中率统计窗口（毫秒）：取窗口内的累计比值，避免单次 I/O 导致 0%/100% 跳变
constexpr std::size_t kHitRateSamples = static_cast<std::size_t>(kHitRateWindowMs / kSampleIntervalMs);   ///< 窗口内的采样数
static_assert(kSampleIntervalMs > 0, "kSampleIntervalMs 必须大于 0");
static_assert(kHitRateSamples >= 1, "kHitRateWindowMs 必须不小于一个采样间隔，否则命中率窗口为空（会导致取模 0）");
```
（另有 `PrimoCacheMonitor` 内：`kIntervalMs = kSampleIntervalMs`、`kFailureThreshold = 2`、`kIdleTimeoutMs = 30000`、`kIdlePollMs = 1000`）

- 片段 ID：P-003
- 来源：[S-002]（旧会话 08:54 回复，`PrimoCache.cpp` 约 L464）
- 用途：失败容错语义的权威实现（新对话不要改回单次失败即 `--`）
- 原文：
```cpp
if (!source_->Sample(current)) {
    ++consecutive_failures_;
    // 单次失败视为抖动：不更新快照，沿用上一次的显示值
    if (consecutive_failures_ >= kFailureThreshold) {
        PrimoCacheSnapshot stale = Snapshot();
        stale.valid = false;
        stale.rate_valid = false;
        stale.hit_rate_valid = false;
        Publish(stale);           // → 上层显示 --
    }
    continue;
}
consecutive_failures_ = 0;        // 成功即清零
```

- 片段 ID：P-004
- 来源：[S-006]（`taskbardynamic/config.cpp` L159-L190）
- 用途：显示项注册表的准确值（ID/标签/示例/是否绘图）
- 原文（节选）：
```cpp
DynamicInfo<unsigned long long> primo_hit_speed_info_  = { L"primocache hit read speed",  L"PC_HIT_SPD",  L"命中:",   L"12.1MB/s", 1, ... };
DynamicInfo<unsigned long long> primo_miss_speed_info_ = { L"primocache miss read speed", L"PC_MISS_SPD", L"未中:",   L"345KB/s",  1, ... };
DynamicInfo<float>              primo_hit_rate_info_   = { L"primocache hit rate",       L"PC_HIT_RATE", L"命中率:", L"99.9%",    1, ... };
```

- 片段 ID：P-005
- 来源：[S-002]（旧会话 13:22 回复），行号见 [Q-005]
- 用途：唯一需要用户手工完成的配置改动（助手不得代改，[C-002]）
- 原文（目标状态）：
```
C:\tool\TrafficMonitor\config.ini
  tbar_display_item    : 520 → 8
  plugin_display_item  : → mq72ZrBHN2,DE7etVv1nR,PC_HIT_RATE
  PC_MISS_SPD          : 未命中: → 未中:      （标签由 config 缓存，必须改这里）
```

- 片段 ID：P-006
- 来源：[S-002]（旧会话 17:01 回复）
- 用途：v1.2.1 已发布产物的权威校验值（发布/部署前比对）
- 原文：
```
v1.2.1  latest=True  tag→261213e
  taskbardynamic-x64.dll   71,168 bytes   sha256=03338161e4d25d39...
  taskbardynamic-x86.dll   64,512 bytes   sha256=ff6d7402f6f30599...
本机 plugins\taskbardynamic-x64.dll 与本附件哈希完全相同 → 本地跑的就是发布版
```

- 片段 ID：P-007
- 来源：本会话用户原话（[S-003]）
- 用途：本轮交接任务的验收口径
- 原文：
  - 「你现在暂停原任务，执行一次"长对话上下文压缩与交接"。唯一目标：从本项目的全部可见上下文中，提取对后续工作仍然重要的信息，去重、分类、消歧并压缩，然后写入：D:\coding\taskbardynamic\CONTEXT\_HANDOFF.md」
  - 「不要一下子构建完成，要逐步构建，防止上下文爆炸导致这个对话没法完成任务。」

- 片段 ID：P-008
- 来源：提交 `3a653a8`（`DynamicBase.h/.cpp`、`DynamicData.h/.cpp`、`TaskBarDynamic.cpp`）
- 用途：绘图窗口不得再依赖本地 `SYSTEMTIME`，避免系统时间调整污染历史极值
- 原文（要点）：
```cpp
using Clock = std::chrono::steady_clock;
using Stamp = Clock::time_point;
void DynamicBase<T>::PutInValue() {
    const Stamp now = Clock::now();
    // ...
}

struct DynamicInfo {
    std::wstring label_;  // 原拼写 lable_ 已废弃
};
```

- 片段 ID：P-009
- 来源：提交 `f47e36c`（`config.cpp`、`PrimoCache.cpp`、README）
- 用途：速率未就绪时必须显示 `--`，不能展示默认 0 或空闲前的旧值
- 原文（要点）：
```cpp
std::atomic<bool> g_primo_rate_valid{ false };
g_primo_rate_valid.store(snapshot.rate_valid);

void SetPrimoSpeed(std::wstring& text, unsigned long long value) {
    if (!g_primo_valid.load() || !g_primo_rate_valid.load()) {
        text = L"--";
        return;
    }
    SetNetSpeed(text, value);
}

// 进入空闲及重新建立基准时发布默认快照：valid=true, rate_valid=false
```

- 片段 ID：P-010
- 来源：2026-09-24 工作区修改（尚未提交）
- 用途：`rxpcc` 字段缺失、非法值或溢出时必须让本次采样失败，不能静默解析为 0
- 原文（要点）：
```cpp
bool ParseFirstNumber(const std::string& text, std::uint64_t& value) noexcept;

constexpr std::uint64_t kMax = static_cast<std::uint64_t>(-1);
if (!ParseFirstNumber(valueText, parsed) || parsed > kMax - result.total_read) {
    return false;
}

if (!sawTotalRead || !sawCachedRead) {
    return false;
}
```
