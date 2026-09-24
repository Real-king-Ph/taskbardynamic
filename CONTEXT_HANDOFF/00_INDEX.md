# Context Index

> 唯一入口：本文件
> 上级协议：[PROTOCOL.md](PROTOCOL.md)
> 适用范围：`D:\coding\taskbardynamic` 仓库的全部开发、发布、部署与上下文交接工作
> 更新时间：2026-09-24 10:44
> 完整性状态：完整；原 81 KB 单文件已无损拆分，未保留重复归档

## 快速状态

- 主目标：[G-001] 维护并持续完善 Traffic Monitor 插件 `taskbardynamic`。
- 当前交接目标：[G-006] 已完成拆分与项目级持久化；后续以本文件为入口。
- 最后已发布版本：`v1.2.1`（`261213e`）；当前远端基线为 `2f02745`。`v1.2.2` 候选版本已完成全部发布前验证，正在准备提交和 Release。
- 许可类型：继承 TrafficMonitor 原作者 Zhong Yang 的 `"Anti 996" License Version 1.0 (Draft)`；完整条款见仓库 `LICENSE`。
- 当前不可用入口：旧会话 `01a0cb30-...` 已不可继续，禁止再向它发送消息（[R-001]）。
- 当前阻塞：
  - B4-b 命中率最小样本门槛仍未拍板（[Q-003]）。
  - 命中率窗口时间语义、进程树与输出上限加固仍未实施（[Q-011]），但不阻塞 `v1.2.2`。
  - 当前 Codex shell 的 `Path`/`PATH` 重复会触发 MSBuild `MSB6001`；已验证用只保留单一 `Path` 的干净子进程可正常构建（[R-011]）。
- 下一步最安全动作：提交并推送 `v1.2.2`，创建 GitHub Release 并校验 x64/x86 附件；用户已明确授权本轮提交、推送和发布。
- 本地环境约束：不要修改 `C:\tool\TrafficMonitor\config.ini`；需要 UAC/手动批准的操作必须停下等用户（[C-002]/[C-003]）。

## P0 快速摘要

- [C-001] 显示项是否显示由用户自己控制，插件不得代改勾选状态。
- [C-002] 不要修改 `C:\tool\TrafficMonitor\config.ini`。
- [C-003] 需要用户批准或 UAC 的操作必须等待用户；用户休息期间不得执行。
- [C-004] 禁止 `git add -A` 等宽泛暂存，必须按显式路径暂存。
- [C-005] 提交前清理测试截图、`NEXT-STEPS.md` 等不需要的文件。
- [C-006] 发布前必须完成代码/漏洞检查；发现问题则修复并重复检查。
- [C-007] 用户说“先别执行”时不得提交、发布或实现。
- [C-009] 已废弃：旧的“只写单个 `_HANDOFF.md`”约束被 [C-010] 取代。
- [C-010] HANDOFF 必须保持项目级持久化拆分；`00_INDEX.md` 是唯一入口，同一信息只能有一个权威位置。

## 阅读顺序

1. 每次任务开始：先读 [PROTOCOL.md](PROTOCOL.md) 和本文件。
2. 目标和约束：[01_GOALS_CONSTRAINTS.md](01_GOALS_CONSTRAINTS.md)。
3. 当前状态：[02_STATE_FACTS.md](02_STATE_FACTS.md)。
4. 决策、问题和冲突：[03_DECISIONS_OPEN_ISSUES.md](03_DECISIONS_OPEN_ISSUES.md)。
5. 按任务需要读取：
   - 产物：[04_ARTIFACTS.md](04_ARTIFACTS.md)
   - 技术细节：[05_TECH_DETAILS.md](05_TECH_DETAILS.md)
   - 精确代码/原文：[06_CODE_SNIPPETS.md](06_CODE_SNIPPETS.md)
   - 风险与错误：[07_RISKS_ERRORS.md](07_RISKS_ERRORS.md)
   - 时间线与来源：[08_TIMELINE_SOURCES.md](08_TIMELINE_SOURCES.md)
