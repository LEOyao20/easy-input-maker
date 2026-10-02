#include <array>
#include <cassert>
#include <cstddef>
#include <string>

#include "keyboard/config_payload.h"
#include "keyboard/host_action_protocol.h"
#include "keyboard/keymap.h"

namespace {

struct KeyBinding {
  const char* name;
  ai_keyboard::InputId input;
};

constexpr std::array<KeyBinding, 8> kMainKeys{{
    {"KEY1", ai_keyboard::InputId::Key1},
    {"KEY2", ai_keyboard::InputId::Key2},
    {"KEY3", ai_keyboard::InputId::Key3},
    {"KEY4", ai_keyboard::InputId::Key4},
    {"KEY5", ai_keyboard::InputId::Key5},
    {"KEY6", ai_keyboard::InputId::Key6},
    {"KEY7", ai_keyboard::InputId::Key7},
    {"KEY8", ai_keyboard::InputId::Key8},
}};

std::string payload_with_host_action_at(std::size_t target,
                                        const std::string& host_action) {
  std::string payload =
      R"({"schema":"ai_keyboard.v1","profiles":[{"id":"default","keys":{)";
  for (std::size_t index = 0; index < kMainKeys.size(); ++index) {
    if (index != 0) {
      payload += ',';
    }
    payload += '"';
    payload += kMainKeys[index].name;
    payload += R"(":{"press":")";
    payload += index == target ? host_action : "disabled";
    payload += R"("})";
  }
  payload +=
      R"(},"encoder":{"left":"disabled","right":"disabled","press":"disabled"}}]})";
  return payload;
}

void every_main_key_preserves_and_emits_one_host_action_per_press_cycle() {
  const std::string configured_action =
      "host_action:123e4567-e89b-12d3-a456-426614174000";

  for (std::size_t target = 0; target < kMainKeys.size(); ++target) {
    const auto result = ai_keyboard::parse_config_payload(
        payload_with_host_action_at(target, configured_action));
    assert(result.status == ai_keyboard::ConfigParseStatus::Ok);

    for (std::size_t index = 0; index < kMainKeys.size(); ++index) {
      const auto& action =
          result.config.keymap.action_for(kMainKeys[index].input);
      if (index == target) {
        assert(action.kind == ai_keyboard::ActionKind::HostAction);
        assert(action.host_action == configured_action);
        assert(action.host_action.rfind("host_action:", 0) == 0);
      } else {
        assert(action.kind == ai_keyboard::ActionKind::Disabled);
        assert(action.host_action.empty());
      }
    }

    const auto& action =
        result.config.keymap.action_for(kMainKeys[target].input);
    const std::array<ai_keyboard::InputPhase, 2> cycle{{
        ai_keyboard::InputPhase::Pressed,
        ai_keyboard::InputPhase::Released,
    }};
    std::size_t host_action_events = 0;
    for (const auto phase : cycle) {
      const auto event = ai_keyboard::event_for_action(
          action, phase, "RightMeta", "RightOption");
      if (event.kind == ai_keyboard::FirmwareEventKind::HostAction) {
        ++host_action_events;
        assert(phase == ai_keyboard::InputPhase::Pressed);
        assert(event.value == configured_action);
      } else {
        assert(phase == ai_keyboard::InputPhase::Released);
        assert(event.kind == ai_keyboard::FirmwareEventKind::None);
        assert(event.value.empty());
      }
    }
    assert(host_action_events == 1);
  }
}

void encoded_host_action_matches_the_frozen_payload_for_every_main_key() {
  // The configuration, Keymap and event layers are covered by the case above.
  // This closes the last host-visible hop: the event emitted by a configured
  // key is fed into the very same shared encoder that both the USB and BLE
  // adapters call, so the frozen wire bytes are verified end to end without
  // either adapter copying its own constants.
  const std::string configured_action =
      "host_action:123e4567-e89b-12d3-a456-426614174000";
  const std::string expected_uuid =
      "123e4567-e89b-12d3-a456-426614174000";

  for (std::size_t target = 0; target < kMainKeys.size(); ++target) {
    const auto result = ai_keyboard::parse_config_payload(
        payload_with_host_action_at(target, configured_action));
    assert(result.status == ai_keyboard::ConfigParseStatus::Ok);

    const auto& action =
        result.config.keymap.action_for(kMainKeys[target].input);
    const auto event = ai_keyboard::event_for_action(
        action, ai_keyboard::InputPhase::Pressed, "RightMeta", "AltGr");
    assert(event.kind == ai_keyboard::FirmwareEventKind::HostAction);
    assert(event.value == configured_action);

    ai_keyboard::HostActionV1Report report;
    assert(ai_keyboard::encode_host_action_v1(event.value, &report));
    assert(report.report_id == ai_keyboard::kHostActionV1ReportId);
    assert(report.payload[0] == ai_keyboard::kHostActionV1CommandKind);
    assert(report.payload[1] == ai_keyboard::kHostActionV1ChunkIndex);
    assert(report.payload[2] == ai_keyboard::kHostActionV1TotalChunks);
    assert(report.payload[3] == 36);
    assert(report.payload[3] == ai_keyboard::kHostActionV1UuidLen);

    const std::string encoded(
        reinterpret_cast<const char*>(report.payload.data() +
                                      ai_keyboard::kHostActionV1HeaderLen),
        ai_keyboard::kHostActionV1UuidLen);
    assert(encoded == expected_uuid);
    assert(encoded.find("host_action:") == std::string::npos);
    for (std::size_t index = ai_keyboard::kHostActionV1HeaderLen +
                             ai_keyboard::kHostActionV1UuidLen;
         index < report.payload.size(); ++index) {
      assert(report.payload[index] == 0);
    }
  }
}

}  // namespace

int main() {
  every_main_key_preserves_and_emits_one_host_action_per_press_cycle();
  encoded_host_action_matches_the_frozen_payload_for_every_main_key();
  return 0;
}
