# 决策日志（decisions）

> 这里只追加适合公开、对后续协作确有价值的过程决策。

## 2026-08-25 · 完整基线、课程起点与产品专属能力分层
- 背景：完整公共固件已经具备 Host Action v1 和 BLE 配置持久化修复；课程需要让学员按节点实现 Host Action，同时又不能退回到缺少 BLE 修复的旧固件。产品固件另有 OTA、签名和发布流程，不属于本课程目标。
- 决定：Maker `main` 维护完整公共固件基线；`course/host-action-v1-starter` 从同一公共底座中只移除 Host Action 的实现与答案型测试，继续保留 BLE 修复和所有无关公共能力。两者分别用 `course-host-action-v1-complete-v1` 与 `course-host-action-v1-start-v1` 固定课程版本。产品专属 OTA、签名、发布和部署工具不进入 Maker 的任何分支。
- 同步原则：公共固件以行为合同和经过审计的文件差异同步，不做整仓覆盖。仓库专属文档、测试和课程差异可以不同；Maker 的公开隐私规则属于共同安全基线，不能为了机械一致恢复敏感日志。
- 原因：这样既让学员面对真实、最新的 USB／BLE 底座，又只实现一件明确的课程目标；同时把 OTA 的分区迁移、可信发布和恢复风险隔离为独立产品能力。

## 2026-08-22 · Host Action v1 能力只属于状态 JSON
- 背景：Host Action v1 的发送协议已经冻结并接入；第 08 步需要让 App 能从配置状态判断固件能力，同时维持 BLE 状态 512 字节硬上限。
- 决定：`"host_action_v1": true` 只作为各配置状态 `capabilities` 对象中的唯一布尔字段，由完整、紧凑、speaker probe、确认、battery 和共享 fallback 路径声明；不进入 Report ID `0x11` 的 Host Action payload。speaker probe 只把专用 firmware 内容预算收紧为 16 字节并保留全部指标；含 recent power-cycle 的状态超限时，先保留 current power 并只省略四个 cycle 字段。BLE 最终附加编码必须在完整 UTF-8 JSON 不超过 512 字节时才生效。
- 原因：能力发现与动作发送属于两个不同职责；共用状态构造和最终 BLE 编码可避免各发布路径漂移，同时让 0x04 状态响应、0x05 Host Action 和 512 字节上限继续各守原边界。

## 2026-08-22 · 冻结 Host Action v1 兼容协议
- 背景：已经准备好的 EasyInput App 0.1.26 按既有合同向固件同步带真实 UUID 的按键配置；当前 Maker 公开基线尚未实现 Host Action，也没有 App Command kind `0x05`。缺少实现是后续节点要补的能力，不是合同冲突。
- 决定：配置层保存完整的 `host_action:<canonical-lowercase-uuid>`；运行传输层固定使用 Report ID `0x11`、kind `0x05`、chunk index `0`、total chunks `1`、data length `36`，数据区只放去掉 `host_action:` 前缀后的 36 字节 UUID ASCII。63 字节是既有 App Command 消息容器，4 字节头与 36 字节数据占用 payload `[0..39]`，`[40..62]` 仅为容器余量；kind `0x04` 继续专用于状态响应。
- 约束：UUID 只接受规范小写形式；大写、长度错误、连字符位置错误或非法字符必须直接拒绝并 fail closed，不自动转小写，也不增加 UUID version 或 nil UUID 限制。按下发送一次、松开不发送；USB 与 BLE 内容相同并沿用现有单通道选择，不得双发。固件不保存应用路径、名称或 Bundle ID；真实“UUID → 本机应用”映射只存在 EasyInput App 本地，固定示例 UUID 只可用于宿主测试。
- 演进：完整实现必须覆盖配置解析与持久化、Keymap 与事件、共享协议编码、USB／BLE 适配和宿主测试；`"host_action_v1": true` 能力声明及 BLE 状态不超过 512 字节的检查留到第 08 步。v1 字段已冻结，未来不兼容变化必须使用新版本与新能力声明，不能静默改写 v1。
- 原因：现有 App Command kind `0x01`—`0x04`、Report ID `0x11`、63 字节容器及 USB 优先的单通道路由能够容纳这份固定合同，没有实际编号、容量或项目边界冲突。

