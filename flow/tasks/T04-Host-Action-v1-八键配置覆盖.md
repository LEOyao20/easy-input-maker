# T04 · Host Action v1 八键配置覆盖

> 本任务继续遵守 T02 冻结合同，并以 T03 纯逻辑实现为基线。检查确认现有生产代码已通用覆盖八键，因此本节点只新增测试覆盖，不重写架构。

- **目标**：证明 KEY1—KEY8 每个实体主按键都能解析完整 Host Action 配置，Keymap 保留完整前缀，并在一次 Pressed／Released 周期中只产生一个 Host Action。
- **允许范围**：`host_test/` 与根级 `flow/`；只有发现真实缺口时才允许最小修改 `components/keyboard/`。
- **禁止范围**：不修改 `main/platform`、USB／BLE、能力声明、GPIO、BOOT、电源、Flash 分区或设备身份；不烧录。
- **状态**：完成；未发现生产逻辑缺口。

## 通用解析检查

`components/keyboard/src/config_payload.cpp` 现有 `key_inputs` 数组明确列出 KEY1—KEY8，随后用同一个循环为每项调用 `parse_binding()` 并写入对应 `InputId`。Host Action 的识别发生在该公共函数继续调用的 `parse_action()` 中，没有逐键特例。

结论：八键能力已经存在；本轮无需修改配置解析、Keymap 或事件生产代码，只缺少逐键自动化覆盖。

## 操作前状态与本轮改动

- 分支与提交：`main`，`34087cd40d24d23579da0357973ebc1a37e7ce7c`。
- 操作前工作区包含 T03 的既有纯逻辑、宿主测试及 `flow/` 变更；本轮先记录关键文件 SHA-256，以区分增量。
- 本轮实际代码／测试修改只有：
  - `host_test/CMakeLists.txt`
  - `host_test/host_action_key_bindings_tests.cpp`（新增）
- 本轮 Flow 修改：
  - `flow/plan.md`
  - `flow/进展.md`
  - 本任务卡
- `components/keyboard/src/config_payload.cpp`、`components/keyboard/src/keymap.cpp`、既有 `config_payload_tests.cpp` 与 `keymap_tests.cpp` 的前后 SHA-256 完全一致，未为制造差异修改生产逻辑或旧测试。

## KEY1—KEY8 逐项证据

参数化测试在启用断言的宿主测试目标中，对下列每一行分别构造完整配置并实际执行解析和事件周期：

| 配置槽位 | Keymap 落位 | 完整前缀保留 | Pressed | Released | 结果 |
|---|---|---|---|---|---|
| KEY1 | `InputId::Key1` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY2 | `InputId::Key2` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY3 | `InputId::Key3` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY4 | `InputId::Key4` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY5 | `InputId::Key5` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY6 | `InputId::Key6` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY7 | `InputId::Key7` | 是 | 1 个 `HostAction` | `None` | 通过 |
| KEY8 | `InputId::Key8` | 是 | 1 个 `HostAction` | `None` | 通过 |

每轮还断言其他七键保持 `Disabled` 且 `host_action` 字段为空，防止配置串位。事件计数器覆盖一次 Pressed 和一次 Released，最终必须严格等于 1。

## 旧动作兼容证据

- **复制**：`config_payload_tests` 继续确认 KEY6 解析为 `ActionKind::Copy`；`keymap_tests` 继续确认 macOS／Windows 分别生成 `Meta+C`／`Ctrl+C`。
- **粘贴**：`config_payload_tests` 继续确认 KEY7 解析为 `ActionKind::Paste`；`keymap_tests` 继续确认 macOS／Windows 分别生成 `Meta+V`／`Ctrl+V`。
- **快捷键**：`config_payload_tests` 继续确认非工厂热键保留为 `ActionKind::Hotkey`；`keymap_tests` 的自定义快捷键继续产生配对 `HidKeyDown`／`HidKeyUp`。
- **固定文字**：`config_payload_tests` 继续验证文字解析、960 字节边界与错误输入；`keymap_tests` 继续验证按下产生 `FixedText`、松开 `None`。
- 上述测试均包含在本轮定向 5/5 与完整宿主 58/58 结果中。

