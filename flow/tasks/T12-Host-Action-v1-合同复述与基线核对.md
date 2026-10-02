# T12 · Host Action v1 合同复述与当前基线核对

> 本任务卡是「合同复述 + 事实核对 + 落位状态」记录，不是方案建议。本节点只改 `flow/`，不代表本轮新增了代码。
>
> 合同原文见 `flow/tasks/T02-Host-Action-v1-固定兼容协议.md`。本卡不重抄全文，只在必要处引用并给出**当前基线的实现证据**。

## 操作前文件状态

- 分支与提交：`main`，`7d5f15749e8671e9c14d49f37d0b8ebc6ddc7721`。
- `git status --porcelain` 在本节点写入前（仅 7 项，全部来自更早的节点）：
  - `M flow/进展.md`（2026-10-01 的构建/测试与修订记录）
  - `M host_test/CMakeLists.txt`、`M host_test/audio_packet_wire_tests.cpp`、`M host_test/config_status_tests.cpp`、`M host_test/firmware_source_contract_tests.cpp`、`M host_test/ima_adpcm_decoder_tests.cpp`、`M host_test/sound_asset_store_tests.cpp`（2026-10-01 的宿主测试跨平台修订）
- 关键文件 SHA256 前 16 位（操作前）：`host_action_protocol.h` `2aa1371cd94f0108`、`host_action_protocol.cpp` `2d415b173f0873a9`、`status_hid_protocol.h` `d3eb112ae4d6bce4`、`status_hid_protocol.cpp` `c26574363924f4d5`、`keymap.h` `1e22731ead6d0def`、`keymap.cpp` `3a232cef672b8e32`、`config_payload.cpp` `0a49b6ec58bd7863`、`usb_hid.cpp` `47528a5dfc4e74e5`、`ble_hid.cpp` `c5993febfa0671a2`、`host_action_protocol_tests.cpp` `cc7069d2822c5ca2`。
- 结论：操作前没有本轮引入的非 `flow/` 变更；上述 `host_test/` 改动属于先前节点，本节点在其上增量记录，不覆盖、不冒充。

## 逐项复述与自检

### 1. 配置层保存内容

- 复述：配置层保存完整 `host_action:<canonical-lowercase-uuid>`。
- 自检证据：`components/keyboard/src/config_payload.cpp:554` 赋值 `parsed.host_action = *named;` —— 保存的是**含前缀的完整字符串**，未做剥离；`:549` 先调用 `is_canonical_host_action_value()`，失败即返回 `ConfigParseStatus::UnknownAction`（fail closed，不进入 Keymap）。
- 结论：**一致**。

### 2. UUID 规范与拒绝语义

- 复述：大写、长度错误、连字符位置错误或含非法字符的非规范 UUID 直接拒绝并 fail closed，**不自动转小写**；不增加 UUID version 或 nil UUID 限制。
- 自检证据：`components/keyboard/src/host_action_protocol.cpp:19-36`。长度必须精确等于 `12 + 36`；前缀逐字符比对；连字符只允许在索引 `8/13/18/23`（等价于从 1 计数的第 9/14/19/24 位）；其余位置只接受 `0-9`、`a-f`（`is_lowercase_hex()`）。全文件**没有**任何大小写转换调用，因此不存在自动小写；也**没有**任何 version 位或 nil 判定。
- 反向证据：`host_test/host_action_protocol_tests.cpp:15-16` 明确断言 `00000000-0000-0000-0000-000000000000` 与 `ffffffff-...` 均**合法**，即无 nil / version 限制；`:26-29` 覆盖大写、少一位、连字符错位、含 `g` 四类拒绝；测试名 `rejects_noncanonical_uuid_and_leaves_output_untouched` 同时证明拒绝时不改动输出（fail closed 而非部分写入）。
- 结论：**一致**。

### 3. 运行传输字节合同

