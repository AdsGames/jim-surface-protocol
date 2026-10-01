#include "toolbar.h"

#include <cmath>
#include <format>
#include <map>
#include <optional>
#include <utility>

#include "../lib/controls.h"
#include "../tiles/tile_dictionary.h"
#include "../tiles/tile_ids.h"

namespace {
// The toolbar art is the bottom of a full screen image, from this row down
constexpr float BAR_ART_TOP = 799.0F;
constexpr float BAR_WIDTH = 1280.0F;
constexpr float BAR_HEIGHT = 161.0F;

constexpr float BUTTON_SIZE = 64.0F;
constexpr float UPGRADE_SIZE = 32.0F;

// Distance from the worker a tile can be worked, in tiles
constexpr float WORK_RANGE = 10.0F;

const asw::Color GREEN(128, 255, 128);
const asw::Color CAN_BUY(150, 255, 150);
const asw::Color CANT_BUY(255, 150, 150);

std::string need_more(const std::string& resource, int amount) {
  return "Need " + std::to_string(amount) + " " + resource;
}
}  // namespace

void ToolButton::draw(asw::ui::Context& ctx) {
  Button::draw(ctx);
  if (selected) {
    asw::draw::rect(transform, asw::color::yellow);
  }
}

void ShadowLabel::draw(asw::ui::Context& ctx) {
  const auto& f = asw::ui::pick_font(font, ctx.theme);
  if (!text.empty() && f != nullptr) {
    asw::draw::text_shadow(f, text, transform.position,
                           color.value_or(ctx.theme.text));
  }
  Widget::draw(ctx);
}

void ProgressBar::draw(asw::ui::Context& ctx) {
  asw::draw::rect_fill(transform, background);
  auto filled = transform;
  filled.size.x *= std::clamp(value, 0.0F, 1.0F);
  asw::draw::rect_fill(filled, fill);
  Widget::draw(ctx);
}

template <class T>
T& Toolbar::place(T& widget, float x, float y, float w, float h) {
  widget.anchor = asw::ui::Anchor::TopLeft;
  widget.anchor_margin = {x, y};
  widget.transform.size = {w, h};
  return widget;
}

asw::ui::Label& Toolbar::addLabel(float x,
                                  float y,
                                  const asw::Font& label_font,
                                  asw::Color color) {
  auto& label = place(bar->add_child<asw::ui::Label>(), x, y);
  label.font = label_font;
  label.color = color;
  return label;
}

