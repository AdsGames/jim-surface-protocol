#pragma once

#include <asw/asw.h>

#include "world.h"

constexpr float BUTTON_SIZE = 64.0F;
constexpr float TOOLBAR_HEIGHT = 160.0F;

enum class ToolMode { PURIFIER, TREE, DRILL };

class Toolbar {
 public:
  Toolbar() = default;

  void init();
  void update(float dt, World& world);
  void draw(World& world);

 private:
  float actionProgress{0.0F};

  void action(World& world, float dt);
  bool actionEnabled(World& world);

  void toolZoneAction(World& world);
  void rightClickAction(World& world);

  void drawWireframe(const asw::Vec3<int>& position,
                     const asw::Vec2<float>& offset,
                     asw::Color colour);

  asw::Font font;
  asw::Font fontLarge;

  asw::Vec3<int> cursor_idx{0, 0, 0};
  bool cursor_in_range{false};

  asw::Texture inspect_button;
  asw::Texture purifier_button;
  asw::Texture tree_button;
  asw::Texture drill_button;
  asw::Texture upgrade;
  asw::Texture toolbar_ui;

  asw::Quad<float> drill_button_trans{620.0F, 820.0F, BUTTON_SIZE, BUTTON_SIZE};
  asw::Quad<float> purifier_button_trans{700.0F, 820.0F, BUTTON_SIZE,
                                         BUTTON_SIZE};
  asw::Quad<float> tree_button_trans{780.0F, 820.0F, BUTTON_SIZE, BUTTON_SIZE};

  asw::Quad<float> upgrade_drill_trans{1040.0F, 868.0F, 32, 32};
  asw::Quad<float> upgrade_move_trans{1040.0F, 915.0F, 32, 32};

  int drill_upgrade_cost{10};
  int move_upgrade_cost{10};

  ToolMode mode{ToolMode::DRILL};

  bool can_take_action{false};
};