# Project Instructions

## Mandatory Context Handoff

本项目使用持久化、拆分式上下文交接系统。根目录：

`D:\coding\taskbardynamic\CONTEXT_HANDOFF\`

### 每个任务开始时必须执行

1. 先完整读取：
   - `D:\coding\taskbardynamic\CONTEXT_HANDOFF\PROTOCOL.md`
   - `D:\coding\taskbardynamic\CONTEXT_HANDOFF\00_INDEX.md`
2. 根据 `00_INDEX.md` 的阅读顺序，只加载与当前任务相关的 HANDOFF 文件。
3. 将 P0 约束、当前目标和有效决策视为强制要求。
4. 不得把 `[推断]`、`[待确认]` 或 `[冲突]` 内容当作已确认事实。
5. 如果缺少信息，先按来源 ID 回查，再决定是否向用户提问。

### 工作过程中必须执行

1. 当目标、约束、事实、决策、状态、产物、风险或下一步发生变化时，更新权威 HANDOFF 文件。
2. 同一信息只能有一个权威存放位置。
3. 其他文件只能通过稳定 ID 或链接引用，不得复制全文。
4. 不得静默删除已废弃或存在冲突的信息；必须标注并说明替代关系。
5. 不得修改与当前任务无关的业务文件。

### 每个任务结束前必须执行

1. 更新所有受影响的 HANDOFF 文件。
2. 更新 `00_INDEX.md` 的当前状态、阻塞、下一步和更新时间。
3. 必要时更新时间线和 `10_AUDIT.md`。
4. 按照 `PROTOCOL.md` 执行完整性检查。
5. 最终报告中说明修改了哪些 HANDOFF 文件。

### 强制规则

- 如果 `CONTEXT_HANDOFF` 不存在，按照协议创建。
- 如果文件超过拆分阈值，按协议拆分并同步索引。
- 不得遗漏 P0 约束、未解决问题、最后有效进展和下一步行动。
- 未获用户明确同意，不提交、不发布、不部署。
- 禁止 `git add -A`；只按显式路径暂存。
- 不修改 `C:\tool\TrafficMonitor\config.ini`；UAC/手动批准操作必须等待用户。
