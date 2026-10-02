# T06 · Host Action v1 Vendor HID 发送接入

> 本任务严格落实 T02 冻结合同，复用 T03 共享编码和 T04 八键事件，不重新设计 Host Action v1。

- **目标**：把按下产生的 Host Action 事件接入现有 Report ID `0x11` Vendor HID App Command 发送路径，并保持 USB 优先、否则 BLE、失败不跨通道补发。
- **允许范围**：事件分发、现有 USB／BLE App Command 适配、宿主测试与根级 `flow/`。
- **禁止范围**：不增加 `"host_action_v1": true`；不修改 HID 描述符、BLE GATT、设备身份、GPIO、BOOT、电源或 Flash 分区；不烧录。
- **状态**：代码接入、宿主验证与 ESP-IDF 5.5.5 默认构建完成；App、传输抓包和实板尚未验证。

## 操作前状态

- 分支与提交：`main`，`34087cd40d24d23579da0357973ebc1a37e7ce7c`。
- 操作前工作区包含 T03 纯逻辑、T04 八键测试和既有 `flow/` 增量；先保存跟踪差异名、未跟踪文件清单及本轮相关文件 SHA-256，用于区分本轮改动。
- 操作前 `main/` 没有差异；共享 `host_action_protocol.h` 和路由实现的前后 SHA-256 不变。

## 实际修改文件

### 运行接入

- `main/app_main.cpp`
- `main/platform/usb_hid.cpp`
- `main/platform/ble_hid.cpp`

### 宿主测试

- `host_test/host_action_protocol_tests.cpp`
- `host_test/transport_routing_tests.cpp`
- `host_test/firmware_source_contract_tests.cpp`

### 项目记录

- `flow/plan.md`
- `flow/进展.md`
- 本任务卡

## 调用链与共享边界

### 公共上游

`handle_input_event()` → `event_for_action()` → `FirmwareEventKind::HostAction` → `dispatch_firmware_event()` → `route_for_firmware_event()`。

### USB

`dispatch_firmware_event()` 读取当前 USB endpoint epoch → `UsbHidTransport::send_firmware_event_for_epoch()` → `encode_host_action_v1()` → `send_app_command_report()` → `queue_app_command_report()` → `push_app_command_report_locked()` → `try_send_app_command_report()` → `tud_hid_report()`。

### BLE

只有没有 USB owner 时，`dispatch_firmware_event()` 调用 `BleHidTransport::send_firmware_event()` → `send_firmware_event_for_owner()` → `encode_host_action_v1()` → `send_app_command_report()` → `send_input_report()` → `HidReportQueue`／`BleInputScheduler` → `easy_input_hidd_dev_input_set_for_owner()`。

USB 与 BLE 适配都包含共享 `host_action_protocol.h`，各自只调用一次 `encode_host_action_v1()`；适配文件没有新增 `kAppCommandKindHostAction`、`0x05` 或 `36` 常量。两侧还用 `static_assert` 把现有 App Command Report ID、payload 长度和头长度锁定到共享合同。

## 固定 payload 证据

| 字段 | 证据 |
|---|---|
| Report ID | 共享编码固定 `0x11`；两侧适配静态断言现有 App Command ID 与其相同 |
| payload `[0]` | 共享编码固定 kind `0x05` |
| payload `[1]` | chunk index `0` |
| payload `[2]` | total chunks `1` |
| payload `[3]` | data length `36`，没有改成 48 |
| payload `[4..39]` | 仅 36 字节 UUID ASCII，不含配置前缀 |
| payload `[40..62]` | 共享报告和两侧既有零初始化 App Command 容器保持为零 |

`host_action_protocol_tests` 同时静态确认状态响应 kind 仍为 `0x04` 且与 Host Action `0x05` 不同。

## 单通道与失败不迁移证据

- `route_for_firmware_event()` 对 Host Action 在 BLE 已连接和未连接两种输入下都返回现有 `UsbFirst`。
- `dispatch_firmware_event()` 在 USB epoch 非零时只调用 `send_firmware_event_for_epoch()`，无论入队成功或失败都立即返回；失败日志明确说明 BLE fallback 被抑制。
- 仅当 USB epoch 为零时才调用 BLE；不存在同时对两个适配器调用的执行分支。
- BLE 继续通过 owner token 和现有分类队列发送；USB 继续绑定 endpoint epoch。没有增加跨 owner 重试或 App 端去重。

## 测试与构建

### 失败到通过

- 红灯：先增加宿主合同后，定向 4 项中 3 项通过；`firmware_source_contract_tests` 在找不到 `dispatch_firmware_event()` 的 Host Action 分支处失败。
- 绿灯：补齐三个最小 switch 分支后，扩大后的定向 8/8 通过，0 失败。

### 命令与结果

```bash
cmake --build build-host --target host_action_protocol_tests host_action_key_bindings_tests keymap_tests transport_routing_tests firmware_source_contract_tests hid_report_queue_tests usb_hid_endpoint_arbiter_tests ble_input_scheduler_tests -j4
ctest --test-dir build-host --output-on-failure -R '^(host_action_protocol_tests|host_action_key_bindings_tests|keymap_tests|transport_routing_tests|firmware_source_contract_tests|hid_report_queue_tests|usb_hid_endpoint_arbiter_tests|ble_input_scheduler_tests)$'
cmake --build build-host -j4
ctest --test-dir build-host --output-on-failure
idf.py build
```

