# 状态与已确认事实

> 上级入口：[00_INDEX.md](00_INDEX.md)
> 用途：拆分自旧 `_HANDOFF.md` 第 4 章与第 8 章。
> 权威 ID：F / 当前进展与阻塞
> 不存放：决策、问题、技术细节、产物和行动清单。
> 更新时间：2026-09-23
> 迁移说明：旧章节编号 (4, 8)；完整映射见 `00_INDEX.md`。

## 4. 已确认事实

- [F-001] [P1] [已确认] 项目性质与技术栈：TrafficMonitor 插件 DLL，C++17，MSVC v143（VS2022），配置类型 DynamicLibrary，导出符号 `TMPluginGetInstance`，依赖上游插件接口 `PluginInterface.h`（插件 API 版本 6）。
  - 来源：[S-005]（README「环境要求/编译」）、[S-006]（`taskbardynamic.vcxproj`、`PluginInterface.h`）
  - 依据：README 表格与 vcxproj 配置
  - 是否可能变化：低

- [F-002] [P0] [已确认] 仓库与版本状态（2026-09-23 21:58 复核）：远端 `https://github.com/Real-king-Ph/taskbardynamic`，分支 `master`；用户恢复全部本地改动后，基线为 `c27dafc`，本轮依次提交 `517cdd7`（README 发布状态）、`3a653a8`（标签拼写 + 单调时钟）、`f47e36c`（速率未就绪显示 `--`）、`0c81e41`（README 字段名同步），并已推送 `c27dafc..0c81e41`。
  - 来源：`git log --oneline --decorate` / `git status --short --branch` / `git show --stat`（[S-013]/[S-014]）
  - 依据：四个提交均已本地验证并推送成功；HANDOFF 更新提交随后推送
  - 是否可能变化：是（推送与后续加固会产生新提交）

- [F-003] [P1] [已确认] Release 现状：GitHub 上有 3 个 Release —— `v1.1`、`v1.2`、`v1.2.1`，其中 `v1.2.1` 为 **Latest**（标签指向 `261213e`）；v1.2.1 附件已下载回本地做过 SHA256 比对：`taskbardynamic-x64.dll` 71,168 字节 / `03338161e4d25d39…`，`taskbardynamic-x86.dll` 64,512 字节 / `ff6d7402f6f30599…`。
  - 来源：[S-002]（旧会话 17:01 回复）；[S-011]（中间交接稿）
  - 依据：助手发布后逐字节校验记录
  - 是否可能变化：是

- [F-004] [P0] [已确认] 本机部署状态：`C:\tool\TrafficMonitor\plugins\taskbardynamic-x64.dll` 当前为 71,168 字节 / `03338161E4D25D39…`，与 v1.2.1 发布附件一致（即本机运行的是 v1.2.1 构建），文件时间 2026-09-23 16:23:16。
  - 来源：本会话命令 `Get-ChildItem` + `Get-FileHash`（[S-010]）
  - 依据：哈希与 [F-003] 相同
  - 是否可能变化：是（用户可能再部署）

- [F-005] [P0] [已确认] 显示项最终保留 3 个 PrimoCache 项（其余已删）：`PC_HIT_SPD` 标签 `命中:`、`PC_MISS_SPD` 标签 `未中:`、`PC_HIT_RATE` 标签 `命中率:`；插件总显示项 = 原 3 项（上传、下载、CPU 温度）+ 这 3 项（提权且检测到 PrimoCache 时才注册）。
  - 来源：[S-006]（`taskbardynamic/config.cpp` 中 `L"命中:"` / `L"未中:"` / `L"命中率:"`）、[S-002]（旧会话 13:22/13:48 回复）
  - 依据：源码字符串 + README「显示项」表
  - 是否可能变化：是（用户可能再调整标签/取舍）

- [F-006] [P1] [已确认] PrimoCache 取数方式：调用 `C:\Program Files\PrimoCache\rxpcc.exe`（版本 4.4.0，清单含 requireAdministrator，必须在管理员终端运行）的命令 `rxpcc perf -a -u=b -s`；输出为自 `Stat Start Time` 起的**累计计数**（英文标签、纯字节整数、百分比在括号内），因此速率与命中率必须靠两次采样求增量。
  - 来源：[S-002]（旧会话 15:24/15:43 回复）
  - 依据：真机输出样例
  - 是否可能变化：低（除非 PrimoCache 升级改输出格式）

