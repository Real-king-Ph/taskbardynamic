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
  - 当前状态：存在；`master` HEAD=`261213e`，含 2 个未提交修改 + 未跟踪 `_handoff/`
  - 版本 / commit / hash：`261213ecc791d155344449d001b417e410ae7410`
  - 如何使用：作为所有命令的工作目录（`cd D:\coding\taskbardynamic`）
  - 来源：[S-007]

- [A-002] [P0] [已确认] 项目文档 `README.md`
  - 绝对路径：`D:\coding\taskbardynamic\README.md`
  - 用途：功能说明、编译、安装、显示项、工作原理、更新日志
  - 当前状态：24,218 字节；**有未提交修改**；L414/L416 仍写「v1.2.1（未发布）」/「尚未发布」（待修，[Q-001]）
  - 版本 / commit / hash：工作区版本（未提交）
  - 如何使用：改动后与源码一起提交；文档口径必须与实现/发布状态一致
  - 来源：[S-005]、[S-007]

- [A-003] [P0] [已确认] PrimoCache 模块头文件 `taskbardynamic/PrimoCache.h`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\PrimoCache.h`
  - 用途：定义 `IPrimoCacheSource` 抽象、`PrimoCacheCounters`、`PrimoCacheSnapshot`、采样器单例 `PrimoCacheMonitor` 及全部采样常量
  - 当前状态：6,979 字节，已提交（`261213e`）；mtime 2026-09-23 16:18:08
  - 版本 / commit / hash：属于 `261213e`
  - 如何使用：修改采样间隔/窗口/失败阈值/空闲超时都在此文件的常量处改
  - 来源：[S-006]

- [A-004] [P0] [已确认] PrimoCache 实现 `taskbardynamic/PrimoCache.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\PrimoCache.cpp`
  - 用途：`RxpccSource`（定位/权限检测/进程调用/文本解析）+ 后台采样线程 + 增量计算
  - 当前状态：17,339 字节，已提交；mtime 16:18:09
  - 版本 / commit / hash：属于 `261213e`
  - 如何使用：解析与采样行为改动在此；改后必须跑解析器用例
  - 来源：[S-006]

- [A-005] [P0] [已确认] 显示项配置 `taskbardynamic/config.h` / `config.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\config.h`、`...\config.cpp`
  - 用途：声明式显示项表（名称、ID、标签、示例文本、是否绘图、取数回调、格式化回调）
  - 当前状态：`config.cpp` 4,913 字节（含 `L"命中:"` / `L"未中:"` / `L"命中率:"`），已提交
  - 版本 / commit / hash：属于 `261213e`
  - 如何使用：新增显示项时改 config.h/config.cpp 并在 `TaskBarDynamic.cpp` 注册
  - 来源：[S-006]

- [A-006] [P0] [已确认] 插件主体 `taskbardynamic/TaskBarDynamic.h` / `.cpp`、`dllmain.cpp`
  - 绝对路径：`D:\coding\taskbardynamic\taskbardynamic\TaskBarDynamic.cpp`（2,327 字节，**有未提交修改**）、`TaskBarDynamic.h`、`dllmain.cpp`
  - 用途：`ITMPlugin` 实现、单例、显示项注册、`GetInfo()`（名称/描述/作者/版权/版本）、DLL 入口
  - 当前状态：`TaskBarDynamic.cpp` 含未提交改动（`TMI_DESCRIPTION` 扩充 PrimoCache 说明、`TMI_COPYRIGHT` 由 `cpy` 改为 `Copyright (C) 2025-2026 Real-King-ph`）
  - 版本 / commit / hash：`TMI_VERSION` 当前为 `v1.2.1`
  - 如何使用：版本号、插件元信息在此文件
  - 来源：[S-006]、[S-007]

- [A-007] [P1] [已确认] 构建产物（本地）
  - 绝对路径与当前哈希（本会话实测）：
    - `D:\coding\taskbardynamic\x64\Release\taskbardynamic-x64.dll` — 71,168 B / `AE62E841955E2D80…` / mtime 2026-09-23 18:40:31
    - `D:\coding\taskbardynamic\Release\taskbardynamic-x86.dll` — 65,024 B / `7BF0DD33CF9F1E97…` / mtime 18:39:51
    - `D:\coding\taskbardynamic\x64\Debug\taskbardynamic-x64.dll` — 774,144 B / mtime 18:39:42
    - `D:\coding\taskbardynamic\Debug\taskbardynamic-x86.dll` — 578,560 B / mtime 18:39:48
  - 用途：部署到 TrafficMonitor `plugins`、上传 Release 附件
  - 当前状态：**与已发布 v1.2.1 附件不一致**（因为包含未提交改动，见 [R-009]）
  - 如何使用：部署/发布前先确认改动已提交，再重编译并比对 SHA256
  - 来源：[S-010]、[S-007]

- [A-008] [P0] [已确认] 运行时部署位置与备份
  - 绝对路径：`C:\tool\TrafficMonitor\plugins\`
    - `taskbardynamic-x64.dll` — 71,168 B / `03338161E4D25D39…` / 2026-09-23 16:23:16（= v1.2.1 发布附件，当前生效）
    - `taskbardynamic.v1.1.dll.bak` — 30,720 B / `EFC095724FBB5DF8…`
    - `taskbardynamic.primo-test.dll.bak` — 72,192 B / `589BBA62C31C26C7…`
    - `taskbardynamic.prev-test.dll.bak` — 71,680 B / `CB356C5989926D39…`
    - `taskbardynamic.20250617-022452.dll.bak` — 34,816 B / `981041D7628216C7…`（2025-06-17 原始版本）
  - 用途：运行中的插件与回滚依据
  - 当前状态：生效版本 = v1.2.1
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
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2.1`（Latest；附件 x64 `03338161…` 71,168B、x86 `ff6d7402…` 64,512B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.2`（附件为修复前构建：x64 `5c8ca5bc…` 71,680B、x86 `d1f72363…` 65,536B）
    - `https://github.com/Real-king-Ph/taskbardynamic/releases/tag/v1.1`（x64 `efc09572…` 30,720B、x86 `e96fb785…` 26,112B）
  - 用途：用户下载安装；附件与本地构建做哈希比对
  - 当前状态：共 3 个 Release，v1.2.1 为 Latest
  - 如何使用：发布用 `gh release create` / `gh release upload`（需要 GitHub 凭据；历史实现通过 `git credential fill` 取得 token）
  - 来源：[S-002]（旧会话 17:01 回复）

- [A-011] [P0] [已确认] 本文件（交接入口）
  - 绝对路径：`D:\coding\taskbardynamic\CONTEXT\_HANDOFF.md`
  - 用途：新对话继续工作的唯一入口
  - 当前状态：正在分批生成（本会话）
  - 如何使用：新对话第一步完整读取本文件
  - 来源：[S-003]

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

- [A-016] [P2] [待确认] 本会话早先创建的中间交接稿
  - 绝对路径：`D:\coding\taskbardynamic\_handoff\2026-09-23-traffic-monitor.md`
  - 用途：曾用于在旧会话外承载交接摘要
  - 当前状态：存在，未跟踪（git status 显示 `?? _handoff/`）；内容已并入本文件
  - 如何使用：按 [Q-010] 处置（默认删除，避免散落文件）
  - 来源：[S-011]