void Toolbar::init(asw::ui::Root& ui, World& world) {
  drill_upgrade_cost = 10;
  move_upgrade_cost = 10;
  mode = ToolMode::DRILL;
  actionProgress = 0.0F;

  font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 18);
  fontLarge = asw::assets::load_font("assets/fonts/syne-mono.ttf", 30);

  purifier_texture =
      asw::assets::load_texture("assets/images/ui/purify-button.png");
  tree_texture = asw::assets::load_texture("assets/images/ui/tree-button.png");
  drill_texture =
      asw::assets::load_texture("assets/images/ui/drill-button.png");
  upgrade_texture = asw::assets::load_texture("assets/images/ui/upgrade.png");
  overlay_1 = asw::assets::load_texture("assets/images/ui/overlay_1.png");

  // Cut the bar out of the full screen art, so it can sit at any bottom edge
  const auto full_art =
      asw::assets::load_texture("assets/images/ui/toolbar.png");
  bar_texture = asw::assets::create_texture(static_cast<int>(BAR_WIDTH),
                                            static_cast<int>(BAR_HEIGHT));
  asw::draw::set_blend_mode(bar_texture, asw::BlendMode::Blend);
  asw::display::set_render_target(bar_texture);
  asw::display::clear(asw::Color(0, 0, 0, 0));
  asw::draw::stretch_sprite_blit(
      full_art, asw::Quad(0.0F, BAR_ART_TOP, BAR_WIDTH, BAR_HEIGHT),
      asw::Quad(0.0F, 0.0F, BAR_WIDTH, BAR_HEIGHT));
  asw::display::reset_render_target();

  bar = &ui.root.add_child<asw::ui::Panel>();
  bar->bg_image = bar_texture;
  bar->anchor = asw::ui::Anchor::Bottom;
  bar->transform.size = {BAR_WIDTH, BAR_HEIGHT};

  // J1M, who explains the game when pointed at
  guide_button =
      &place(bar->add_child<asw::ui::Button>(), 0.0F, 0.0F, 170.0F, BAR_HEIGHT);
  guide_button->draw_background = false;

  // Info panel
  for (std::size_t i = 0; i < info_lines.size(); ++i) {
    info_lines[i] = &addLabel(183.0F, 26.0F + (20.0F * i), font, GREEN);
  }

  // Tools
  const std::array<std::pair<const char*, asw::Texture>, 3> tools = {{
      {"Drill", drill_texture},
      {"Purifier", purifier_texture},
      {"Tree", tree_texture},
  }};
  for (std::size_t i = 0; i < tools.size(); ++i) {
    const float x = 620.0F + (80.0F * i);
    auto& button =
        place(bar->add_child<ToolButton>(), x, 21.0F, BUTTON_SIZE, BUTTON_SIZE);
    button.set_images(tools[i].second, nullptr, nullptr, nullptr, false);
    button.on_click = [this, i]() { selectTool(static_cast<ToolMode>(i)); };
    tool_buttons[i] = &button;

    auto& label = addLabel(x, 95.0F, font);
    label.transform.size = {BUTTON_SIZE, 20.0F};
    label.justify = asw::TextJustify::Center;
    label.text = tools[i].first;
  }
  selectTool(ToolMode::DRILL);

  purification_label = &addLabel(620.0F, 121.0F, font);
  purification_bar =
      &place(bar->add_child<ProgressBar>(), 620.0F, 145.0F, 200.0F, 2.0F);
  purification_bar->fill = GREEN;

  // Upgrades
  auto& upgrades = place(bar->add_child<ShadowLabel>(), 880.0F, 21.0F);
  upgrades.font = fontLarge;
  upgrades.text = "Upgrades";

  drill_speed_label = &addLabel(882.0F, 59.0F, font);
  drill_cost_label = &addLabel(882.0F, 79.0F, font);
  move_speed_label = &addLabel(882.0F, 106.0F, font);
  move_cost_label = &addLabel(882.0F, 126.0F, font);

  upgrade_drill_button = &place(bar->add_child<asw::ui::Button>(), 1040.0F,
                                69.0F, UPGRADE_SIZE, UPGRADE_SIZE);
  upgrade_move_button = &place(bar->add_child<asw::ui::Button>(), 1040.0F,
                               116.0F, UPGRADE_SIZE, UPGRADE_SIZE);
  for (auto* button : {upgrade_drill_button, upgrade_move_button}) {
    button->set_images(upgrade_texture, nullptr, nullptr, nullptr, false);
  }
  upgrade_drill_button->on_click = [this, &world, &ui]() {
    buyUpgrade(true, world, ui);
  };
  upgrade_move_button->on_click = [this, &world, &ui]() {
    buyUpgrade(false, world, ui);
  };

  // Resources
  auto& resources = place(bar->add_child<ShadowLabel>(), 1110.0F, 21.0F);
  resources.font = fontLarge;
  resources.text = "Resources";

  addLabel(1110.0F, 69.0F, font).text = "Scrap";
  addLabel(1110.0F, 116.0F, font).text = "Biomass";
  scrap_label = &addLabel(1160.0F, 69.0F, font, GREEN);
  biomass_label = &addLabel(1160.0F, 116.0F, font, GREEN);
  for (auto* label : {scrap_label, biomass_label}) {
    label->transform.size = {100.0F, 20.0F};
    label->justify = asw::TextJustify::Right;
  }

  refresh(world, ui);
}

