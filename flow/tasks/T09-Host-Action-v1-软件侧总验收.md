# T09 · Host Action v1 软件侧总验收

## 本节点范围

- 重新配置宿主测试并先读取 CTest 实际清单，再执行清单中的全部测试。
- 在宿主测试全部通过后，激活项目锁定的 ESP-IDF v5.5.5，对默认 `v2`／`esp32s3` 目标执行构建。
- 本节点不识别设备、不读取串口、不烧录、不运行 EasyInput App，也不声称真机功能已经通过。
- 不修改 Report ID `0x11`、kind `0x04`／`0x05`、63 字节 App Command 容器、HID 描述符、BLE GATT、设备身份、GPIO、BOOT、GPIO8 电源控制或 Flash 分区。

## 操作前状态与证据口径

- 操作前工作区已包含 T03／T04／T06／T08 的 Host Action 实现、测试和 `flow/` 记录；本节点保留这些差异，不撤销或重复实现。
- 上一轮完整宿主测试 `59/59` 与能力状态最大 `512/512` 字节只作为已有交接读取，不计作本节点重新运行结果。
- 课程起点宿主测试为 56 个；T03 新增 `host_action_protocol_tests` 后为 57 个，T04 新增 `host_action_key_bindings_tests` 后为 58 个，T06 只扩充既有测试目标而未新增 CTest 目标，T08 新增 `host_action_capability_status_tests` 后为 59 个。

## 实际命令与退出结果

本机普通 shell 未直接暴露 `cmake`／`ctest`，因此复用已安装的 ESP-IDF 工具目录中的 CMake 3.30.2；公开记录只保留工具版本与命令，不写入本机 SDK 绝对路径。

```bash
"$MAKER_CMAKE_BIN" -S host_test -B build-host -DCMAKE_BUILD_TYPE=Debug
# exit 0

"$MAKER_CTEST_BIN" --test-dir build-host -N
# exit 0

"$MAKER_CMAKE_BIN" --build build-host -j4
# exit 0

"$MAKER_CTEST_BIN" --test-dir build-host --output-on-failure
# exit 0

source "$MAKER_IDF_PATH/export.sh" >/dev/null 2>&1
idf.py --version
# ESP-IDF v5.5.5；exit 0

idf.py --list-targets | rg '^esp32s3$'
# esp32s3；exit 0

idf.py -C . build
# Project build complete；exit 0
```

这里的 `MAKER_CMAKE_BIN`／`MAKER_CTEST_BIN` 是 ESP-IDF 工具目录中匹配 CMake 3.30.2 的可执行文件，`MAKER_IDF_PATH` 是本机已安装且项目锁定的 ESP-IDF v5.5.5；三者只用于本轮命令，不写入仓库配置。

## CTest 数量核对

- 课程起点：56。
- Host Action 新增目标：`host_action_protocol_tests`、`host_action_key_bindings_tests`、`host_action_capability_status_tests`，共 3 个。
- 本轮重新配置后实际发现：59。
- 数量关系：`56 + 3 = 59`；没有新增、缺失或未被 CTest 发现的测试。
- 本轮执行：59；通过：59；失败：0；跳过：0；发现数、执行数和通过数一致。
- CTest 报告总耗时：1.06 秒。

## 本轮完整 CTest 清单与结果