- [F-007] [P1] [已确认] 关键采样常量（源码 `taskbardynamic/PrimoCache.h` 原文）：`kSampleIntervalMs = 5000`、`kHitRateWindowMs = 30000`、`kFailureThreshold = 2`、`kIdleTimeoutMs = 30000`、`kIdlePollMs = 1000`，并含 `static_assert(kSampleIntervalMs > 0, ...)`、`static_assert(kHitRateSamples >= 1, ...)`。
  - 来源：[S-006]（PrimoCache.h L14-L18、L106-L115）
  - 依据：源码逐行读取
  - 是否可能变化：否（除非再次改动代码）

- [F-008] [P1] [已确认] 命中率算法：最近 30 秒窗口内 `ΣΔ(Cached Read) / ΣΔ(Total Read) × 100%`，结果钳位 0~100，显示 1 位小数；整个窗口都没有读取时显示 `--`。
  - 来源：[S-002]（旧会话 16:44 回复、v1.2.1 发布说明）、[S-005]（README「运行时数据流」「更新日志」）
  - 依据：实现提交 `7b1c759`
  - 是否可能变化：是（B4-b 门槛未定，见 [Q-003]）

- [F-009] [P1] [已确认] 惰性采样已实现并真机验证：仅在显示项被绘制时采样；超过 `kIdleTimeoutMs = 30000` 没有任何显示项被查询即完全停止调用 rxpcc；重新显示后先采一次作基准，约 5 秒后给出速率。实测关闭显示项后 22 秒内 `rxpcc` 出现 **0 次**；开启时约 0.9% 时间 / 0.3% CPU。
  - 来源：[S-002]（旧会话 12:42/13:03 回复）
  - 依据：真机探测记录
  - 是否可能变化：否

- [F-010] [P1] [已确认] 异常显示规则（2026-09-23 更新，`N/A` 已全文移除）：采样正常且速率已就绪→正常数值；启动或恢复显示后的首个采样间隔（速率未就绪）→速度 `--`；单次失败（约 5 秒）→沿用上一次数值；连续 2 次及以上失败（≥10 秒）→显示 `--`；恢复成功→立刻恢复；窗口内无读取→命中率 `--`、速度 `0.00B/s`；未提权或未安装 PrimoCache→显示项根本不注册。
  - 来源：[S-002]（旧会话 08:54 回复）、[S-005]（README 异常显示规则表）
  - 依据：用户指令「2次以上失败才进行显示 --，不要显示 NA」（08:50）
  - 是否可能变化：否

- [F-011] [P1] [已确认] 质量验证基线（v1.2.1 发布前）：4 配置编译 `exit=0 issues=0`；MSVC `/analyze` 0 告警（修复前有 2 条 C28159）；解析器边界用例 7/7（真实输出/千位分隔符/多卷累加/空输出/缺字段/LF+制表符/计数相同）；滚动窗口 + 速率换算用例 15/15；真机平滑实测 0.0% → 9.4% → 21.6% → 40.2%。
  - 来源：[S-002]（旧会话 15:36、16:44 回复）
  - 依据：编译与测试记录
  - 是否可能变化：是（新改动需重跑）

- [F-012] [P1] [已确认] 构建平台名映射：**解决方案**平台名为 `x64` / `x86`，**工程文件（vcxproj）**里对应 `x64` / `Win32`。因此 `msbuild taskbardynamic.sln /p:Platform=x86` 正确，写成 `Win32` 会报 `MSB4126`。
  - 来源：[S-006]（`taskbardynamic.sln` 中 `Debug|x86 = Debug|Win32`）、[S-005]（README L74「解决方案里的 x86 对应工程文件中的 Win32」）
  - 依据：sln 文件原文
  - 是否可能变化：否

- [F-013] [P0] [已废弃] 交接时点曾有 2 个未提交修改：`README.md`、`taskbardynamic/TaskBarDynamic.cpp`；另有未跟踪目录 `_handoff/`。
  - 来源：命令 `git status --porcelain` / `git diff` / `Get-Item`（[S-007]/[S-010]）
  - 被替代关系：两处代码改动已在 `dcc53a5` / `6257311` 入库，`_handoff/` 已不存在；用户于本轮开始时恢复工作区，当前状态见 [F-021]