void Toolbar::update(float dt,
                     World& world,
                     asw::ui::Root& ui,
                     bool pointer_used_by_ui,
                     bool world_input) {
  // CHEATING ZONE (DOONT LOOK)
  if (asw::input::get_key_down(asw::input::Key::P)) {
    world.getResourceManager().addResourceCount("scrap", 100);
    world.getResourceManager().addResourceCount("biomass", 100);
  }

  const auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  const auto pointer = controls::pointer();

  pointer_in_world = world_input && !pointerOnToolbar() &&
                     (controls::is_controller_pointer() || !pointer_used_by_ui);

  cursor_idx = pointer_in_world
                   ? tile_map.getIndexAt(camera.screen_to_world(pointer))
                   : asw::Vec3<int>(-1, -1, -1);
  tile_map.setSelectedIndex(cursor_idx);

  // Find distance to worker
  const auto* selected_tile = tile_map.getTileAtIndex(cursor_idx);
  cursor_in_range = false;
  if (selected_tile != nullptr) {
    const auto tile_pos = selected_tile->getPosition();
    const auto distance = asw::Vec3<float>(tile_pos.x, tile_pos.y, tile_pos.z)
                              .distance(world.getPlayer().getPosition());
    cursor_in_range = distance < WORK_RANGE;
  }

  // Outline colour: the tool the next click or trigger would use
  can_take_action =
      controls::is_controller_pointer()
          ? actionEnabled(world, ToolMode::DRILL) || actionEnabled(world, mode)
          : actionEnabled(world, mode);

  drilling = false;

  if (world_input) {
    // Left click and LT use the chosen tool. RT drills whatever tool is
    // chosen, so a controller needs no tool switch to dig
    std::optional<std::pair<ToolMode, float>> active;
    if (asw::input::get_action(controls::USE)) {
      active = {mode, 1.0F};
    } else if (asw::input::get_action(controls::DRILL)) {
      active = {ToolMode::DRILL,
                asw::input::get_action_strength(controls::DRILL)};
    } else if (asw::input::get_action(controls::USE_TOOL)) {
      active = {mode, 1.0F};
    }

    // Drilling works while held. Buildings place once per press, so a drag
    // or a long press does not place a row of them by accident
    const bool pressed = asw::input::get_action_down(controls::USE) ||
                         asw::input::get_action_down(controls::USE_TOOL);
    if (active && active->first != ToolMode::DRILL && !pressed) {
      active.reset();
    }

    if (active && cursor_in_range && pointer_in_world &&
        actionEnabled(world, active->first)) {
      action(world, dt, ui, active->first, active->second);
    } else {
      actionProgress = 0.0F;
    }

    if (asw::input::get_action_down(controls::USE)) {
      explainDisabledAction(world, ui, mode);
    } else if (asw::input::get_action_down(controls::DRILL)) {
      explainDisabledAction(world, ui, ToolMode::DRILL);
    } else if (asw::input::get_action_down(controls::USE_TOOL)) {
      explainDisabledAction(world, ui, mode);
    }

    // A moves the worker, or presses the toolbar button under the cursor
    if (asw::input::get_action_down(controls::WAYPOINT)) {
      pressButtonUnderControllerPointer(world);
    }
    if (asw::input::get_action(controls::WAYPOINT)) {
      setWaypoint(world, ui, asw::input::get_action_down(controls::WAYPOINT));
    }

    // Tool select. Clicks on the tool buttons get the UI sound already
    const auto old_mode = mode;
    if (asw::input::get_action_down(controls::TOOL_DRILL)) {
      selectTool(ToolMode::DRILL);
    } else if (asw::input::get_action_down(controls::TOOL_PURIFIER)) {
      selectTool(ToolMode::PURIFIER);
    } else if (asw::input::get_action_down(controls::TOOL_TREE)) {
      selectTool(ToolMode::TREE);
    } else if (asw::input::get_action_down(controls::TOOL_NEXT)) {
      cycleTool(1);
    } else if (asw::input::get_action_down(controls::TOOL_PREV)) {
      cycleTool(-1);
    }
    if (mode != old_mode) {
      world.getSounds().play("ui");
    }
  } else {
    actionProgress = 0.0F;
  }

  world.getSounds().setDrilling(drilling, actionProgress / 100.0F);

  refresh(world, ui);
}