| # | 测试名称 | 结果 |
|---:|---|---|
| 1 | `board_pins_tests` | Passed |
| 2 | `board_pins_v2_tests` | Passed |
| 3 | `audio_io_arbiter_tests` | Passed |
| 4 | `audio_control_wire_tests` | Passed |
| 5 | `audio_packet_wire_tests` | Passed |
| 6 | `audio_session_tests` | Passed |
| 7 | `agent_status_tests` | Passed |
| 8 | `awake_wait_planner_tests` | Passed |
| 9 | `boot_led_sequence_tests` | Passed |
| 10 | `cold_boot_feedback_tests` | Passed |
| 11 | `battery_estimator_tests` | Passed |
| 12 | `ble_advertising_state_tests` | Passed |
| 13 | `ble_connection_profile_tests` | Passed |
| 14 | `ble_management_gate_tests` | Passed |
| 15 | `ble_input_scheduler_tests` | Passed |
| 16 | `ble_fixed_text_stream_tests` | Passed |
| 17 | `ble_owner_recovery_tests` | Passed |
| 18 | `ble_status_wire_tests` | Passed |
| 19 | `config_payload_tests` | Passed |
| 20 | `config_receiver_tests` | Passed |
| 21 | `config_state_tests` | Passed |
| 22 | `config_status_tests` | Passed |
| 23 | `debounce_tests` | Passed |
| 24 | `diagnostic_command_tests` | Passed |
| 25 | `encoder_tests` | Passed |
| 26 | `encoder_press_gesture_tests` | Passed |
| 27 | `firmware_source_contract_tests` | Passed |
| 28 | `held_keyboard_state_tests` | Passed |
| 29 | `hid_report_queue_tests` | Passed |
| 30 | `hid_keycode_tests` | Passed |
| 31 | `host_action_capability_status_tests` | Passed |
| 32 | `input_feedback_tests` | Passed |
| 33 | `input_test_tests` | Passed |
| 34 | `speaker_assets_wifi_policy_tests` | Passed |
| 35 | `ima_adpcm_decoder_tests` | Passed |
| 36 | `sound_asset_store_tests` | Passed |
| 37 | `sound_asset_reader_tests` | Passed |
| 38 | `speaker_assets_store_executor_tests` | Passed |
| 39 | `speaker_assets_flash_runner_tests` | Passed |
| 40 | `speaker_assets_protocol_tests` | Passed |
| 41 | `speaker_assets_wifi_wire_tests` | Passed |
| 42 | `speaker_assets_session_tests` | Passed |
| 43 | `speaker_assets_runtime_tests` | Passed |
| 44 | `gatt_status_snapshot_tests` | Passed |
| 45 | `host_action_key_bindings_tests` | Passed |
| 46 | `host_action_protocol_tests` | Passed |
| 47 | `keyboard_snapshot_delivery_tests` | Passed |
| 48 | `keymap_tests` | Passed |
| 49 | `platform_selection_tests` | Passed |
| 50 | `peripheral_power_lease_tests` | Passed |
| 51 | `power_cycle_tests` | Passed |
| 52 | `power_policy_tests` | Passed |
| 53 | `speaker_audio_contract_tests` | Passed |
| 54 | `speaker_probe_status_tests` | Passed |
| 55 | `speaker_playback_tests` | Passed |
| 56 | `speaker_service_startup_tests` | Passed |
| 57 | `status_hid_protocol_tests` | Passed |
| 58 | `transport_routing_tests` | Passed |
| 59 | `usb_hid_endpoint_arbiter_tests` | Passed |

## ESP-IDF 默认构建证据

- ESP-IDF：`v5.5.5`。
- Maker 板型：默认 `v2`。
- 芯片目标：`esp32s3`。
- 构建结果：`Project build complete`，退出码 0；没有环境、依赖、编译或链接失败。
- 本轮增量重新编译并与 Host Action 状态／发送路径相关的文件：`components/keyboard/src/status_hid_protocol.cpp`、`components/keyboard/src/config_status.cpp`、`main/app_main.cpp`、`main/platform/usb_hid.cpp`、`main/platform/ble_hid.cpp`。此外重新编译了 `main/platform/speaker_assets_supervisor.cpp`。
- `components/keyboard/src/host_action_protocol.cpp` 已作为 `libkeyboard.a` 的输入参与本次链接；由于其对象文件在本轮构建前已是最新状态，Ninja 没有重复编译它，不能把“参与链接”误写成“本轮重新编译”。
- 应用镜像：`build/easy_input_keyboard.bin`，`0x190470`／1,639,536 字节。
- ELF：`build/easy_input_keyboard.elf`，21,387,660 字节。
- 最小 App 分区：`0x300000`／3,145,728 字节；剩余 `0x16fb90`／1,506,192 字节，构建工具报告 48% free。
- Bootloader：`0x5160` 字节；其区域剩余 `0x2ea0` 字节，36% free。
- 本轮构建输出中的 `warning`、`error` 与 `deprecated` 均为 0；无需代码或配置修复。

## 修改与禁止范围核对

- 本轮测试与构建没有修改源代码、宿主测试、`sdkconfig.defaults` 或 `partitions.csv`；只在验证完成后新增本任务卡，并更新 `flow/plan.md` 与 `flow/进展.md`。
- 对操作前后受保护文件做 SHA-1 对照，`sdkconfig.defaults`、`partitions.csv`、Host Action 共享协议头／实现、状态 HID 协议头、能力声明头、`main/app_main.cpp`、USB／BLE 适配和 `host_test/CMakeLists.txt` 均保持一致。
- 未运行 `set-target`、`menuconfig`、`flash`、`monitor`、端口枚举、设备识别或 App 命令；没有修改 Report ID、kind、容器、描述符、GATT、设备身份、GPIO、BOOT、GPIO8 电源或分区。

## 结论与仍未验证项

- `59/59` 宿主测试通过只证明当前代码的宿主可执行逻辑和源码合同通过。
- ESP-IDF v5.5.5 默认构建通过只证明当前 `v2`／`esp32s3` 源码可以生成固件镜像。
- 本节点仍未验证：烧录、启动、串口日志、USB／BLE 真机发送、双连接时的真机单通道路由、512 字节状态的真机发布、EasyInput App 0.1.26 能力识别与 UUID→本机应用映射。

## 2026-10-02 状态更新：软件侧总验收在当前基线上重跑（60 项，与上文 59 项的差异已定位）

