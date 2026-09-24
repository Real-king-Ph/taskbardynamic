# 技术细节速查

> 上级入口：[00_INDEX.md](00_INDEX.md)
> 用途：拆分自旧 `_HANDOFF.md` 第 10 章。
> 权威 ID：架构、接口、环境、命令、错误
> 不存放：产物清单、关键代码原文、决策与未解决问题。
> 更新时间：2026-09-23
> 迁移说明：旧章节编号 (10)；完整映射见 `00_INDEX.md`。

## 10. 技术细节速查

### 架构与模块

- 内容：三层结构 —— ① `TaskBarDynamic.h/.cpp`：`ITMPlugin` 实现（单例，持有显示项列表 `items_`，负责注册与 `GetInfo()`）；② `DynamicData.h/.cpp`：`IPluginItem` 实现（模板 `DynamicData<T>`，把取数/格式化回调接到主程序接口）；③ `DynamicBase.h/.cpp`：滑动窗口 min/max 计算（模板，供图形归一化用），时间基准为 `std::chrono::steady_clock`。显示项由 `config.h/.cpp` 的声明式表定义，`PrimoCache.h/.cpp` 提供 PrimoCache 数据源与后台采样。
- 来源：[S-002]（旧会话 06:17 扫描报告）、[S-005]（README「分层结构」）、[S-006]
- 影响：新增显示项 = 改 `config.h/.cpp` 回调与配置 + 在 `TaskBarDynamic.cpp` 注册一行；不要在别处硬编码显示项。

### 数据模型与接口

- 内容：
  - `DynamicInfo<T>{ name_, id_, label_, value_sample_, draw_graph_, get_data_, set_data_ }` —— 显示项配置结构；`label_` 拼写已于 `3a653a8` 修正。
  - 显示项 ID（主程序用它保存勾选状态，**改名会导致用户设置失效**）：
    - 上传速度 `upload speed` → ID `mq72ZrBHN2`，标签 `↑:`，示例 `12.1MB/s`
    - 下载速度 `download speed` → ID `DE7etVv1nR`，标签 `↓:`，示例 `12.1MB/s`
    - CPU 温度 `cpu temperature` → ID `ST5sLiDY3f`，标签 `CPU:`，示例 `17°C`
    - PrimoCache 命中速度 `PC_HIT_SPD`，标签 `命中:`
    - PrimoCache 未中速度 `PC_MISS_SPD`，标签 `未中:`
    - PrimoCache 命中率 `PC_HIT_RATE`，标签 `命中率:`（`DynamicInfo<float>`）
  - `PrimoCacheCounters{ total_read, cached_read }`；`PrimoCacheSnapshot{ valid, rate_valid, hit_rate_valid, ... }`。速度只在 `valid && rate_valid` 时显示，命中率另要求 `hit_rate_valid`。
  - `ParseFirstNumber` 返回显式成功标志并检查单值 `uint64_t` 溢出；`ParseCounters` 必须同时找到可解析的 `Total Read` 与 `Cached Read`，多卷累加同样检查溢出。任一异常返回 false，不发布 0 值快照。
  - 抽象接口 `IPrimoCacheSource::Sample(...)`；实现 `RxpccSource`；单例 `PrimoCacheMonitor`（`Instance()` / `Probe()` / `Snapshot()` / `NotifyQueried()` / `RequestStop()`）；显示项类 `PrimoCacheItem<T>` 在被绘制时上报 `NotifyQueried()`。
  - DLL 只导出 `TMPluginGetInstance`（主程序用 `GetProcAddress` 查找）。
- 来源：[S-006]（config.cpp L126-L190、PrimoCache.h、README「显示项」表）、[S-002]
  - 影响：改 ID = 新增显示项（用户需重新勾选）；标签可能被主程序配置缓存覆盖（见 [Q-005]）。`DynamicBase::PutInValue()` 不再接收 `SYSTEMTIME`。

### 环境与配置

- 内容：
  - 开发环境：Windows 10/11；Visual Studio 2022（v143）或 VS2022 生成工具；C++17；`/MP` 多处理器编译；`/permissive-`；源码 UTF-8 with BOM（`PluginInterface.h` 保持 GBK 原样）。
  - 解决方案平台名：`x64` / `x86`（工程文件内为 `x64` / `Win32`）。
  - 运行环境：宿主 `C:\tool\TrafficMonitor\`；插件目录 `plugins\`；PrimoCache 4.4.0（`rxpcc.exe` 需管理员权限）；TrafficMonitor 需提权才能让 PrimoCache 显示项注册；当前源码版本为 `v1.2.2`。
  - 用户配置 `C:\tool\TrafficMonitor\config.ini`：**助手不得修改**（[C-002]）；相关键 `tbar_display_item`、`plugin_display_item`、`PC_MISS_SPD`（见 [Q-005]）。
  - 模型/中转环境：Codex 使用 `model_provider = "custom"`（opencode_go，`https://opencode.ai/zen/go/v1`），本机有 CC Switch 代理 `127.0.0.1:15721`；旧会话在该链路上失败（[R-001]）。