bool Toolbar::pointerOnToolbar() const {
  return bar != nullptr && bar->transform.contains(controls::pointer());
}

bool Toolbar::isPointedAt(const asw::ui::Widget& widget,
                          const asw::ui::Root& ui) const {
  if (controls::is_controller_pointer()) {
    return widget.transform.contains(controls::pointer()) ||
           (widget.is_focused() && ui.ctx.show_focus);
  }
  return widget.is_highlighted(ui.ctx);
}

void Toolbar::pressButtonUnderControllerPointer(World& world) {
  if (!controls::is_controller_pointer() || !pointerOnToolbar()) {
    return;
  }

  const auto pointer = controls::pointer();
  for (auto* button : {static_cast<asw::ui::Button*>(tool_buttons[0]),
                       static_cast<asw::ui::Button*>(tool_buttons[1]),
                       static_cast<asw::ui::Button*>(tool_buttons[2]),
                       upgrade_drill_button, upgrade_move_button}) {
    // Click sound by hand, the UI only plays it for its own presses
    if (button->enabled && button->transform.contains(pointer) &&
        button->on_click) {
      world.getSounds().play("ui");
      button->on_click();
      return;
    }
  }
}

void Toolbar::selectTool(ToolMode new_mode) {
  mode = new_mode;
  for (std::size_t i = 0; i < tool_buttons.size(); ++i) {
    if (tool_buttons[i] != nullptr) {
      tool_buttons[i]->selected = static_cast<ToolMode>(i) == mode;
    }
  }
}

void Toolbar::cycleTool(int step) {
  const int count = static_cast<int>(tool_buttons.size());
  const int next = (static_cast<int>(mode) + step + count) % count;
  selectTool(static_cast<ToolMode>(next));
}

void Toolbar::buyUpgrade(bool drill, World& world, asw::ui::Root& ui) {
  auto& resource_manager = world.getResourceManager();
  auto& player = world.getPlayer();
  int& cost = drill ? drill_upgrade_cost : move_upgrade_cost;

  if (resource_manager.getResourceCount("scrap") < cost) {
    ui.toast(need_more("scrap", cost));
    world.getSounds().play("blocked");
    return;
  }

  resource_manager.addResourceCount("scrap", -cost);
  cost = static_cast<int>(static_cast<float>(cost) * 1.5F);

  if (drill) {
    player.setDrillSpeed(player.getDrillSpeed() + 1);
    ui.toast("Drill speed is now " + std::to_string(player.getDrillSpeed()));
  } else {
    player.setMoveSpeed(player.getMoveSpeed() + 1);
    ui.toast("Move speed is now " + std::to_string(player.getMoveSpeed()));
  }
}

void Toolbar::setWaypoint(World& world, asw::ui::Root& ui, bool pressed) {
  if (!pointer_in_world ||
      world.getTileMap().getTileAtIndex(cursor_idx) == nullptr) {
    return;
  }

  if (world.setPlayerWaypoint(cursor_idx)) {
    world.setWaypointActive(true);
  } else if (pressed) {
    // Only on the press, since a held button sets the waypoint each frame
    ui.toast("J1M can't get there");
    world.getSounds().play("blocked", controls::pointer().x);
  }
}