- Host Action 与传输定向测试：8/8 通过，0 失败。
- 完整宿主测试：58/58 通过，0 失败。
- ESP-IDF 环境：v5.5.5，target `esp32s3`。
- 默认构建：`Project build complete`；应用镜像 `0x190200` 字节，3 MiB App 分区剩余 `0x16fe00` 字节（48%）。
- `git diff --check` 通过。

## 原有 App Command／HID 回归

- 固定文字分片与 BLE 背压、配置回执、状态响应、HID 报告队列、USB endpoint 仲裁、BLE 输入调度继续通过完整宿主测试。
- 复制、粘贴、普通快捷键、固定文字、标准键盘按下／松开和 KEY1—KEY8 Host Action 事件继续通过。
- ESP-IDF 默认构建真实编译 `app_main.cpp`、`usb_hid.cpp` 与 `ble_hid.cpp`；这证明平台代码可生成固件，不等于已经发送到 App 或实板工作正常。

## 禁止范围差异证据

- 本轮没有修改 HID report descriptor 数组或 BLE GATT service／characteristic 定义。
- 没有新增能力声明；对精确 JSON 字段的代码检索无命中。
- `sdkconfig.defaults`、分区表、板级引脚、GPIO 输入实现没有差异。
- USB VID／PID／版本和 BLE 名称、外观、HID 版本没有差异。
- 没有执行端口识别、烧录、串口或实板操作。

## 仍未验证项

- EasyInput App 0.1.26 同步真机 UUID 尚未验证。
- 未在 USB-only、BLE-only 或双连接条件下抓取实际 Vendor HID 报告。
- 未在实板制造已选 USB／BLE 队列失败，因而“失败不补发”目前由宿主源码合同和现有控制流证明，不是实板观察。
- 未烧录、未读串口、未验证真实按键到 App 动作的端到端结果。
- 第 08 步 `"host_action_v1": true` 和 BLE 状态 512 字节预算仍未实现或验证。

## 2026-10-02 状态更新：发送接入在当前基线上复核对齐

> 本节由后续节点追加，上文是 `34087cd` 时点的真实记录。其中「完整宿主测试 58/58」「第 08 步 `"host_action_v1": true` 和 BLE 状态 512 字节预算仍未实现或验证」对当前基线 `7d5f157` 已不成立（完整套件现为 60 项；第 08 步能力声明与 512 字节预算已实现）。**原文保留不改。**

- 复核方式：逐行重读 `main/app_main.cpp` 的 `dispatch_firmware_event()`、`main/platform/usb_hid.cpp` 与 `main/platform/ble_hid.cpp` 的 Host Action 分支，与上文调用链逐一对照。
- 复核结论：调用链、共享编码边界与单通道路由**与上文完全一致，无需改动**。USB 侧 `UsbHidTransport::send_firmware_event_for_epoch()` 与 BLE 侧 `BleHidTransport::send_firmware_event_for_owner()` 的 Host Action 分支结构相同，各自**只调用一次** `ai_keyboard::encode_host_action_v1()`，随后把 `report.payload[0..3]` 与 `report.payload.data() + kHostActionV1HeaderLen` 交给各自既有的 `send_app_command_report()`；两侧都没有 `kAppCommandKindHostAction`、`0x05`、`36`、`48` 字面量，只用共享的 `ai_keyboard::kHostActionV1*` 常量与 `static_assert` 锁定编号。
- 本轮唯一改动：`host_test/host_action_key_bindings_tests.cpp` 新增 `encoded_host_action_matches_the_frozen_payload_for_every_main_key()`。此前「配置 → Keymap → 事件」与「字符串 → 冻结字节」分别有测试，但**没有任何用例把两者拼起来**；新用例对 8 个主键各跑一遍按键周期，把事件值喂进**两侧传输共用的同一个编码器**，断言 Report ID `0x11`、kind `0x05`、chunk index `0`、total chunks `1`、data length `36`、数据区仅 36 字节 UUID 且不含前缀、`[40..62]` 全零。
- 测试结果：定向 8/8 通过（含 `firmware_source_contract_tests`、`transport_routing_tests`、`hid_report_queue_tests`、`usb_hid_endpoint_arbiter_tests`、`ble_input_scheduler_tests`）；完整宿主 **60/60 通过、0 失败**，编译错误 0，`ctest` 退出码 0。
- 禁止范围差异：`git diff --stat -- main` 无输出；`config_status.h`／`config_status.cpp`／`sdkconfig.defaults`／`partitions.csv`／`board_pins.h` 均无差异。本轮**没有增加也没有修改** `"host_action_v1": true`（它由第 08 步先行节点引入，现状保持原样），未改 HID 描述符、BLE GATT、设备身份、GPIO、BOOT、电源或 Flash 分区，未烧录。
- 仍未验证项：与上文一致 —— App 0.1.26 真机 UUID 同步、USB-only／BLE-only／双连接下的实际 Vendor HID 抓包、实板队列失败下的「不补发」观察、烧录与实板端到端，均未验证。本轮同样**未运行 ESP-IDF 构建**。