- 复述：Report ID `0x11`、kind `0x05`、chunk index `0`、total chunks `1`、data length `36`，数据区只放不带前缀的 UUID ASCII。
- 自检证据：`components/keyboard/include/keyboard/host_action_protocol.h:13-18` 定义 `0x11 / 0x05 / 0 / 1 / 36 / 63 / 4`；`components/keyboard/src/host_action_protocol.cpp:45-49` 依次写 `payload[0..3] = 0x05 / 0 / 1 / 36`；`:50-52` 用 `action.substr(kHostActionPrefixLen)` 去掉前缀后拷到 `payload.begin() + 4`，即数据落在 `[4..39]`。
- 结论：**一致**。

### 4. 容器与余量

- 复述：`payload[40..62]` 只是现有容器余量；63 字节是现有 App Command 消息容器，不是 Host Action 数据长度。
- 自检证据：容器 63 字节在 `host_action_protocol.h:17` 与 `components/keyboard/include/keyboard/status_hid_protocol.h:21`（`kStatusAppCommandPayloadLen = 63`）两处交叉确认；`usb_hid.cpp:40`、`ble_hid.cpp:65` 同样固定 63。已用区间为 `[0..39]`（4 字节头 + 36 字节 UUID），故 `[40..62]` 恰为 23 字节未使用余量。Host Action 的 data length 固定 36，与容器 63 无混淆。
- 结论：**一致**。

### 5. 编号占用

- 复述：`0x04` 已用于状态响应，Host Action 不得复用。
- 自检证据：现有 App Command kind 完整占用为 —— `0x01` 固定文本（`fixed_text_protocol.h:10`）、`0x02` 热键（`usb_hid.cpp:36`）、`0x03` 配置回执（`usb_hid.cpp:37`）、`0x04` 状态响应（`status_hid_protocol.h:20`）、`0x05` Host Action v1（`host_action_protocol.h:14`）。**`0x05` 无第二处语义占用**；源码中其他 `0x05` 字面量均为 HID 报告描述符字节（Usage Page / Report Count 等），不属于命令种类编号空间。
- Report ID：`0x01` 键盘、`0x02` 鼠标、`0x10` 配置、`0x11` App Command（Host Action 与固定文本共用，按 kind 区分）、`0x12` 状态响应、`0x13` 状态请求。`usb_hid.cpp:46-...` 与 `ble_hid.cpp:68-...` 用 `static_assert` 把 `kReportIdAppCommand == kHostActionV1ReportId` 固化为编译期约束。
- 结论：**一致，无冲突**。

### 6. 按键事件语义

- 复述：按下发送一次，松开不发送。
- 自检证据：`components/keyboard/src/keymap.cpp:82-87` —— `ActionKind::HostAction` 在 `InputPhase::Released` 或值非规范时返回空事件 `{}`，仅在 `Pressed` 返回 `{FirmwareEventKind::HostAction, action.host_action}`。宿主测试 `host_test/host_action_key_bindings_tests.cpp:46` `every_main_key_preserves_and_emits_one_host_action_per_press_cycle()` 断言按下产生事件、松开为 `None`，且 `host_action_events == 1`；`:61` 断言保留 `host_action:` 前缀。
- 结论：**一致**。

### 7. 运输一致性与禁止双发

- 复述：USB 与 BLE 使用相同内容，沿用现有单通道选择，不能双发。
- 自检证据：两侧调用**同一个**共享编码器 `ai_keyboard::encode_host_action_v1()` —— `usb_hid.cpp:1451` 与 `ble_hid.cpp:1961`，内容由构造决定，不可能漂移。选路 `components/keyboard/src/transport_routing.cpp:5-9` 恒返回 `UsbFirst`；`main/app_main.cpp:1417` 在 USB owner 存在时**直接 return**，USB 拒收时明确抑制 BLE 兜底，不存在双发路径。`host_test/transport_routing_tests.cpp` 对 `ble_connected` 为 true/false 两种输入都断言 `UsbFirst`。
- 结论：**一致**。

### 8. 固件身份边界

- 复述：固件不保存应用路径、名称或 Bundle ID；真实「UUID → 本机应用」映射只保存在 App 本地；示例 UUID 只用于宿主测试。
- 自检证据：`components/` 与 `main/` 全量检索 `bundle|app_path|app_name|application_id|bundle_id` 无相关字段（唯一命中 `speaker_assets_supervisor.cpp:35` 的 `bundle_sha256` 指语音资源包摘要，与应用 Bundle ID 无关）。示例 UUID 仅出现在 `host_test/config_payload_tests.cpp` 与 `host_test/host_action_protocol_tests.cpp`。
- 结论：**一致**。

