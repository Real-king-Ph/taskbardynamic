# 关键产物与资源

> 上级入口：[00_INDEX.md](00_INDEX.md)
> 用途：拆分自旧 `_HANDOFF.md` 第 9 章。
> 权威 ID：A
> 不存放：技术细节、代码原文、风险和行动清单。
> 更新时间：2026-09-23
> 迁移说明：旧章节编号 (9)；完整映射见 `00_INDEX.md`。

## 9. 关键产物与资源

- [A-001] [P0] [已确认] 代码仓库根目录
  - 绝对路径：`D:\coding\taskbardynamic`
  - 用途：全部源码、工程、README、构建输出
  - 当前状态：存在；`v1.2.2` 发布提交 `4e0c900dc04a5b084d49bb575f519b9ae3048c52` 已推送，发布记录提交随后加入
  - 版本 / commit / hash：`4e0c900dc04a5b084d49bb575f519b9ae3048c52`
  - 如何使用：作为所有命令的工作目录（`cd D:\coding\taskbardynamic`）
  - 来源：[S-013]/[S-014]

- [A-002] [P0] [已确认] 项目文档 `README.md`
  - 绝对路径：`D:\coding\taskbardynamic\README.md`
  - 用途：功能说明、编译、安装、显示项、工作原理、更新日志
  - 当前状态：已增加 `v1.2.2` 更新日志；`v1.2.1` 发布说明保留但不再声称 Latest；异常规则包含“速率未就绪显示 `--`”
  - 版本 / commit / hash：`4e0c900`
  - 如何使用：改动后与源码一起提交；文档口径必须与实现/发布状态一致
  - 来源：[S-005]、[S-007]