void Toolbar::explainDisabledAction(World& world,
                                    asw::ui::Root& ui,
                                    ToolMode tool) {
  if ((actionEnabled(world, tool) && cursor_in_range) || !pointer_in_world ||
      world.getTileMap().getTileAtIndex(cursor_idx) == nullptr) {
    return;
  }

  // Every refused action buzzes, even where there is nothing to explain
  world.getSounds().play("blocked", controls::pointer().x);

  const auto biomass = world.getResourceManager().getResourceCount("biomass");
  if (tool == ToolMode::PURIFIER && biomass < PURIFIER_COST) {
    ui.toast(need_more("biomass", PURIFIER_COST));
  } else if (tool == ToolMode::PURIFIER && isUnderPlayer(world, cursor_idx)) {
    ui.toast("J1M is in the way");
  } else if (tool == ToolMode::TREE && biomass < TREE_COST) {
    ui.toast(need_more("biomass", TREE_COST));
  } else if (!cursor_in_range) {
    ui.toast(controls::is_controller_pointer()
                 ? "Too far away. Press A to move closer"
                 : "Too far away. Right click to move closer");
  }
}

void Toolbar::action(World& world,
                     float dt,
                     asw::ui::Root& ui,
                     ToolMode tool,
                     float strength) {
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  const auto& player = world.getPlayer();

  // Find selected tile
  auto* selected_tile = tile_map.getTileAtIndex(cursor_idx);
  if (selected_tile == nullptr || selected_tile->getType() == nullptr) {
    return;
  }
  const auto select_type = selected_tile->getType();

  if (tool == ToolMode::DRILL) {
    const auto density = select_type->getDensity();
    if (density <= 0) {
      return;
    }

    // this is where the drilling begins
    actionProgress += dt * (30.0F / density) * (player.getDrillSpeed() * 3);
    world.getEffects().drilling(cursor_idx, dt);
    drilling = true;
    // Harder trigger, harder rumble. Trigger motors buzz under the finger
    controls::rumble(0.15F * strength, 0.25F * strength, 100);
    controls::rumble_triggers(0.0F, 0.3F * strength, 100);
    if (actionProgress > 100.0F) {
      controls::rumble(0.6F, 0.8F, 150);
      controls::rumble_triggers(0.0F, 0.8F, 150);
      std::map<std::string, int> drops;
      for (const auto& drop :
           select_type->getActionsOfType(ActionType::DESTROY)) {
        if (!drop.drop_resource_id.empty()) {
          resource_manager.addResourceCount(drop.drop_resource_id, 1);
          ++drops[drop.drop_resource_id];
        }
      }

      // Show what the tile gave, e.g. "+1 scrap  +2 biomass"
      std::string gained;
      for (const auto& [resource, count] : drops) {
        gained += std::format("{}+{} {}", gained.empty() ? "" : "  ", count,
                              resource);
      }
      if (!gained.empty()) {
        world.getEffects().popup(cursor_idx, gained, GREEN);
      }
      world.getEffects().tileBroken(cursor_idx, world.getCamera());
      world.getSounds().play("destroy", controls::pointer().x);
      selected_tile->setType(tile_id::NONE);
      selected_tile->setStructure(nullptr);
      actionProgress = 0.0F;
    }
    return;
  }

  // Build on top of the selected tile
  auto idx = cursor_idx;
  idx.z = idx.z + 1;
  auto* tile = tile_map.getTileAtIndex(idx);
  if (tile == nullptr || tile->getType() != nullptr) {
    return;
  }

  if (tool == ToolMode::PURIFIER) {
    tile->setType(tile_id::PURIFIER);
    resource_manager.addResourceCount("biomass", -PURIFIER_COST);
    tile_map.addPop(idx);
    world.getEffects().built(idx);
    controls::rumble(0.4F, 0.2F, 120);
    world.getSounds().play("human-impact", controls::pointer().x);
    ui.toast("Purifier placed");
  } else if (tool == ToolMode::TREE) {
    tile->setType(tile_id::SAPLING);
    resource_manager.addResourceCount("biomass", -TREE_COST);
    tile_map.addPop(idx);
    world.getEffects().built(idx);
    controls::rumble(0.2F, 0.4F, 80);
    world.getSounds().play("human-impact", controls::pointer().x);
  }
}