## 2026-08-22 · README 公众号二维码保留来源仓库独立许可
- 背景：Maker README 缺少 `project-flow-cy` 已公开的公众号二维码，且两个仓库的根许可证不同。
- 决定：从 `project-flow-cy` 提交 `7d3ad181f65e034b7b45cff916f15cfd8fc7db74` 原样复制二维码到 `assets/readme/wechat-qr.jpg`，并保留 GPL-3.0-or-later 许可证文本、固定来源和文件 SHA-256；Maker 其他材料的许可证保持不变。
- 原因：避免外部视觉资产被 Maker 根许可证静默覆盖，让公开分发时的来源、版权和再分发要求可追溯。

## 2026-08-22 · 编码器文字选择使用原生键盘语义
- 背景：EasyInput App 已使用 `cursor + text_caret_select` 配置编码器文字选择；旧 Maker 固件缺少该动作，导致整份配置以 `unknown_action` 被拒绝。
- 决定：规范动作名固定为 `text_caret_select`，旧名称 `mouse_drag_select` 解析为同一动作；该动作只允许位于编码器按压位并配合 `cursor` 模式。短按切换选择状态，旋转发送完整的标准键盘 `Shift+ArrowRight/Left`，3 秒长按继续进入既有系统配置。默认仍为 `scroll_axis_toggle`。
- 原因：复用标准键盘报告可以兼容现有 USB/BLE 输入通道和真实按键状态，不需要新增鼠标拖拽、App Command 或宿主权限协议；严格槽位和模式校验可以继续保持配置 fail-closed。

## 2026-10-02 · Host Action v1 合同与当前基线核对结论
- 背景：`flow/tasks/T02` 在提交 `34087cd` 时写就，记录的是「基线尚未实现 Host Action／尚未出现 kind `0x05`」的时点事实；当前 `main` 已推进到 `7d5f157`，需要确认冻结合同是否与现有保留编号、消息容器或项目约定发生冲突。
- 决定：确认**无冲突**，合同原样保留、不做任何改写。`0x05` 在命令种类编号空间内唯一（`0x01` 固定文本／`0x02` 热键／`0x03` 配置回执／`0x04` 状态响应／`0x05` Host Action v1）；63 字节是 App Command 容器而非 Host Action 数据长度，`0x11` 下 4 字节头加 36 字节 UUID 占 `[0..39]`，`[40..62]` 恰为余量。落位状态改写记录在 `flow/tasks/T12`，`T02` 仅追加带日期的状态更新、原文保留。
- 原因：合同字段已在冻结点固定，核对只应产出「一致／冲突」结论，不应借核对之机改动 v1；把「合同文本」与「合同落位状态」分开记录，可以避免历史节点被后续进展悄悄改写，也便于课程分支按节点复现。
- 边界：本轮只写 `flow/`，未改代码、未运行测试或构建、未烧录；合同核对完成不等于功能新增或验证通过。

## 2026-10-02 · Host Action v1 纯逻辑层已完成，本轮只补事件层覆盖
- 背景：后续节点被要求「先补宿主测试、再做最小实现」。核对 `flow/tasks/T02` 与 `T12` 记录的冻结合同后确认，`7d5f157` 的纯逻辑层（`host_action_protocol`、`config_payload`、`keymap`）**已经完整实现**该合同，协议层／配置层／状态层／Keymap 层的既有测试也已覆盖契约的全部条目。
- 决定：不为「凑一次实现改动」而改写已正确的代码。本轮只补齐唯一一处真实缺口 —— `keymap.cpp` 中事件级 canonical 守卫（`!is_canonical_host_action_value(...)` 返回空事件）此前从未被任何测试执行过，因为既有用例全部经由配置解析进入、被前置层拦住；新增 `host_test/keymap_tests.cpp` 中绕过配置层的程序化用例覆盖它。
- 原因：测试的价值在于能否发现回归。实现已存在时，直接新增测试只会「初次即通过」，无法证明其有效性，因此采用**变异验证**：临时移除守卫 → 新测试在 `keymap_tests.cpp:155` 断言失败 → 还原并校验哈希与操作前逐字节一致 → 复跑通过。这样既留下失败到通过的证据，又保证纯逻辑层实现零净改动。
- 附带记录：`shutil.copy2` 保留旧 mtime 会导致 ninja 不重编、误判仍失败，还原后需 `os.utime` 更新时间戳再构建。
- 边界：本轮未接 USB／BLE、未构建固件、未烧录；传输层接线仍按合同留待后续节点，不提前修改。

<!-- 新记录模板：
## YYYY-MM-DD · <决策标题>
- 背景：
- 决定：
- 原因：
-->