- [A-003] [P0] [已确认] PrimoCache 模块头文件 `taskbardynamic/PrimoCache.h`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\PrimoCache.h`
  - 用途：定义 `IPrimoCacheSource` 抽象、`PrimoCacheCounters`、`PrimoCacheSnapshot`、采样器单例 `PrimoCacheMonitor` 及全部采样常量
  - 当前状态：6,979 字节；速率就绪语义仍是 `valid + rate_valid`
  - 版本 / commit / hash：基线 `261213e`；显示门控由 `f47e36c` 更新
  - 如何使用：修改采样间隔/窗口/失败阈值/空闲超时都在此文件的常量处改
  - 来源：[S-006]

- [A-004] [P0] [已确认] PrimoCache 实现 `taskbardynamic/PrimoCache.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\PrimoCache.cpp`
  - 用途：`RxpccSource`（定位/权限检测/进程调用/文本解析）+ 后台采样线程 + 增量计算
  - 当前状态：18,283 字节；新增解析失败标志与 `uint64_t` 溢出检查，纳入 `v1.2.2` 候选
  - 版本 / commit / hash：`4e0c900`
  - 如何使用：解析与采样行为改动在此；改后必须跑解析器用例
  - 来源：[S-006]

- [A-005] [P0] [已确认] 显示项配置 `taskbardynamic/config.h` / `config.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\config.h`、`...\config.cpp`
  - 用途：声明式显示项表（名称、ID、标签、示例文本、是否绘图、取数回调、格式化回调）
  - 当前状态：`config.cpp` 5,086 字节（含 `L"命中:"` / `L"未中:"` / `L"命中率:"`）；速度显示同时检查 `valid` 与 `rate_valid`
  - 版本 / commit / hash：`f47e36c`
  - 如何使用：新增显示项时改 config.h/config.cpp 并在 `TaskBarDynamic.cpp` 注册
  - 来源：[S-006]

- [A-006] [P0] [已确认] 插件主体 `taskbardynamic/TaskBarDynamic.h` / `.cpp`、`dllmain.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\TaskBarDynamic.cpp`（3,510 字节）、`TaskBarDynamic.h`、`dllmain.cpp`
  - 用途：`ITMPlugin` 实现、单例、显示项注册、`GetInfo()`、`GetTooltipInfo()`、DLL 入口
  - 当前状态：`DataRequired()` 不再获取本地 `SYSTEMTIME`，只触发各显示项用单调时钟推进窗口；PrimoCache 悬浮提示使用缓存文本并在查询时调用 `NotifyQueried()`
  - 版本 / commit / hash：`TMI_VERSION = v1.2.2`（`4e0c900`）
  - 如何使用：版本号、插件元信息在此文件
  - 来源：[S-006]、[S-007]

- [A-007] [P1] [已确认] 构建产物（本地，含未发布 PrimoCache 悬浮提示）
  - 绝对路径与当前哈希（2026-09-30 实测）：
    - `D:\coding\taskbardynamic\x64\Release\taskbardynamic-x64.dll` — 75,264 B / `A2CAB605E803E0A6FB0E8D9FB1B2D34354B16CA07E7C1296140E8CF65D633DEB`
    - `D:\coding\taskbardynamic\Release\taskbardynamic-x86.dll` — 70,144 B / `22EF931ED1F16C6AB8F987B1B243AC6F3827635D678DD823A038838BF4C80644`
    - `D:\coding\taskbardynamic\x64\Debug\taskbardynamic-x64.dll` — 778,240 B / `BC75E85071AC534420F19D670936BFE5A9E4F91C7237247976D44B598E2DFD7B`
    - `D:\coding\taskbardynamic\Debug\taskbardynamic-x86.dll` — 582,144 B / `F0EA1F6EAFB595C97D78049E2D34FFD515D305C978DAC63CB45CB56FEBAECE69`
  - 用途：本地验证；以后发布或部署时必须基于明确提交重新构建
  - 当前状态：x64/x86 Release 构建均已作为 `v1.2.3` 发布；x64 正式版尚未部署到本机，本机运行的是先前的 `5A570D18` 开发构建。发布附件见 [A-010]，本机实际生效文件见 [A-008]
  - 如何使用：部署/发布前先确认改动已提交，再重编译并比对 SHA256
  - 来源：[S-010]、[S-007]

- [A-008] [P0] [已确认] 运行时部署位置与备份
  - 绝对路径：`C:\tool\TrafficMonitor\plugins\`
    - `taskbardynamic-x64.dll` — 74,752 B / `5A570D18…` / 2026-09-30 17:04:18（PrimoCache 悬浮提示开发构建，当前生效；不是 v1.2.3 正式附件）
    - `taskbardynamic.v1.2.2-20260930-170809.dll.bak` — 71,168 B / `01FD0257…`（本次部署前备份，可回滚到 v1.2.2 发布版）
    - `taskbardynamic.v1.2.1-20260924-113018.dll.bak` — 71,168 B / `03338161…`（更早的 v1.2.1 备份）
    - `taskbardynamic.v1.1.dll.bak` — 30,720 B / `EFC095724FBB5DF8…`
    - `taskbardynamic.primo-test.dll.bak` — 72,192 B / `589BBA62C31C26C7…`
    - `taskbardynamic.prev-test.dll.bak` — 71,680 B / `CB356C5989926D39…`
    - `taskbardynamic.20250617-022452.dll.bak` — 34,816 B / `981041D7628216C7…`（2025-06-17 原始版本）
  - 用途：运行中的插件与回滚依据
  - 当前状态：生效文件 = PrimoCache 悬浮提示开发构建（`TMI_VERSION = v1.2.2`）；TrafficMonitor PID 113940 已启动；正式 `v1.2.3` x64 尚未部署
  - 如何使用：替换前先退出主程序（[R-005]）；不要删备份
  - 来源：[S-010]

- [A-009] [P1] [已确认] PrimoCache 命令行工具
  - 绝对路径：`C:\Program Files\PrimoCache\rxpcc.exe`（GUI：`FancyCcV.exe`）
  - 用途：插件唯一数据源；命令 `rxpcc perf -a -u=b -s`
  - 当前状态：版本 4.4.0，需管理员权限
  - 如何使用：管理员终端执行；输出为累计计数，需两次采样求增量
  - 来源：[S-002]（旧会话 15:24/15:43 回复）

- [A-010] [P1] [已确认] GitHub Releases（发布产物）
  - 绝对路径/URL：
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2.3`（Latest；标签/提交 `7c51fa9`；附件 x64 `A2CAB605…` 75,264B、x86 `22EF931E…` 70,144B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2.2`（历史版本；目标 `4e0c900`；附件 x64 `01FD0257…` 71,168B、x86 `E87B3AD6…` 66,048B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2.1`（附件 x64 `03338161…` 71,168B、x86 `ff6d7402…` 64,512B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2`（附件为修复前构建：x64 `5c8ca5bc…` 71,680B、x86 `d1f72363…` 65,536B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.1`（x64 `efc09572…` 30,720B、x86 `e96fb785…` 26,112B）
  - 用途：用户下载安装；附件与本地构建做哈希比对
  - 当前状态：共 4 个 Release，`v1.2.2` 为 Latest
  - 如何使用：发布用 `gh release create` / `gh release upload`（需要 GitHub 凭据；历史实现通过 `git credential fill` 取得 token）
  - 来源：[S-002]（旧会话 17:01 回复）