6. 执行前读取：[09_NEXT_ACTIONS.md](09_NEXT_ACTIONS.md)。
7. 维护或审计时读取：[10_AUDIT.md](10_AUDIT.md)。

## 文件地图

| 文件 | 内容 | 何时读取 |
|---|---|---|
| [00_INDEX.md](00_INDEX.md) | 唯一入口、快速状态、文件地图 | 每次开始 |
| [PROTOCOL.md](PROTOCOL.md) | 更新、拆分、索引、审计规则 | 每次开始及维护时 |
| [01_GOALS_CONSTRAINTS.md](01_GOALS_CONSTRAINTS.md) | G / C / U | 开始任务前 |
| [02_STATE_FACTS.md](02_STATE_FACTS.md) | F、当前进展、阻塞 | 判断下一步前 |
| [03_DECISIONS_OPEN_ISSUES.md](03_DECISIONS_OPEN_ISSUES.md) | D / Q / X | 修改方案或拍板前 |
| [04_ARTIFACTS.md](04_ARTIFACTS.md) | A、路径、哈希、部署产物 | 操作文件或发布前 |
| [05_TECH_DETAILS.md](05_TECH_DETAILS.md) | 架构、接口、命令、错误 | 改代码或调试前 |
| [06_CODE_SNIPPETS.md](06_CODE_SNIPPETS.md) | 必须精确复现的原文/代码 | 需要精确内容时 |
| [07_RISKS_ERRORS.md](07_RISKS_ERRORS.md) | R、坑、失败路径 | 调试或高风险操作前 |
| [08_TIMELINE_SOURCES.md](08_TIMELINE_SOURCES.md) | 时间线、S 来源索引 | 回查历史时 |
| [09_NEXT_ACTIONS.md](09_NEXT_ACTIONS.md) | T、优先级、验收条件 | 执行任务前 |
| [10_AUDIT.md](10_AUDIT.md) | 完整性与迁移审计 | 维护后或怀疑缺失时 |

## ID 权威位置

| ID 类型 | 权威文件 |
|---|---|
| G、C、U | 01_GOALS_CONSTRAINTS.md |
| F、当前状态 | 02_STATE_FACTS.md |
| D、Q、X | 03_DECISIONS_OPEN_ISSUES.md |
| A | 04_ARTIFACTS.md |
| 技术细节 | 05_TECH_DETAILS.md |
| P-xxx 精确片段 | 06_CODE_SNIPPETS.md |
| R | 07_RISKS_ERRORS.md |
| 时间线、S | 08_TIMELINE_SOURCES.md |
| T | 09_NEXT_ACTIONS.md |
| 审计 | 10_AUDIT.md |

## 旧章节映射

| 旧 `_HANDOFF.md` 章节 | 新位置 |
|---|---|
| 0 使用说明 | 00_INDEX.md / PROTOCOL.md |
| 1~3 目标、约束、偏好 | 01_GOALS_CONSTRAINTS.md |
| 4、8 事实、状态与进展 | 02_STATE_FACTS.md |
| 5、6、15 决策、问题、冲突 | 03_DECISIONS_OPEN_ISSUES.md |
| 9 产物与资源 | 04_ARTIFACTS.md |
| 10 技术细节 | 05_TECH_DETAILS.md |
| 11 原文与代码片段 | 06_CODE_SNIPPETS.md |
| 7 风险与失败经验 | 07_RISKS_ERRORS.md |
| 12、14 时间线与来源 | 08_TIMELINE_SOURCES.md |
| 13 下一步行动 | 09_NEXT_ACTIONS.md |
| 16 压缩审计 | 10_AUDIT.md |

## 更新规则

- 新事实、决策、问题、产物或行动必须写入其权威文件，并在必要时追加时间线。
- 不得在多个文件中复制同一段正文；跨文件引用使用稳定 ID。
- 被替代条目不得删除，必须标记 `[已废弃]` 并注明替代项。
- 冲突必须保留双方来源，写入 `03_DECISIONS_OPEN_ISSUES.md`。
- 更新完成后同步本文件的快速状态、文件地图和更新时间。
- 完整规则与拆分阈值见 [PROTOCOL.md](PROTOCOL.md)。