- [F-014] [P2] [已废弃] 曾推断上述 2 处未提交改动由用户手工完成，作者与提交意图待确认。
  - 推断依据：[S-009]（rollout 逐行扫描结果）、[S-010]（文件时间戳）
  - 被替代关系：相关改动已入库，当前工作区处置状态见 [F-021]/[Q-002]

- [F-015] [P1] [已确认] 旧会话（`01a0cb30-827c-7fe1-848f-e3b5d68dae2e`，标题「帮我扫描一下这个项目」）上下文规模已达 **667,507 / 950,000 token**（最后一次成功统计：2026-09-23 17:04:20，`last_token_usage.input_tokens = 667507`，`model_context_window = 950000`）。
  - 来源：[S-009]（rollout 中 `token_count` 事件）
  - 依据：rollout 原文
  - 是否可能变化：是（只会更大）

- [F-016] [P0] [已确认] 旧会话自 2026-09-23 17:12 起所有回合都在网络层失败、无任何助手输出；错误原文与重试行为见第 11 章。查询模型为 `deepseek-v4.1-flash`（provider `custom` = opencode_go 中转）。
  - 来源：[S-008]（`logs_2.sqlite` 中 `codex_core::responses_retry`、`codex_http_client::client` 记录）、[S-009]
  - 依据：日志原文
  - 是否可能变化：是（网络/中转恢复后可能又能跑，但仍受上下文规模限制）

- [F-017] [P1] [已确认] 本机 CC Switch 本地代理仍存活：`127.0.0.1:15721`（LISTENING，进程 `cc-switch.exe`，PID 22520），本会话实测 `GET http://127.0.0.1:15721/v1/models` 返回 **200**；同项目内 ~15 万 token 的会话（「你好呀」）同期能正常回复。
  - 来源：命令 `netstat -ano` / `Invoke-WebRequest` / 旧会话日志（[S-010]/[S-008]）
  - 依据：实测输出
  - 是否可能变化：是

- [F-018] [P1] [已确认] 仓库标签现状：本地已有 `v1.1`、`v1.2`、`v1.2.1`；本轮将新增 `v1.2.2`。历史 `release` 标签与 `v1.0` Release 已被删除（那条「v1.0 附件实为 v1.1 DLL」的遗留问题以删除 v1.0 Release 收尾）。
  - 来源：命令 `git tag --list`（[S-007]）、[S-002]（旧会话 15:10/15:11 回复）
  - 依据：命令输出 + 当时确认
  - 是否可能变化：是

- [F-019] [P2] [已确认] 未知/无关内容：同项目下另有两条与本工作无关的会话——「调整作息与褪黑素」（健康咨询）与「回应问候」（「你好呀」，助手已验证 ~150k token 可用）；它们不含本项目技术决策，可忽略。
  - 来源：[S-009]（线程列表与内容）、[S-002]
  - 依据：读会话内容
  - 是否可能变化：否