- [A-011] [P0] [已确认] 本文件（交接入口）
  - 绝对路径：`D:\coding\taskbardynamic\CONTEXT_HANDOFF\00_INDEX.md`
  - 用途：新对话继续工作的唯一入口；按协议选择性读取分域文件
  - 当前状态：项目级拆分结构有效
  - 如何使用：新对话先读 `AGENTS.md` → `PROTOCOL.md` → `00_INDEX.md`
  - 来源：[S-003]/[S-012]

- [A-012] [P2] [已确认] 旧会话原始记录（回查用）
  - 绝对路径：
    - `C:\Users\phrkn\.codex\sessions\2026\09\23\rollout-2026-09-23T06-15-49-01a0cb30-827c-7fe1-848f-e3b5d68dae2e.jsonl`（9.9MB，早期部分）
    - `C:\Users\phrkn\.codex\sessions\2026\09\23\rollout-2026-09-23T08-41-51-01a0cb30-827c-7fe1-848f-e3b5d68dae2e_01a0cbb6-3668-76d3-96e9-1161095ba232.jsonl`（12.5MB，后期部分，含 token_count 与失败回合）
  - 用途：需要原文/精确数值时回查
  - 当前状态：存在；文件可能被应用进程占用（写入中）
  - 如何使用：按行读取 JSONL；注意行内 message 文本可能很长
  - 来源：[S-009]

- [A-013] [P2] [已确认] 应用日志库（错误原文来源）
  - 绝对路径：`C:\Users\phrkn\.codex\logs_2.sqlite`（表 `logs`，字段 `ts/level/target/feedback_log_body/thread_id`）
  - 用途：查历史错误、重试、HTTP 请求失败原因
  - 当前状态：存在（约 2.8 万条）
  - 如何使用：`sqlite3 "file:...?mode=ro&immutable=1" "SELECT ..."`
  - 来源：[S-008]

- [A-014] [P2] [已废弃] `D:\coding\taskbardynamic\NEXT-STEPS.md`
  - 用途：曾用于把「需要用户手动的任务」集中交给用户（[U-004]）
  - 当前状态：已按 [C-005] 删除（随 26 个测试文件一起清理）
  - 如何使用：不要重新创建散落文件；同类内容写进本文件的第 13 章或询问用户
  - 来源：[S-002]（旧会话 13:48 回复）

- [A-015] [P2] [已废弃] 测试截图（`_r_pass1..6.png`、`_r_compare.png`、`_f_cold/hot*.png`、`_demo_plugin_zoom.png` 等，约 21 张）
  - 用途：曾用于真机数值/平滑曲线验证
  - 当前状态：已全部删除（用户要求清理，[C-005]）
  - 如何使用：需要复现验证时重新截图，但**不要提交**（[R-002]）
  - 来源：[S-002]、[S-007]

- [A-016] [P2] [已完成] 本会话早先创建的中间交接稿
  - 绝对路径：`D:\coding\taskbardynamic\_handoff\2026-09-23-traffic-monitor.md`
  - 用途：曾用于在旧会话外承载交接摘要
  - 当前状态：已不存在；内容已并入项目级 HANDOFF
  - 如何使用：不要重新创建散落交接文件
  - 来源：[S-011]

- [A-017] [P1] [已确认] 许可证文件
  - 绝对路径：`D:\coding\taskbardynamic\LICENSE`
  - 用途：声明本项目继承 TrafficMonitor 原作者 Zhong Yang 的许可类型
  - 当前状态：已新增，许可证原文保持上游内容
  - 来源：用户原话（本会话“版权类型继承原作者。”）+ TrafficMonitor 官方仓库 `LICENSE`

- [A-018] [P1] [已确认] 绘图滑动窗口实现 `taskbardynamic/DynamicBase.h` / `.cpp`、`DynamicData.h` / `.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\DynamicBase.h/.cpp`、`DynamicData.h/.cpp`
  - 用途：按 10/5/2 分钟窗口维护 min/max 并归一化资源占用图；把显示项数据/文本回调接到主机接口
  - 当前状态：字段已修正为 `label_`；窗口时间基准为 `std::chrono::steady_clock`，不再使用 `SYSTEMTIME`
  - 版本 / commit / hash：`3a653a8`
  - 来源：本轮源码修改与构建验证（[S-013]/[S-014]）