### 9. 版本演进

- 复述：v1 字段已冻结；不兼容变化必须使用新版本和新能力声明。
- 自检证据：`flow/decisions.md` 已有同口径决策记录；本卡不新增不同解释。
- 结论：**一致**。

## 与 T02 的差异：落位状态已推进

T02 写于提交 `34087cd`，当时记录的「当前公开基线尚未实现 Host Action／尚未出现 kind `0x05`」在**当时成立**，但对当前基线 `7d5f157` 已不再成立。逐项落位状态：

| 改动层 | T02 记录 | 当前 `7d5f157` 实测 |
|---|---|---|
| 配置解析与持久化 | 待实现 | 已实现：`config_payload.cpp:545-557` 解析并保留完整前缀；`main/app_main.cpp:3063` 经 `NvsConfigStore::save_config_and_host_platform()` 持久化整份配置 JSON |
| Keymap 与事件 | 待实现 | 已实现：`keymap.cpp:82-87`；`host_action_key_bindings_tests.cpp` 覆盖 |
| 共享协议编码 | 待实现 | 已实现：`host_action_protocol.{h,cpp}`（`0x11/0x05/0/1/36`） |
| USB／BLE 适配 | 未开始 | 已实现：`usb_hid.cpp:1449-1461`、`ble_hid.cpp:1959-1971`，共用同一编码器 |
| 宿主测试 | 待实现 | 已实现：`host_action_protocol_tests.cpp`（22 处断言）、`host_action_key_bindings_tests.cpp`、`transport_routing_tests.cpp` |
| 第 08 步能力声明 + BLE 512 字节 | 留到第 08 步 | 已实现：`components/keyboard/include/keyboard/config_status.h:16` 与 `config_status.cpp:243/508/562` 声明 `"host_action_v1":true`；`config_status.h:11` `kConfigStatusGattSafeLen = 512`。`flow/plan.md` 已记录第 08 步完成与 512/512 字节回归 |

**因此本合同与现有保留编号、消息容器、项目约定均无实际冲突**，且合同已被当前基线完整落实；本节点按授权写入 `flow/`，不停止、不改代码。

## 本合同不承担的范围

- 不修改、读取或设计其他电脑端工程；App 侧固定为已经准备好的 EasyInput 0.1.26。
- 不在固件中保存或推断应用路径、名称、Bundle ID，也不复制 App 本地的 UUID 映射。
- 不把测试示例 UUID 变成真实或默认应用映射。
- 不增加 UUID version 或 nil UUID 限制，不自动规范化非规范输入。
- 不新增 Report ID、第二种 Host Action 消息、释放消息、分片方式或 USB／BLE 双发；不静默改写 v1。
- 不修改 GPIO、BOOT、GPIO8 共享供电、分区、USB／BLE 设备身份或硬件生命周期。
- 本节点不改代码、不运行测试或构建、不烧录；合同核对与落位确认不等于新增实现、测试通过或实板通过。

## 本节点验收自检

- [x] 操作前分支、提交、`git status` 与关键文件哈希已先记录。
- [x] 合同已逐项复述并逐项给出当前基线的文件与行号证据。
- [x] 已确认 `0x05` 无第二处语义占用，`0x04` 专用于状态响应。
- [x] 已确认 63 字节为 App Command 容器、数据长度为 36、`[40..62]` 为余量。
- [x] 已确认规范校验 fail closed、无自动小写、无 UUID version／nil 限制。
- [x] 已确认按下一次／松开不发、USB 与 BLE 共用同一编码器、禁止双发。
- [x] 已确认固件不保存应用路径／名称／Bundle ID，示例 UUID 仅存在于宿主测试。
- [x] 已记录全部改动层落位状态与第 08 步能力声明、BLE 512 字节检查。
- [x] 已明确本合同与现有编号、容器、约定无冲突，故按授权直接写入。
- [x] 本节点未修改任何非 `flow/` 文件，未运行测试、构建或烧录。
