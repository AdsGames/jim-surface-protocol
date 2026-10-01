#pragma once

#include <asw/asw.h>
#include <array>
#include <nlohmann/json.hpp>
#include <vector>

#include "world.h"

constexpr float TOOLBAR_HEIGHT = 160.0F;
constexpr int TREE_COST = 10;
constexpr int PURIFIER_COST = 40;

// In button order, left to right
enum class ToolMode { DRILL, PURIFIER, TREE };

// Image button with a frame when its tool is chosen
class ToolButton : public asw::ui::Button {
 public:
  bool selected{false};

  void draw(asw::ui::Context& ctx) override;
};

// Label with a drop shadow, for headings
class ShadowLabel : public asw::ui::Label {
 public:
  void draw(asw::ui::Context& ctx) override;
};

// Thin bar filled from the left
class ProgressBar : public asw::ui::Widget {
 public:
  float value{0.0F};
  asw::Color fill{128, 255, 128};
  asw::Color background{32, 32, 32};

  void draw(asw::ui::Context& ctx) override;
};

class Toolbar {
 public:
  Toolbar() = default;

  // Build the toolbar into the UI, anchored to the bottom of the screen
  void init(asw::ui::Root& ui, World& world);

  // pointer_used_by_ui: the UI used the mouse this frame, e.g. over a button.
  // world_input: false while a controller moves focus around the toolbar
  void update(float dt,
              World& world,
              asw::ui::Root& ui,
              bool pointer_used_by_ui,
              bool world_input);

  // Cursor outline, drill progress and toxic haze. Call before the UI draws
  void drawWorld(World& world);

  // Widget to focus when a controller moves into the toolbar
  asw::ui::Widget& firstFocus() { return *tool_buttons[0]; }

  void save(nlohmann::json& data) const;
  void load(const nlohmann::json& data);

 private:
  // Build helpers. Positions are from the top left of the toolbar
  template <class T>
  T& place(T& widget, float x, float y, float w = 0.0F, float h = 0.0F);
  asw::ui::Label& addLabel(float x,
                           float y,
                           const asw::Font& font,
                           asw::Color color = asw::color::white);

  void selectTool(ToolMode new_mode);
  // Choose the tool step places along, wrapping round. LB and RB
  void cycleTool(int step);
  void buyUpgrade(bool drill, World& world, asw::ui::Root& ui);

  // True if the pointer is over the toolbar
  bool pointerOnToolbar() const;

  // True if the widget is under the mouse, the controller cursor or has
  // controller focus
  bool isPointedAt(const asw::ui::Widget& widget,
                   const asw::ui::Root& ui) const;

  // The controller cursor presses toolbar buttons itself, since the UI only
  // follows the mouse
  void pressButtonUnderControllerPointer();

  // strength: how hard the trigger is held, from 0 to 1, for rumble
  void action(World& world,
              float dt,
              asw::ui::Root& ui,
              ToolMode tool,
              float strength);
  bool actionEnabled(World& world, ToolMode tool) const;
  void explainDisabledAction(World& world, asw::ui::Root& ui, ToolMode tool);
  void setWaypoint(World& world);

  // Widget text and state from the world
  void refresh(World& world, const asw::ui::Root& ui);
  void setInfo(const std::vector<std::string>& lines);

  void drawWireframe(const asw::Vec3<int>& position,
                     const asw::Vec2<float>& offset,
                     asw::Color colour);

  asw::Font font;
  asw::Font fontLarge;

  asw::Texture bar_texture;
  asw::Texture purifier_texture;
  asw::Texture tree_texture;
  asw::Texture drill_texture;
  asw::Texture upgrade_texture;
  asw::Texture overlay_1;

  // Widgets, owned by the UI tree
  asw::ui::Panel* bar{nullptr};
  asw::ui::Button* guide_button{nullptr};
  std::array<ToolButton*, 3> tool_buttons{};
  asw::ui::Button* upgrade_drill_button{nullptr};
  asw::ui::Button* upgrade_move_button{nullptr};
  std::array<asw::ui::Label*, 6> info_lines{};
  asw::ui::Label* drill_speed_label{nullptr};
  asw::ui::Label* drill_cost_label{nullptr};
  asw::ui::Label* move_speed_label{nullptr};
  asw::ui::Label* move_cost_label{nullptr};
  asw::ui::Label* scrap_label{nullptr};
  asw::ui::Label* biomass_label{nullptr};
  asw::ui::Label* purification_label{nullptr};
  ProgressBar* purification_bar{nullptr};

  asw::Vec3<int> cursor_idx{0, 0, 0};
  bool cursor_in_range{false};
  bool pointer_in_world{false};
  bool can_take_action{false};
  float actionProgress{0.0F};

  int drill_upgrade_cost{10};
  int move_upgrade_cost{10};

  ToolMode mode{ToolMode::DRILL};
};