bool Toolbar::isUnderPlayer(World& world, const asw::Vec3<int>& tile) {
  const auto& position = world.getPlayer().getPosition();
  return static_cast<int>(std::round(position.x)) == tile.x &&
         static_cast<int>(std::round(position.y)) == tile.y;
}

bool Toolbar::actionEnabled(World& world, ToolMode tool) const {
  const auto& resource_manager = world.getResourceManager();
  const auto* selected_tile = world.getTileMap().getTileAtIndex(cursor_idx);
  if (selected_tile == nullptr || selected_tile->getType() == nullptr) {
    return false;
  }

  const auto type = selected_tile->getTypeId();
  const auto biomass = resource_manager.getResourceCount("biomass");

  switch (tool) {
    case ToolMode::DRILL:
      return selected_tile->getType()->getDensity() > 0;
    case ToolMode::PURIFIER:
      // Not on the worker, which would be trapped inside it
      return biomass >= PURIFIER_COST &&
             (type == tile_id::TOXIC_WATER || type == tile_id::WATER) &&
             !isUnderPlayer(world, cursor_idx);
    case ToolMode::TREE:
      return biomass >= TREE_COST &&
             (type == tile_id::TOXIC_GRASS || type == tile_id::GROUND_GRASS);
  }

  return false;
}

void Toolbar::setInfo(const std::vector<std::string>& lines) {
  for (std::size_t i = 0; i < info_lines.size(); ++i) {
    info_lines[i]->text = i < lines.size() ? lines[i] : "";
  }
}

