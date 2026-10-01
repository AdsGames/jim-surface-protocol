#include "controls.h"

#include <algorithm>

namespace {
using asw::input::ControllerAxis;
using asw::input::ControllerAxisBinding;
using asw::input::ControllerButton;
using asw::input::ControllerButtonBinding;
using asw::input::Key;
using asw::input::KeyBinding;
using asw::input::MouseButton;
using asw::input::MouseButtonBinding;

constexpr auto ANY = asw::input::ANY_CONTROLLER;

// Stick must pass this before it counts as a direction
constexpr float STICK_THRESHOLD = 0.5F;

// Trigger must pass this before it counts as held
constexpr float TRIGGER_THRESHOLD = 0.2F;

// Controller cursor speed at full tilt, in pixels per second
constexpr float POINTER_SPEED = 650.0F;

// Action name with no bindings, so it never fires
constexpr const char* UNBOUND = "ui_off";

asw::Vec2<float> controller_pointer{0.0F, 0.0F};
bool controller_pointer_active{false};

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
  // The left stick moves the cursor, so the right stick moves the camera
  bind_direction(CAMERA_UP, Key::W, Key::Up, ControllerButton::DPadUp,
                 ControllerAxis::RightY, false);
  bind_direction(CAMERA_DOWN, Key::S, Key::Down, ControllerButton::DPadDown,
                 ControllerAxis::RightY, true);
  bind_direction(CAMERA_LEFT, Key::A, Key::Left, ControllerButton::DPadLeft,
                 ControllerAxis::RightX, false);
  bind_direction(CAMERA_RIGHT, Key::D, Key::Right, ControllerButton::DPadRight,
                 ControllerAxis::RightX, true);

  asw::input::bind_action(TOOL_DRILL, KeyBinding{Key::Num1});
  asw::input::bind_action(TOOL_PURIFIER, KeyBinding{Key::Num2});
  asw::input::bind_action(TOOL_TREE, KeyBinding{Key::Num3});
  asw::input::bind_action(
      TOOL_PREV, ControllerButtonBinding{ControllerButton::LeftShoulder, ANY});
  asw::input::bind_action(
      TOOL_NEXT, ControllerButtonBinding{ControllerButton::RightShoulder, ANY});

  asw::input::bind_action(USE, MouseButtonBinding{MouseButton::Left});
  asw::input::bind_action(
      DRILL, ControllerAxisBinding{ControllerAxis::RightTrigger, ANY,
                                   TRIGGER_THRESHOLD, true});
  asw::input::bind_action(
      USE_TOOL, ControllerAxisBinding{ControllerAxis::LeftTrigger, ANY,
                                      TRIGGER_THRESHOLD, true});
  asw::input::bind_action(WAYPOINT, MouseButtonBinding{MouseButton::Right});
  asw::input::bind_action(WAYPOINT,
                          ControllerButtonBinding{ControllerButton::A, ANY});

  asw::input::bind_action(CENTER_CAMERA, KeyBinding{Key::Space});
  asw::input::bind_action(CENTER_CAMERA,
                          ControllerButtonBinding{ControllerButton::X, ANY});

  asw::input::bind_action(TOOLBAR,
                          ControllerButtonBinding{ControllerButton::Y, ANY});

  asw::input::bind_action(BACK, KeyBinding{Key::Escape});
  asw::input::bind_action(BACK,
                          ControllerButtonBinding{ControllerButton::Back, ANY});

  asw::input::bind_action(PAUSE, KeyBinding{Key::Escape});
  asw::input::bind_action(
      PAUSE, ControllerButtonBinding{ControllerButton::Start, ANY});
  asw::input::bind_action(PAUSE,
                          ControllerButtonBinding{ControllerButton::Back, ANY});

  asw::input::bind_action(FULLSCREEN, KeyBinding{Key::F11});
}

bool controls::any_skip() {
  return asw::input::get_keyboard().any_pressed ||
         asw::input::get_controller_button_down(ANY, ControllerButton::A) ||
         asw::input::get_controller_button_down(ANY, ControllerButton::B) ||
         asw::input::get_controller_button_down(ANY, ControllerButton::Start);
}

void controls::rumble(float low, float high, int duration_ms) {
  asw::input::rumble_controller(ANY, low, high,
                                static_cast<uint32_t>(duration_ms));
}

bool controls::any_world_action_held() {
  return asw::input::get_action(USE) || asw::input::get_action(DRILL) ||
         asw::input::get_action(USE_TOOL) || asw::input::get_action(WAYPOINT);
}

void controls::rumble_triggers(float left, float right, int duration_ms) {
  asw::input::rumble_controller_triggers(ANY, left, right,
                                         static_cast<uint32_t>(duration_ms));
}

void controls::update_pointer(float dt) {
  const auto& mouse = asw::input::get_mouse();
  if (mouse.change.x != 0.0F || mouse.change.y != 0.0F) {
    controller_pointer_active = false;
  }

  const auto stick =
      asw::input::get_controller_stick(ANY, asw::input::ControllerStick::Left);
  if (stick.x == 0.0F && stick.y == 0.0F) {
    return;
  }

  // Start from wherever the mouse left off
  if (!controller_pointer_active) {
    controller_pointer = mouse.position;
    controller_pointer_active = true;
  }

  const auto screen = asw::display::get_logical_size();
  controller_pointer += stick * POINTER_SPEED * dt;
  controller_pointer.x =
      std::clamp(controller_pointer.x, 0.0F, static_cast<float>(screen.x - 1));
  controller_pointer.y =
      std::clamp(controller_pointer.y, 0.0F, static_cast<float>(screen.y - 1));
}

asw::Vec2<float> controls::pointer() {
  return controller_pointer_active ? controller_pointer
                                   : asw::input::get_mouse().position;
}

bool controls::is_controller_pointer() {
  return controller_pointer_active;
}

void controls::draw_pointer() {
  if (!controller_pointer_active) {
    return;
  }

  // Crosshair, with a dark edge so it shows on any tile
  const auto& p = controller_pointer;
  asw::draw::circle(p, 11.0F, asw::color::black);
  asw::draw::circle(p, 10.0F, asw::color::white);
  asw::draw::line(p - asw::Vec2(14.0F, 0.0F), p - asw::Vec2(5.0F, 0.0F),
                  asw::color::white);
  asw::draw::line(p + asw::Vec2(5.0F, 0.0F), p + asw::Vec2(14.0F, 0.0F),
                  asw::color::white);
  asw::draw::line(p - asw::Vec2(0.0F, 14.0F), p - asw::Vec2(0.0F, 5.0F),
                  asw::color::white);
  asw::draw::line(p + asw::Vec2(0.0F, 5.0F), p + asw::Vec2(0.0F, 14.0F),
                  asw::color::white);
}

asw::ui::Navigation controls::menu_navigation(const char* back) {
  auto navigation = asw::ui::bind_default_navigation();
  navigation.back = back;
  return navigation;
}

asw::ui::Navigation controls::off_navigation() {
  return {.up = UNBOUND,
          .down = UNBOUND,
          .left = UNBOUND,
          .right = UNBOUND,
          .next = UNBOUND,
          .prev = UNBOUND,
          .activate = UNBOUND,
          .back = UNBOUND};
}