> 本节由后续节点追加，上文是 `34087cd` 时点的真实验收记录，**原文保留不改**。上文的 `59/59`、镜像 `0x190470` 等数字是当时快照，对当前基线 `7d5f157` 已不适用。

### 数量差异的定位（回应「与上一轮记录不同」）

- 上文记载「课程起点 56 + Host Action 新增 3 = 59」。**实测课程起点注册数为 57，不是 56**：`git show origin/course/host-action-v1-starter:host_test/CMakeLists.txt` 统计到 57 条测试注册，其中第 97 行已包含 `ble_persistence_policy_tests`，且**不含任何 Host Action 测试**。
- 把上文清单与当前 `ctest -N` 实际清单做集合比对：**新增 1 项 = `ble_persistence_policy_tests`；缺失 0 项**。
- 因此当前 60 = 课程起点 57 + Host Action 3（`host_action_protocol_tests`、`host_action_key_bindings_tests`、`host_action_capability_status_tests`，三者均已注册），**没有缺失或未被 CTest 发现的测试**。上文「56」应为计数误差。

### 本轮实际命令与结果

```bash
cmake -S host_test -B build-host -G Ninja -DCMAKE_BUILD_TYPE=Debug   # exit 0
ctest --test-dir build-host -N                                        # Total Tests: 60；exit 0
cmake --build build-host                                             # exit 0
ctest --test-dir build-host --output-on-failure                       # exit 0
```

- CTest 实际发现 **60**、执行 **60**、通过 **60**、失败 **0**、跳过 **0**；声明清单与执行清单差集为空。未跳过、删除或缩减任何测试来凑数字。
- 本轮 `main/`、`components/`、`host_test/CMakeLists.txt` 与 `sdkconfig.defaults` 均未产生新改动，宿主测试全部沿用先前节点已通过的版本（本轮只补充了测试内部断言，不改变注册数量）。

### ESP-IDF 默认构建（本轮为全量重建）

- ESP-IDF `v5.5.5`（`idf.py --version`，退出码 0）；目标 `esp32s3` 经 `idf.py --list-targets` 确认存在（退出码 0）；板型为默认 `v2`。**未升级 ESP-IDF、未 `set-target`、未改分区表**。
- 本轮先删除 `build/` 再执行 `idf.py build`，因此是**全量重建**：约 1290 个编译单元，`Project build complete`，退出码 0。
- 本轮真实重新编译且与 Host Action 相关的文件：`components/keyboard/src/config_payload.cpp`、`keymap.cpp`、`config_status.cpp`、`host_action_protocol.cpp`、`status_hid_protocol.cpp`、`transport_routing.cpp`、`main/app_main.cpp`、`main/platform/ble_hid.cpp`、`main/platform/usb_hid.cpp`。（与上文不同：上文为增量构建，`host_action_protocol.cpp` 未重编；本轮全量重建中它确实被重新编译。）
- 产物：`build/easy_input_keyboard.bin` 1,640,528 字节（0x190850）；`build/easy_input_keyboard.elf` 21,469,408 字节；`build/bootloader/bootloader.bin` 20,832 字节（0x5160）；`build/partition_table/partition-table.bin` 3,072 字节。
- App 分区 `0x300000`／3,145,728 字节，剩余 `0x16f7b0`／1,505,200 字节（工具报告 48% free）；Bootloader 区域剩余 `0x2ea0`（36% free）。镜像比上文 `0x190470` 大 0x3E0 字节，属基线代码增长，非本轮引入。
- warning 与 error 均为 **0**。构建输出中 13 处 `deprecated` 命中的是 ESP-IDF 自带的 `deprecated/` 源码目录名（`esp_adc`、`driver`、`esp_hw_support`、`bt/ble_log`），**不是弃用告警**，无需修复。

### 禁止范围与仍未验证项

- `git diff --stat -- main components/keyboard/src/host_action_protocol.cpp components/keyboard/include/keyboard/host_action_protocol.h components/keyboard/src/status_hid_protocol.cpp components/keyboard/include/keyboard/status_hid_protocol.h components/keyboard/include/keyboard/config_status.h components/keyboard/src/config_status.cpp components/keyboard/include/keyboard/board_pins.h sdkconfig.defaults partitions.csv` **无输出**；未改 Report ID、kind、容器、HID 描述符、BLE GATT、设备身份、GPIO、BOOT、GPIO8 电源控制或分区；未识别设备、未读串口、未烧录、未运行 App。
- `60/60` 宿主测试通过只证明当前代码的宿主可执行逻辑与源码合同通过；ESP-IDF 默认构建通过只证明当前 `v2`／`esp32s3` 源码可以生成固件镜像。**烧录、启动、串口日志、USB／BLE 真机发送、双连接真机单通道路由、512 字节状态真机发布、App 0.1.26 能力识别与 UUID→本机应用映射，均仍未验证。**