void Toolbar::refresh(World& world, const asw::ui::Root& ui) {
  const auto& resource_manager = world.getResourceManager();
  const auto& player = world.getPlayer();
  const int scrap = resource_manager.getResourceCount("scrap");
  const int biomass = resource_manager.getResourceCount("biomass");
  const float progression = world.getProgression();

  // Resources
  scrap_label->text = std::to_string(scrap);
  biomass_label->text = std::to_string(biomass);

  // Tools you can not afford are tinted red
  asw::draw::set_tint(purifier_texture,
                      biomass >= PURIFIER_COST ? asw::color::white : CANT_BUY);
  asw::draw::set_tint(tree_texture,
                      biomass >= TREE_COST ? asw::color::white : CANT_BUY);

  // Upgrades
  drill_speed_label->text =
      "Drill Speed: " + std::to_string(player.getDrillSpeed());
  drill_cost_label->text =
      "Cost " + std::to_string(drill_upgrade_cost) + " scrap";
  drill_cost_label->color = scrap >= drill_upgrade_cost ? CAN_BUY : CANT_BUY;
  move_speed_label->text =
      "Move Speed: " + std::to_string(player.getMoveSpeed());
  move_cost_label->text =
      "Cost " + std::to_string(move_upgrade_cost) + " scrap";
  move_cost_label->color = scrap >= move_upgrade_cost ? CAN_BUY : CANT_BUY;

  // Purification
  purification_label->text =
      std::format("Purification: {:.1f}%",
                  progression > 0.99F ? 100.0F : progression * 100.0F);
  purification_bar->value = progression;

  // Info panel
  const auto* selected_tile = world.getTileMap().getTileAtIndex(cursor_idx);

  if (asw::input::get_key(asw::input::Key::Q)) {
    const auto view = world.getCamera().get_view();
    const auto pointer = controls::pointer();
    setInfo({
        "",
        std::format("Pos: {}, {}, {}", cursor_idx.x, cursor_idx.y,
                    cursor_idx.z),
        std::format("Cam: {}, {}", view.position.x, view.position.y),
        std::format("Pointer: {}, {}", pointer.x, pointer.y),
    });
  } else if (pointer_in_world && selected_tile != nullptr &&
             selected_tile->getType() != nullptr) {
    std::vector<std::string> lines = {"Tile: " +
                                      selected_tile->getType()->getName()};

    const auto structure = selected_tile->getStructure();
    if (structure != nullptr && structure->getType() != nullptr) {
      lines.push_back(structure->getType()->name);
      lines.push_back(structure->getType()->description);
    } else {
      lines.insert(lines.end(), {"", ""});
    }

    for (const auto& [res, count] : selected_tile->getType()->getResources()) {
      if (!res.empty()) {
        lines.push_back("Drop: " + res + " x " + std::to_string(count));
      }
    }
    setInfo(lines);
  } else if (isPointedAt(*guide_button, ui)) {
    if (progression < 0.01F) {
      setInfo({"J1M Says:", "I am J1M, your guide.",
               "Right click or A to set a waypoint.",
               "Left click to use a tool.",
               "RT drills, LT uses the chosen tool.",
               "Your goal is to purify the planet."});
    } else if (progression < 0.5F) {
      setInfo({"J1M Says:", "You are doing great!", "Keep up the good work!"});
    } else if (progression < 0.99F) {
      setInfo({"J1M Says:", "You are almost there!", "Keep going!"});
    } else {
      setInfo({"J1M Says:", "You did it!", "You have purified the planet!",
               "Congratulations!"});
    }
  } else if (isPointedAt(*tool_buttons[0], ui)) {
    setInfo({"[Key 1] Drill. Hold RT any time",
             "This tool is used to destroy scrap.",
             "It is very useful for clearing the area ",
             "and collecting resources."});
  } else if (isPointedAt(*tool_buttons[1], ui)) {
    setInfo({"[Key 2] Purifier", "This tool is used to place a water ",
             "purifier. Water purifiers slowly ",
             "purify water and must be placed on water.",
             "Cost: " + std::to_string(PURIFIER_COST) + " biomass"});
  } else if (isPointedAt(*tool_buttons[2], ui)) {
    setInfo({"[Key 3] Tree", "This tool is used to plant trees.",
             "Trees will slowly purify soil.", "",
             "Cost: " + std::to_string(TREE_COST) + " biomass"});
  } else if (isPointedAt(*upgrade_drill_button, ui)) {
    setInfo({"Upgrade Drill", "Drill through tiles faster.",
             "Cost: " + std::to_string(drill_upgrade_cost) + " scrap"});
  } else if (isPointedAt(*upgrade_move_button, ui)) {
    setInfo({"Upgrade Move", "Walk to waypoints faster.",
             "Cost: " + std::to_string(move_upgrade_cost) + " scrap"});
  } else {
    setInfo({});
  }
}

void Toolbar::drawWorld(World& world) {
  const auto view = world.getCamera().get_view();
  const auto screen = asw::display::get_logical_size();
  const auto pointer = controls::pointer();

  drawPlacement(world);

  // Tile under the pointer
  if (pointer_in_world &&
      world.getTileMap().getTileAtIndex(cursor_idx) != nullptr) {
    const auto surface = world.getTileMap().getSurfaceOf(cursor_idx);
    if (cursor_in_range && can_take_action) {
      drawWireframe(surface, view.position, GREEN);
    } else if (cursor_in_range) {
      drawWireframe(surface, view.position, asw::color::white);
    } else {
      drawWireframe(surface, view.position, asw::color::red);
    }
  }

  // Toxic haze over the play area. It fades out smoothly as the world is
  // purified, and is gone at two thirds
  const float progression = world.getProgression();
  const float haze =
      1.0F - asw::easing::smoothstep(std::min(progression / 0.66F, 1.0F));
  if (haze > 0.0F) {
    asw::draw::set_alpha(overlay_1, haze);
    asw::draw::stretch_sprite(
        overlay_1, asw::Quad(0.0F, 0.0F, static_cast<float>(screen.x),
                             static_cast<float>(screen.y) - BAR_HEIGHT));
  }

  // Drill progress
  if (actionProgress > 0) {
    asw::draw::rect_fill(asw::Quad(pointer.x, pointer.y - 60, 208.0F, 38.0F),
                         asw::color::black);

    asw::draw::rect_fill(asw::Quad(pointer.x + 4.0F, pointer.y + 4 - 60,
                                   actionProgress * 2, 30.0F),
                         asw::color::yellow.lerp(asw::Color(55, 255, 0),
                                                 actionProgress / 100.0F));

    asw::draw::text(font, "Drilling...",
                    asw::Vec2(pointer.x + 4.0F, pointer.y - 60 + 4.0F),
                    asw::color::white);
  }
}

