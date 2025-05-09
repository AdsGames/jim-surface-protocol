#pragma once

#include <asw/asw.h>

#include "world.h"

constexpr float BUTTON_SIZE = 64.0F;

enum class ToolMode {
  INSPECT,
  TRASH,
  WORKER,
};

class Toolbar {
 public:
  Toolbar() = default;

  void init();
  void update(float dt, World& world);
  void draw(World& world);

 private:
  void action(World& world);

  void drawWireframe(const asw::Vec3<int>& position,
                     const asw::Vec2<float>& offset);

  asw::Font font;
  asw::Vec3<int> cursor_idx{0, 0, 0};

  asw::Texture inspect_button;
  asw::Texture trash_button;
  asw::Texture worker_button;

  asw::Quad<float> inspect_button_trans{10.0F, 0.0F, BUTTON_SIZE, BUTTON_SIZE};
  asw::Quad<float> trash_button_trans{84.0F, 0.0F, BUTTON_SIZE, BUTTON_SIZE};
  asw::Quad<float> worker_button_trans{158.0F, 0.0F, BUTTON_SIZE, BUTTON_SIZE};

  ToolMode mode{ToolMode::INSPECT};
};