- 来源：[S-005]、[S-006]（`taskbardynamic.vcxproj`、`taskbardynamic.sln`）、[S-008]/[S-010]（config.toml 与代理）
- 影响：编译命令必须用解决方案平台名；PrimoCache 功能依赖提权；网络链路选择直接影响能否得到模型回复。

### 关键命令

命令：`cd D:\coding\taskbardynamic`
用途：切到仓库根目录
预期结果：后续命令基于仓库路径
注意事项：所有构建/发布命令都应在此基础上执行

命令：`msbuild taskbardynamic.sln /p:Configuration=Release /p:Platform=x64`
用途：生成 x64 Release（产物 `x64\Release\taskbardynamic-x64.dll`）
预期结果：0 warning / 0 error
注意事项：Debug 同理换 `Configuration`；x86 用 `/p:Platform=x86`（**不是 Win32**，[F-012]）。若当前进程同时存在 `PATH`/`Path`，需在只保留单一 `Path` 的干净子进程运行，否则报 `MSB6001`（[R-011]）。

命令：`msbuild taskbardynamic.sln /p:Configuration=Release /p:Platform=x86`
用途：生成 x86 Release（产物 `Release\taskbardynamic-x86.dll`）
预期结果：0 warning / 0 error
注意事项：见 [F-012]

命令：`rxpcc perf -a -u=b -s`
用途：读取 PrimoCache 累计计数（插件数据源）
预期结果：输出 `Volume #N:` 块，含 `Total Read` / `Cached Read` 等**累计**字节数（`-u=b` 为纯字节，`-s` 为按卷分组）
注意事项：必须管理员权限；实测单次调用约 44ms 墙钟 / 16ms CPU（[F-006]）

命令：`git status --short` / `git log --oneline -3` / `git diff`
用途：确认工作区与提交状态
预期结果：本轮已提交修复后工作区应干净；提交推送后 `master` 与 `origin/master` 同步
注意事项：提交前必须先清理测试产物并用显式路径 `git add <path>`（[C-004]/[C-005]）

命令：`gh release create <tag> <files> --title ... --notes-file ...`（历史用法见 [A-010]）
用途：发布 Release 并挂 x64/x86 附件
预期结果：Release 创建成功、附件可下载、SHA256 与本地构建一致
注意事项：**必须先有用户明确指令**（[C-003]/[C-007]）；发布前先做代码检查（[C-006]）；历史实现通过 `git credential fill` 取 GitHub token

命令：`sqlite3 "file:C:\Users\phrkn\.codex\logs_2.sqlite?mode=ro&immutable=1" "SELECT ts,level,target,feedback_log_body FROM logs WHERE ..."`
用途：回查错误原文与重试记录
预期结果：可查到 `codex_core::responses_retry`、`codex_http_client::client` 等记录
注意事项：用只读模式打开，避免与应用写锁冲突

### 已知错误

错误原文：
```
Request failed method=POST url=https://opencode.ai/zen/go/v1/responses error=error sending request for url (https://opencode.ai/zen/go/v1/responses)
```
复现条件：旧会话（~667,507 token 上下文）发起的任意回合
根本原因：超大请求体 + 跨境中转链路无法完成发送（[R-001]）
已尝试方案：直接重发（失败）；核对本地代理（代理正常，`GET /v1/models` 返回 200）
当前结论：该会话不可用，改用新会话 + 本文件

错误原文：
```
stream disconnected - retrying sampling request (1/5 in 214ms)... retries=1 max_retries=5 sampling_error=unexpected status 502 Bad Gateway: Unknown error, url: http://127.0.0.1:15721/v1/responses
```
复现条件：2026-09-23 17:12:44 起在旧会话发起回合（同一个失败序列的第一阶段）
根本原因：上游（opencode.ai 中转）502
已尝试方案：Codex 自动重试（后续 2/5、3/5…均失败）
当前结论：先从代理 502 退化为直连超时，链路整体不可用

错误原文：
```
error=exec_command failed: CreateProcess { message: "Rejected(\"This action was rejected due to unacceptable risk.\nReason: The broad `git add -A` will commit and push numerous untracked screenshots of the user's taskbar and machine state to `origin/master`; ...") }
```
复现条件：工作区含有未跟踪截图时执行 `git add -A`
根本原因：宽泛暂存会把敏感文件纳入提交
已尝试方案：改为按显式路径暂存
当前结论：永久禁止宽泛暂存（[C-004]）

错误原文：
```
error MSB6001: "CL.exe"的命令行开关无效。System.ArgumentException: 已添加项。字典中的关键字:"PATH"所添加的关键字:"Path"
```
复现条件：当前 Codex shell 的进程环境块同时包含 `PATH` 与 `Path`，MSBuild 创建 `ProcessStartInfo.EnvironmentVariables` 时发生大小写重复键冲突。
已尝试方案：在只保留一个 `Path`（合并并大小写不敏感去重后）的干净子进程中调用 `VsDevCmd.bat` 与 `msbuild.exe`，成功。
当前结论：构建验证使用干净子进程；该问题不影响源码本身（[R-011]）。
