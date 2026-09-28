#include "controls.h"

#include <asw/asw.h>

namespace {
using asw::input::ControllerAxis;
using asw::input::ControllerAxisBinding;
using asw::input::ControllerButton;
using asw::input::ControllerButtonBinding;
using asw::input::Key;
using asw::input::KeyBinding;

constexpr auto ANY = asw::input::ANY_CONTROLLER;

// Stick must pass this before it counts as a direction
constexpr float STICK_THRESHOLD = 0.5F;

void bind_direction(const char* name,
                    Key key,
                    Key alt_key,
                    ControllerButton dpad,
                    ControllerAxis axis,
                    bool positive) {
  asw::input::bind_action(name, KeyBinding{key});
  asw::input::bind_action(name, KeyBinding{alt_key});
  asw::input::bind_action(name, ControllerButtonBinding{dpad, ANY});
  asw::input::bind_action(
      name, ControllerAxisBinding{axis, ANY, STICK_THRESHOLD, positive});
}
}  // namespace

void controls::bind() {
  bind_direction(CAMERA_UP, Key::W, Key::Up, ControllerButton::DPadUp,
                 ControllerAxis::LeftY, false);
  bind_direction(CAMERA_DOWN, Key::S, Key::Down, ControllerButton::DPadDown,
                 ControllerAxis::LeftY, true);
  bind_direction(CAMERA_LEFT, Key::A, Key::Left, ControllerButton::DPadLeft,
                 ControllerAxis::LeftX, false);
  bind_direction(CAMERA_RIGHT, Key::D, Key::Right, ControllerButton::DPadRight,
                 ControllerAxis::LeftX, true);

  asw::input::bind_action(TOOL_DRILL, KeyBinding{Key::Num1});
  asw::input::bind_action(TOOL_PURIFIER, KeyBinding{Key::Num2});
  asw::input::bind_action(TOOL_TREE, KeyBinding{Key::Num3});

  asw::input::bind_action(BACK, KeyBinding{Key::Escape});
  asw::input::bind_action(BACK,
                          ControllerButtonBinding{ControllerButton::Back, ANY});

  asw::input::bind_action(FULLSCREEN, KeyBinding{Key::F11});
}

bool controls::any_skip() {
  return asw::input::get_keyboard().any_pressed ||
         asw::input::get_controller_button_down(ANY, ControllerButton::A) ||
         asw::input::get_controller_button_down(ANY, ControllerButton::B) ||
         asw::input::get_controller_button_down(ANY, ControllerButton::Start);
}