void Toolbar::drawPlacement(World& world) {
  if (mode == ToolMode::DRILL || !pointer_in_world) {
    return;
  }

  auto& tile_map = world.getTileMap();
  if (tile_map.getTileAtIndex(cursor_idx) == nullptr) {
    return;
  }

  // Buildings go on top of the tile under the pointer
  auto build_idx = cursor_idx;
  build_idx.z += 1;
  const auto* build_tile = tile_map.getTileAtIndex(build_idx);
  const bool placeable = cursor_in_range && actionEnabled(world, mode) &&
                         build_tile != nullptr &&
                         build_tile->getType() == nullptr;
  const auto colour = placeable ? GREEN : CANT_BUY;
  const auto offset = world.getCamera().get_view().position;

  // A tree's purifying trunk grows over its sapling
  const auto centre = asw::Vec2(cursor_idx.x, cursor_idx.y);

  // Tint the ground in range, and outline its outer edge. The edge follows
  // the ground up and down
  for (int i = centre.x - PURIFY_RANGE; i <= centre.x + PURIFY_RANGE; ++i) {
    for (int j = centre.y - PURIFY_RANGE; j <= centre.y + PURIFY_RANGE; ++j) {
      const auto surface = tile_map.getSurface(asw::Vec2(i, j));
      if (!surface) {
        continue;
      }

      // Left, top, right and bottom corners, on screen
      auto face = surface->face();
      for (auto& point : face) {
        point -= offset;
      }
      asw::draw::polygon_fill(face,
                              asw::Color(colour.r, colour.g, colour.b, 22));

      const auto edge = asw::Color(colour.r, colour.g, colour.b, 200);
      if (i == centre.x - PURIFY_RANGE) {
        asw::draw::line(face[0], face[1], edge);
      }
      if (j == centre.y - PURIFY_RANGE) {
        asw::draw::line(face[1], face[2], edge);
      }
      if (i == centre.x + PURIFY_RANGE) {
        asw::draw::line(face[2], face[3], edge);
      }
      if (j == centre.y + PURIFY_RANGE) {
        asw::draw::line(face[3], face[0], edge);
      }
    }
  }

  // Ghost of the building, red where it can not go
  const auto type = TileDictionary::getTile(
      mode == ToolMode::PURIFIER ? tile_id::PURIFIER : tile_id::SAPLING);
  if (type != nullptr && build_tile != nullptr) {
    type->drawGhost(build_idx, offset,
                    placeable ? asw::color::white : CANT_BUY, 0.55F);
  }
}

void Toolbar::drawWireframe(const Surface& surface,
                            const asw::Vec2<float>& offset,
                            asw::Color colour) {
  // Top of the ground, in screen space
  auto face = surface.face();
  for (auto& point : face) {
    point -= offset;
  }

  asw::draw::polygon_fill(face, asw::Color(colour.r, colour.g, colour.b, 60));
  asw::draw::polygon(face, colour);
}

void Toolbar::save(nlohmann::json& data) const {
  data["upgrade_costs"] = {{"drill", drill_upgrade_cost},
                           {"move", move_upgrade_cost}};
}

void Toolbar::load(const nlohmann::json& data) {
  const auto& costs = data.at("upgrade_costs");
  drill_upgrade_cost = costs.at("drill").get<int>();
  move_upgrade_cost = costs.at("move").get<int>();
}
