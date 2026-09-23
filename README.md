# taskbardynamic

TrafficMonitor 插件：把**实时上传/下载速度**和 **CPU 温度**显示在任务栏上，并用时间窗口内的历史极值**自适应绘制资源占用图**。

- 显示项由配置表驱动，新增一个监控项只需要改 `config.cpp` 加一行代码。
- 绘图比例不是固定的，而是按最近 10 分钟窗口内的 `[最小值, 最大值]` 动态归一化，因此低速和高速场景下图形都有明显的起伏。
- 纯 C++17 实现，无第三方依赖，编译产物只有一个 DLL。

---

## 目录

- [功能特性](#功能特性)
- [环境要求](#环境要求)
- [编译](#编译)
- [安装与使用](#安装与使用)
- [显示项](#显示项)
- [工作原理](#工作原理)
  - [分层结构](#分层结构)
  - [运行时数据流](#运行时数据流)
  - [滑动窗口算法](#滑动窗口算法)
  - [网速格式化规则](#网速格式化规则)
- [扩展指南：新增一个显示项](#扩展指南新增一个显示项)
- [配置参考](#配置参考)
- [代码结构](#代码结构)
- [常见问题](#常见问题)
- [更新日志](#更新日志)
- [许可](#许可)

---

## 功能特性

| 特性 | 说明 |
| --- | --- |
| 上传速度 | 任务栏显示 `↑: 12.1MB/s` |
| 下载速度 | 任务栏显示 `↓: 12.1MB/s` |
| CPU 温度 | 任务栏显示 `CPU: 45°C` |
| 资源占用图 | 三个显示项都会绘制图形，取值按 10 分钟窗口的历史极值归一化 |
| 自动单位 | 数值部分固定 4 个字符，单位在 `B/s`、`KB/s`、`MB/s`、`GB/s` 之间自动切换 |
| 声明式配置 | 显示项的名称、ID、标签、示例文本、取数回调、格式化回调全部集中在一张表里 |
| PrimoCache 命中 / 未命中速度 | `命中: 11.8MB/s`、`未中: 345KB/s` |
| PrimoCache 实时命中率 | `命中率: 99.7%`（最近 5 秒区间） |

---

## 环境要求

| 项目 | 要求 |
| --- | --- |
| 操作系统 | Windows 10 / 11（x64 或 x86） |
| 编译器 | Visual Studio 2022（v143 工具集），或 VS 2022 生成工具 |
| 组件 | “使用 C++ 的桌面开发”工作负载（含 Windows SDK） |
| 语言标准 | C++17 |
| 宿主程序 | [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)（插件 API 版本 6） |

---

## 编译

### 方式一：Visual Studio

1. 打开 `taskbardynamic.sln`；
2. 选择配置 `Release` + 平台 `x64`；
3. 生成解决方案（F7）。

### 方式二：命令行 MSBuild

```powershell
msbuild taskbardynamic.sln /p:Configuration=Release /p:Platform=x64
```

可用的平台参数为 `x64` 和 `Win32`（解决方案中 `Win32` 对应工程里的 `x86` 配置）。

### 产物

```
x64\Release\taskbardynamic-x64.dll   # Release + x64
x64\Debug\taskbardynamic-x64.dll     # Debug   + x64
Release\taskbardynamic-x86.dll       # Release + Win32(x86)
Debug\taskbardynamic-x86.dll         # Debug   + Win32(x86)
```

生成的 DLL 只导出一个符号：

```
TMPluginGetInstance
```

这是 TrafficMonitor 插件约定的入口函数，返回 `ITMPlugin` 接口指针。

> **常见环境问题**：如果当前 shell 的环境变量里同时存在 `Path` 和 `PATH` 两个键（某些从 PowerShell 派生的 CI 环境会出现），MSBuild 会直接失败并报
> `MSB6001: CL.exe 的命令行开关无效 ... 字典中的关键字:"Path"所添加的关键字:"PATH"`。
> 这不是工程本身的问题，改用 **Developer Command Prompt for VS 2022**，或在调用前把重复的环境变量清理掉即可。

---

## 安装与使用

1. 把与主程序**位数一致**的 DLL（`taskbardynamic-x64.dll` 或 `taskbardynamic-x86.dll`）复制到 TrafficMonitor 的 `plugins` 目录（与主程序 `TrafficMonitor.exe` 同级）；
2. 重启 TrafficMonitor；
3. 打开 **选项 → 任务栏窗口设置 / 显示设置**，勾选需要的显示项；
4. 若需要在任务栏上看到图形，请确保勾选了该显示项对应的“显示资源占用图”类选项。

卸载时删除 DLL 并在主程序中取消勾选即可。

> **升级提示**：从旧版本（旧文件名 `taskbardynamic.dll`）升级时，请先删除 `plugins` 目录里旧的那个 DLL，再放入新文件，否则新旧两个 DLL 会同时被加载，任务栏/悬浮窗上的显示项会重复出现。

---

## 显示项

| 名称 | ID | 标签 | 数值示例 | 数据来源 | 绘制图形 |
| --- | --- | --- | --- | --- | --- |
| upload speed | `mq72ZrBHN2` | `↑:` | `12.1MB/s` | `MonitorInfo::up_speed` | 是 |
| download speed | `DE7etVv1nR` | `↓:` | `12.1MB/s` | `MonitorInfo::down_speed` | 是 |
| cpu temperature | `ST5sLiDY3f` | `CPU:` | `17°C` | `MonitorInfo::cpu_temperature` | 是 |
| primocache hit read speed | `PC_HIT_SPD` | `命中:` | `12.1MB/s` | `rxpcc perf`：Cached Read 增量 | 是 |
| primocache miss read speed | `PC_MISS_SPD` | `未中:` | `345KB/s` | 总读取 − 命中 | 是 |
| primocache hit rate | `PC_HIT_RATE` | `命中率:` | `99.7%` | 命中 / 总读取 | 是 |

> **注意**：`ID` 是主程序识别显示项的键。修改 ID 等同于新增一个显示项，用户原有的勾选状态、颜色等设置会失效，因此不要随意改动。

---

## PrimoCache 集成（可选）

插件内置 3 个 PrimoCache 显示项，数据来自 PrimoCache 自带的命令行工具 `rxpcc.exe`：

```
后台线程每 5 秒执行一次   rxpcc perf -a -u=b -s
        ↓  解析累计计数，并与上一次采样求差
未命中速度 = Δ(Total Read − Cached Read)/Δt
实时命中率 = Δ(Cached Read) / Δ(Total Read) × 100%
```

### 启用条件（不满足时这些显示项根本不注册）

| 条件 | 说明 |
| --- | --- |
| 已安装 PrimoCache | 以注册表 `HKLM\SOFTWARE\Romex Software\FancyCcV` 是否存在判断 |
| TrafficMonitor 以管理员身份运行 | `rxpcc.exe` 需要管理员权限，未提权无法取数 |
| 能找到 `rxpcc.exe` | 先从卸载信息读 `InstallLocation`，失败则回退默认路径 `C:\Program Files\PrimoCache` |
| 启动时试采成功 | 加载插件时先采样一次，取不到数据就不注册这些显示项 |

### 采样间隔与开销

- 间隔由 `PrimoCacheMonitor::kIntervalMs` 控制，**默认 5000 毫秒**。
- 本机实测单次 `rxpcc perf` 调用约 **44 ms 墙钟 / 16 ms CPU**（含进程创建），即 5 秒间隔约 **0.9% 时间 / 0.3% CPU**；想更省可改成 `10000`。
- **惰性采样**：只有这 3 个显示项中任意一个被主程序绘制时才采样；若连续 30 秒（`kIdleTimeoutMs`）没有任何一项被绘制，就**完全停止采样**（不再启动 `rxpcc`，零开销）。
  恢复显示时会先采一次作为基准，约 5 秒后给出速率（避免把空闲期间累计的增长当成速率）。
- 采样在**后台线程**执行，UI 线程只读取快照，不会阻塞主程序。

### 异常与无活动时的显示

| 情况 | 显示 |
| --- | --- |
| 区间内没有读取（命中率分母为 0） | 命中率显示 `--`，各项速度显示 `0.00B/s` |
| 偶发一次采样失败 | 沿用上一次的数值（视为抖动，不清空显示） |
| 连续 2 次及以上采样失败 | 各项显示 `--` |
| 未提权或未安装 PrimoCache | 显示项不注册，主程序列表中看不到 |

> 「命中」= 由缓存（L1 内存 + L2 SSD）服务的读取；「未命中」= 总读取 − 命中，即回落到物理磁盘的部分。若未启用一级缓存，命中全部来自二级缓存。

## 工作原理

### 分层结构

```
┌──────────────────────────────────────────────────────────────┐
│ PluginInterface.h     上游 TrafficMonitor 插件接口（API v6） │
│                       第三方文件，请勿修改                   │
└──────────────────────────────────────────────────────────────┘
              ▲                                   ▲
              │ 实现 ITMPlugin                    │ 实现 IPluginItem
┌──────────────────────────────┐   ┌──────────────────────────────┐
│ TaskBarDynamic               │──▶│ DynamicData<T>               │
│  · 插件单例与元信息          │   │  · 把 IPluginItem 接口转发   │
│  · 定时推进所有显示项        │   │    到 DynamicInfo 的回调     │
│  · 维护显示项列表 items_     │   │  · 缓存显示文本              │
└──────────────────────────────┘   └──────────────────────────────┘
                                                 │
                                                 ▼
                                   ┌──────────────────────────────┐
                                   │ DynamicBase<T>               │
                                   │  · 滑动窗口极值统计          │
                                   │  · 归一化为 0.0 ~ 1.0        │
                                   └──────────────────────────────┘
```

| 文件 | 职责 |
| --- | --- |
| `PluginInterface.h` | 上游接口定义，`ITMPlugin` / `IPluginItem` / `MonitorInfo` |
| `TaskBarDynamic.h/.cpp` | 插件主体：元信息、显示项列表、周期回调 |
| `DynamicData.h/.cpp` | 单个显示项：接口适配 + 文本缓存，实例化 `int` / `unsigned long long` / `float` |
| `DynamicBase.h/.cpp` | 滑动窗口极值统计与归一化，与具体业务无关 |
| `config.h/.cpp` | 显示项配置表与取数/格式化回调 |
| `PrimoCache.h/.cpp` | PrimoCache 数据源抽象（`IPrimoCacheSource`）、CLI 实现与后台采样器 |
| `dllmain.cpp` | DLL 入口，无额外逻辑 |

### 运行时数据流

```
主程序                                            插件
  │                                                  │
  │ GetInfo(TMI_NAME/TMI_VERSION/...)  ─────────────▶│ 返回插件元信息
  │ GetItem(index)                     ─────────────▶│ 返回第 index 个显示项
  │                                                  │
  │ OnMonitorInfo(MonitorInfo)         ─────────────▶│ DynamicWindow::OnMonitorInfo
  │                                                  │   └─▶ 每个 DynamicData::SetData()
  │                                                  │        └─ get_data_() 取出原始值并缓存
  │ DataRequired()（周期性调用）        ─────────────▶│ DynamicWindow::DataRequired
  │                                                  │   └─▶ 每个 DynamicData::GenerateData()
  │                                                  │        ├─ DynamicBase::PutInValue() 推进窗口
  │                                                  │        └─ set_data_() 刷新显示文本
  │ GetItemValueText()                 ─────────────▶│ 返回缓存好的文本
  │ GetResourceUsageGraphValue()       ─────────────▶│ DynamicBase::GenerateValue()
```

取数与格式化被拆成两个回调，好处是 `GetItemValueText()`（主程序可能高频调用）只做字符串返回，不会触发任何计算。

### 滑动窗口算法

`DynamicBase<T>` 维护 4 档窗口，索引由长到短：

| 索引 | 窗口 | 含义 |
| --- | --- | --- |
| 0 | 10 分钟 | 归一化所使用的最长窗口 |
| 1 | 5 分钟 | 极值淘汰时的回退来源 |
| 2 | 2 分钟 | 同上 |
| 3 | 当前值 | 每次采样直接写入 |

每档同时记录**取得该极值的时刻**。一次采样（`PutInValue`）依次做三件事：

```
1) 写入当前值         min_[3] = max_[3] = { 当前值, 现在 }

2) 淘汰过期极值（由短窗口到长窗口）
   若某档极值的时间戳距现在超过该档的窗口长度：
       min_[i] = min_[i + 1]，max_[i] = max_[i + 1]

3) 收缩长窗口（由短窗口到长窗口）
   min_[i] = min(min_[i], min_[i + 1])
   max_[i] = max(max_[i], max_[i + 1])
```

绘图取值由最长窗口归一化得到，并做了边界保护：

```
若 max_[0] == min_[0]（窗口内数值恒定）      -> 返回 0.0，避免 0/0 产生 NaN
否则                                        -> clamp((当前值 - min_[0]) / (max_[0] - min_[0]), 0, 1)
```

`clamp` 保证了返回值一定落在接口约定的 `0.0 ~ 1.0` 区间内，即使当前值已经超出窗口极值也不会溢出。

> 第 2 步必须**由短窗口到长窗口**执行。若反过来，长窗口会读到短窗口尚未刷新的旧极值，导致一个已经出窗的极值在窗口中多残留若干个采样周期。

### 网速格式化规则

单位按 1024 进制换算：

| 原始值范围 | 显示单位 |
| --- | --- |
| `< 1024` | `B/s` |
| `< 1024 × 1024` | `KB/s` |
| `< 1024 × 1024 × 1024` | `MB/s` |
| `≥ 1024 × 1024 × 1024` | `GB/s` |

数值部分固定为 **4 个字符**，避免任务栏上的数值宽度跳动：

| 数值大小 | 格式 | 示例 |
| --- | --- | --- |
| `≥ 100` | 整数 | `512B/s`、`100KB/s`、`999KB/s`、`1023KB/s` |
| `≥ 10` | 一位小数 | `12.1MB/s`、`99.9MB/s` |
| `< 10` | 两位小数 | `1.00KB/s`、`1.50MB/s`、`0.00B/s` |

| 输入（字节/秒） | 输出 |
| --- | --- |
| 0 | `0.00B/s` |
| 512 | `512B/s` |
| 1024 | `1.00KB/s` |
| 102400 | `100KB/s` |
| 1022976（999 KiB/s） | `999KB/s` |
| 1048576（1 MiB/s） | `1.00MB/s` |
| 12685312（约 12.1 MiB/s） | `12.1MB/s` |
| 1073741824（1 GiB/s） | `1.00GB/s` |

---

## 扩展指南：新增一个显示项

假设要新增“GPU 温度”，只需改 `config.cpp` 和 `config.h` 两个文件、共三处。

### 第 1 步：声明取数与格式化函数（`config.h`）

```cpp
namespace config {
    int GetGpuTemperature(const ITMPlugin::MonitorInfo& monitor_info);
    void SetGpuTemperature(std::wstring& text, int value);

    extern DynamicInfo<int> gpu_temperature_info_;
}
```

### 第 2 步：实现回调并追加一条配置（`config.cpp`）

```cpp
int GetGpuTemperature(const ITMPlugin::MonitorInfo& monitor_info) {
    return monitor_info.gpu_temperature;
}

void SetGpuTemperature(std::wstring& text, int value) {
    text = std::to_wstring(value) + L"°C";
}

DynamicInfo<int> gpu_temperature_info_ = {
    L"gpu temperature",   // 显示项名称
    L"GPU_TEMP_001",      // 显示项唯一 ID（自定义，不能与其它项重复）
    L"GPU:",              // 任务栏上的标签
    L"17°C",              // 数值示例文本，用于估算显示宽度
    1,                    // 是否绘制资源占用图：1 绘制，0 不绘制
    GetGpuTemperature,    // 取数回调
    SetGpuTemperature     // 格式化回调
};
```

### 第 3 步：注册到插件（`TaskBarDynamic.cpp` 构造函数）

```cpp
DynamicWindow::DynamicWindow()
{
    AddItem(config::upload_info_);
    AddItem(config::download_info_);
    AddItem(config::cpu_temperature_info_);
    AddItem(config::gpu_temperature_info_);   // 新增的这一行
}
```

`AddItem()` 会根据 `DynamicInfo<T>` 的 `T` 自动实例化对应的 `DynamicData<T>` 并登记为一个 `IPluginItem`，显示顺序就是这里的添加顺序。

---

## 配置参考

`DynamicInfo<T>` 的字段含义：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `name_` | `std::wstring` | 显示项名称，出现在主程序的显示项列表里 |
| `id_` | `std::wstring` | 显示项唯一 ID，主程序据此保存用户设置，**不要随意修改** |
| `lable_` | `std::wstring` | 显示在数值前面的标签，如 `↑:`、`CPU:` |
| `value_sample_` | `std::wstring` | 数值示例文本，主程序按它的长度预留显示宽度 |
| `draw_graph_` | `int` | 是否绘制资源占用图：`1` 绘制，`0` 不绘制 |
| `get_data_` | `std::function<T(const MonitorInfo&)>` | 从主程序推送的监控数据里取出原始值 |
| `set_data_` | `std::function<void(std::wstring&, T)>` | 把原始值格式化成显示文本 |

`MonitorInfo` 中可供使用的字段：`up_speed`、`down_speed`、`cpu_usage`、`memory_usage`、`gpu_usage`、`hdd_usage`、`cpu_temperature`、`gpu_temperature`、`hdd_temperature`、`main_board_temperature`、`cpu_freq`。

---

## 代码结构

```
taskbardynamic/
├── taskbardynamic.sln
├── README.md
└── taskbardynamic/
    ├── PluginInterface.h        # 上游插件接口（保持原样）
    ├── TaskBarDynamic.h/.cpp    # 插件主体与显示项列表
    ├── DynamicData.h/.cpp       # IPluginItem 适配层
    ├── DynamicBase.h/.cpp       # 滑动窗口极值统计
    ├── config.h/.cpp            # 显示项配置表
    ├── dllmain.cpp              # DLL 入口
    ├── pch.h / pch.cpp          # 预编译头
    ├── framework.h              # Windows 头文件聚合
    └── resource.h
```

工程配置要点（`taskbardynamic.vcxproj`）：

- 字符集 `Unicode`，配置类型 `DynamicLibrary`；
- 四种配置（Debug/Release × x64/Win32）统一使用 **C++17**；
- 开启 **多处理器编译**（`/MP`）与一致性模式（`/permissive-`）；
- 编译产物按目标平台命名（`TargetName`）：x64 输出 `taskbardynamic-x64.dll`，Win32 输出 `taskbardynamic-x86.dll`；
- 源码为 **UTF-8 with BOM**；上游文件 `PluginInterface.h` 保持原始 GBK 编码，只含注释，不影响编译。

---

## 常见问题

**Q：编译时报 `MSB6001 ... 字典中的关键字:"Path"所添加的关键字:"PATH"`？**
环境变量里同时存在 `Path` 与 `PATH` 两个键导致的，与工程无关。请使用 Developer Command Prompt for VS 2022，或清理重复的环境变量后重试。

**Q：编译出来的 DLL 放进 plugins 目录后主程序里看不到插件？**
确认 DLL 与主程序位数一致（64 位主程序配 x64 DLL），并确认 TrafficMonitor 版本支持插件 API v6。主程序启动时才会扫描 plugins 目录，需要重启。

**Q：CPU 温度一直是 0？**
温度来自主程序采集，读取传感器通常需要以管理员身份运行 TrafficMonitor；部分主板/虚拟机也不提供该传感器。

**Q：修改了标签或名称，主程序里没变化？**
`name_` 会随插件重新加载显示，但勾选状态由 `id_` 决定。改完 DLL 后重启主程序；如果改的是 `id_`，需要在选项里重新勾选。

**Q：任务栏上的图形是空白的？**
图形需要窗口内存在极值差（`max != min`）。数值恒定（例如温度长时间不变）时归一化结果恒为 0，这是刻意为之的保护行为，用于避免除零。另外请确认主程序里勾选了对应的“显示资源占用图”选项。

**Q：低速时显示 `0.00B/s` 而不是 `0.00KB/s`？**
单位会按数量级自动切换，低于 1024 B/s 时直接使用 `B/s`，比固定使用 `KB/s` 更直观。

---

## 更新日志

### v1.2.1（未发布）

本版本是 PrimoCache 显示项的缺陷修复与健壮性改进，**尚未发布**（等命中率平滑方案完成后一并发布）。

**改进**

- **命中率平滑**：由“单个 5 秒区间的比值”改为“最近 30 秒窗口内的累计比值”（`kHitRateWindowMs = 30000`，即 6 个采样），避免读取量很少时命中率在 0% / 100% 之间跳变；
- 只有整个 30 秒窗口内都没有读取时才显示 `--`（命中率没有分母），一旦恢复读取即自动重新给出数值。

**缺陷修复**

- 修复恢复显示后“多睡一次”导致首次数值要等约 10 秒才出现的问题，现在约 5 秒即可给出速率；
- 修复数字解析遇到**千位分隔符会被截断**的问题（`210,726,629,888` 曾被解析成 `210`，会直接导致速度与命中率错误）；
- 命中率增加 `0 ~ 100` 钳位，避免计数错位时显示超过 100%；
- 采样超时判断改用 `GetTickCount64()`，去掉 `GetTickCount()` 的 49 天溢出隐患（静态分析 C28159）。

**清理**

- 移除已无显示项使用的写入统计（`total_write` / `write_speed` / `deltaWrite`）。

**验证**

- 4 种配置全部 `0 warning / 0 error`；MSVC 静态分析（`/analyze`）无告警；
- 解析器边界用例 7/7 通过：真实输出 / 千位分隔符 / 多卷累加 / 空输出 / 缺少字段 / LF+制表符 / 计数相同。

### v1.2

- 新增 3 个 PrimoCache 显示项：命中速度、未命中速度、实时命中率；
- 数据经 `rxpcc.exe` 后台采样（默认 5 秒）计算增量速率；数据源已抽象为 `IPrimoCacheSource`，后续可平滑替换为驱动直读（`\\.\FancyCcV` + IOCTL）；
- 未提权或未安装 PrimoCache 时**不注册**这些显示项（不占用任何资源）；
- 版本号提升到 `v1.2`。

### v1.1

本次为代码优化版本，行为与稳定性改动如下：

**缺陷修复**

- 修复窗口内数值恒定时 `max == min` 导致 `0/0` 产生 **NaN**、使绘图异常的问题，现在返回 `0.0`；
- 修复 **网速文本被截断成畸形字符串** 的问题（例如 `999.KB/s`、`1023.KB/s`），改为按数值量级选择精度；
- 修复极值淘汰顺序导致 **过期极值在窗口中多残留若干采样周期** 的问题；
- 修复首次采样时各档残留默认 `0` 时刻的伪极值，导致窗口最小值长期为 0 的问题。

**改进**

- 时间间隔改用 `std::chrono` + `FILETIME` 刻度精确计算，去掉原来按“每月 30 天”近似换算的分钟数，跨月/跨年不再漂移；
- 绘图值增加 `[0, 1]` 钳位，即使当前值已超出窗口极值也不会溢出接口约定范围；
- 单位换算统一为 1024 进制，并支持 `B/s`、`GB/s`；
- 显示项注册改为 `AddItem()` 模板方法，`DynamicWindow` 构造函数现在与配置表一一对应；
- 补齐 `override`；`SetData` / `GenerateData` 收为私有；`GetItem` 的索引比较改用显式类型转换；
- `GetInfo(TMI_MAX)` 不再返回无意义的 `L"3"`，描述文字与名称不再重复，版本号更新为 `v1.1`；
- 移除编译警告 C4244；
- 工程配置：为 x86 配置补齐 C++17 标准，统一开启 `/MP` 多处理器编译；
- 源码统一为 UTF-8 with BOM；
- 编译产物按位数命名：`taskbardynamic-x64.dll` / `taskbardynamic-x86.dll`（升级时请先删除 `plugins` 目录里旧版的 `taskbardynamic.dll`，避免重复加载）。

### v1.0

- 初版：上传速度、下载速度、CPU 温度三个显示项，滑动窗口自适应绘图。

---

## 许可

本仓库未附带许可证文件，请遵循作者发布的原始说明使用。

`PluginInterface.h` 来自 TrafficMonitor 项目（Copyright (C) Zhong Yang），版权归原作者所有。