## 示例 UUID 边界证据

- 对固定示例 UUID 做全仓精确检索，只命中 `host_test/` 下的 Host Action 测试。
- 对 `components/` 与 `main/` 的同一精确检索无输出。
- `DefaultKeymap()` 仍为原有 PTT、Return、Backspace、复制、粘贴、撤销和编码器动作，没有 Host Action 或示例 UUID。
- 因此固定示例没有进入默认 Keymap、生产配置或最终应用映射。

## 测试命令与结果

普通 shell 未直接暴露 `cmake`；实际执行继续使用当前 ESP-IDF 工具链已安装的 CMake 3.30.2。公开记录使用等价、可移植命令：

```bash
cmake -S host_test -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target host_action_key_bindings_tests host_action_protocol_tests config_payload_tests config_state_tests keymap_tests -j4
ctest --test-dir build-host --output-on-failure -R '^(host_action_key_bindings_tests|host_action_protocol_tests|config_payload_tests|config_state_tests|keymap_tests)$'
cmake --build build-host -j4
ctest --test-dir build-host --output-on-failure
```

- 定向测试：5/5 通过，0 失败，总用时 0.45 秒。
- 完整宿主测试：58/58 通过，0 失败，总用时 1.00 秒。
- `git diff --check` 通过。

## 禁止范围差异证据

- `git diff -- main main/platform` 无输出。
- `git diff --name-only -- main main/platform` 无输出。
- 本轮没有修改 `components/keyboard/` 生产文件。
- 没有修改能力声明、GPIO、BOOT、电源、Flash 分区或设备身份，没有烧录。

## 仍未验证项

- NVS 持久化仍未专项验证。
- `main/app_main.cpp` 与 USB／BLE 适配仍未接入 Host Action，不能把本轮结果写成实际传输成功。
- 未运行 ESP-IDF 5.5.5 默认构建。
- 未验证 EasyInput App 0.1.26 同步、真实设备发送、烧录、串口或实板行为。
- 第 08 步 `"host_action_v1": true` 与 BLE 状态 512 字节预算仍未实现或验证。

## 2026-10-02 状态更新：八键覆盖在当前基线上复核通过

> 本节由后续节点追加。上文是 `34087cd` 时点的真实记录，**原文保留不改**；其中「完整宿主 58/58」「第 08 步能力声明仍未实现或验证」等表述对当前基线 `7d5f157` 已不成立（完整套件现为 60 项；第 08 步 `"host_action_v1": true` 与 BLE 512 字节预算已实现）。

- 复核结论：八键共用通用解析、Keymap 保留完整 `host_action:` 前缀、按下只产生一次、松开不产生第二个动作，**在当前基线上依旧成立**。
- 本轮测试结果：定向 5/5 通过，完整宿主 **60/60 通过、0 失败**，编译错误 0，`ctest` 退出码 0。
- 本轮唯一新增覆盖：`host_test/keymap_tests.cpp` 的 `default_mapping_covers_all_keys_and_encoder()` 增加两条负向断言 —— 对全部 11 个输入断言 `kind != ActionKind::HostAction` 且 `host_action.empty()`。原有断言只钉住 `ActionKind`，无法发现「动作类型没变、但 `host_action` 字段被写入」的情况；这条边界正是「固定示例 UUID 不得进入默认 Keymap」所要求的。
- 生产代码零改动：`config_payload.cpp`、`keymap.cpp`、`keymap.h`、`host_action_protocol.cpp`、`usb_hid.cpp`、`ble_hid.cpp` 等 10 个文件的 SHA-256 与复核前完全一致。
- 示例 UUID 边界证据（2026-10-02 实测）：精确检索只命中 `host_test/` 下 6 个测试文件；`components/`、`main/`、`features/`、`diagnostics/` 命中数为 **0**。
- 仍未验证项：与上文一致；另本轮**未做变异验证**（避免再次触发 CRT abort 对话框），新增负向断言的「有效性」尚无突变证据；仍未构建固件、未烧录、未接 USB／BLE。