- [F-020] [P2] [已确认] 环境事实：工作区 `D:\coding\taskbardynamic`；宿主程序实际运行为 `C:\tool\TrafficMonitor\`（非 Downloads 副本）；插件目录存在历史备份文件：`taskbardynamic.20250617-022452.dll.bak`(34,816B)、`taskbardynamic.v1.1.dll.bak`(30,720B)、`taskbardynamic.primo-test.dll.bak`(72,192B)、`taskbardynamic.prev-test.dll.bak`(71,680B)。
  - 来源：[S-010]（目录列举与哈希）、[S-002]（旧会话 06:44 定位宿主回复）
  - 依据：命令输出
  - 是否可能变化：是

- [F-021] [P0] [已确认] 2026-09-23 21:58 复核：用户已恢复此前全部修改，本轮开始前工作区干净；随后三项修复及一项文档字段同步按显式路径提交，并推送 `c27dafc..0c81e41`；未触碰 `C:\tool\TrafficMonitor\config.ini`，未部署、未发布。
  - 来源：用户当前指令 + `git status --short --branch` / `git log --oneline`（[S-013]/[S-014]）
  - 依据：本地 `master` 在 `c27dafc` 基础上领先 `origin/master` 三个修复提交
  - 是否可能变化：是（HANDOFF 提交及后续推送会更新状态）

- [F-022] [P1] [已确认] 本轮验证：Debug/Release × x64/x86 四个配置全部完整重建成功，均为 `0 warning / 0 error`；Release x64 另以 `RunCodeAnalysis=true` + NativeRecommendedRules 运行 MSVC `/analyze`，0 告警。构建使用只保留单一 `Path` 的干净子进程绕过 [R-011]。
  - 来源：本轮 MSBuild/静态分析命令输出（[S-014]）
  - 依据：四个构建退出码均为 0
  - 是否可能变化：是（新增代码后需重新验证）

- [F-023] [P1] [已确认] 2026-09-24 的 `ParseCounters` 加固已在工作区完成但尚未提交：`ParseFirstNumber` 返回成功标志并检查 `uint64_t` 溢出；`ParseCounters` 要求 `Total Read` 与 `Cached Read` 同时存在、二者都能解析，且多卷累加不溢出。15 个针对性解析测试全部通过。
  - 来源：用户当前明确指令 + 本轮源码/测试输出（[S-015]）
  - 依据：Debug/Release × x64/x86 重新构建 0 warning/0 error；Release x64 `/analyze` 0 告警；临时测试 15/15
  - 是否可能变化：是（提交与否取决于用户授权）

- [F-024] [P0] [已确认] `v1.2.2` 发布候选已完成预发布检查：插件 `TMI_VERSION` 与 README 更新为 `v1.2.2`；Debug/Release × x64/x86 全量构建 0 warning/0 error；Release x64 `/analyze` 0 告警；解析器 15/15；最终附件为 x64 71,168 B / `01FD0257…`、x86 66,048 B / `E87B3AD6…`。
  - 来源：用户当前发布指令 + 本轮构建/测试输出（[S-016]）
  - 依据：发布前固定流程 [C-006] 已执行且无问题
  - 是否可能变化：是（提交哈希与 Release URL 将在完成后补记）
## 8. 当前进展与当前状态

- 最后一次完成的步骤：用户恢复工作区后，依次完成并提交：
  1) `517cdd7`：README 的 v1.2.1 发布状态改为实际已发布事实；不再出现“v1.2.1 未发布/尚未发布”。
  2) `3a653a8`：`DynamicInfo::label_` 修正拼写；绘图窗口改由 `std::chrono::steady_clock` 驱动，移除 `SYSTEMTIME` 依赖。
  3) `f47e36c`：`PrimoCacheSnapshot::rate_valid` 传递到显示层；启动/恢复显示尚未完成两次采样时，速度显示 `--`。

- 最后已知可用状态（可用 = 能正常显示与回复）：
  - 本机 TrafficMonitor 仍运行已发布 v1.2.1 DLL；本轮修复尚未部署，这是预期状态——[F-004]。
  - 本轮源码四个构建配置与 Release x64 静态分析全部通过——[F-022]。
  - 同项目下小上下文会话可用于继续工作——[F-017]。

- 当前正在进行的步骤：`ParseCounters` 修复与验证已完成，等待用户决定是否提交；HANDOFF 已同步为未提交状态。

- 当前阻塞点：
  1) B4-b 命中率最小样本门槛未拍板（[Q-003]）。
  2) 命中率真实时间窗口、进程树终止、输出上限与启动时延仍未加固（[Q-011]）。
  3) 当前 shell 同时存在 `PATH` 与 `Path` 时会触发 MSBuild `MSB6001`；干净子进程可绕过（[R-011]）。

- 已知失败路径（不要再走）：
  - 向旧会话发送任何消息（含摘要/提示）→ 仍会卡死（[R-001]，本会话已实测一次）。
  - `git add -A` 提交 → 被拒绝并可能泄露截图（[R-002]）。
  - 直接覆盖被进程占用的 DLL → 失败（[R-005]）。
  - 用带缓冲的文件读写去验证 PrimoCache 命中率 → 数据全 0（[R-007]）。
  - 在重复 `Path`/`PATH` 的当前 shell 直接跑 MSBuild → `MSB6001`（[R-011]）。

- 下一步最安全动作：
  1) 用户决定是否提交 `ParseCounters` 修复及本轮 HANDOFF 更新。
  2) 用户若继续优化，从 [Q-011] 选择窗口/进程/启动加固项；未获明确同意不提交、不发布、不部署。
  3) 不要碰 `C:\tool\TrafficMonitor\config.ini`。

- 来源：以上综合 [S-002]、[S-008]、[S-010]、[S-013]、[S-014]
