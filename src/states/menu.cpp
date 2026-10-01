#include "menu.h"

#include <asw/asw.h>
#include <algorithm>

#include "../lib/controls.h"
#include "../lib/save_game.h"

#include "../tiles/tile_map.h"

namespace {
constexpr float BUTTON_WIDTH = 300.0F;
constexpr float BUTTON_HEIGHT = 40.0F;
}  // namespace

void Menu::init() {
  background = asw::assets::load_texture("assets/images/ui/menu.png");
  font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 96);
  font_small = asw::assets::load_font("assets/fonts/syne-mono.ttf", 64);
  font_button = asw::assets::load_font("assets/fonts/syne-mono.ttf", 32);
  jim = asw::assets::load_texture("assets/images/ui/j1m.png");

  // Load jims
  jims[0] = asw::assets::load_texture("assets/images/player/full/1.png");
  jims[1] = asw::assets::load_texture("assets/images/player/full/6.png");
  jims[2] = asw::assets::load_texture("assets/images/player/full/2.png");
  jims[3] = asw::assets::load_texture("assets/images/player/full/3.png");
  jims[4] = asw::assets::load_texture("assets/images/player/full/5.png");
  jims[5] = asw::assets::load_texture("assets/images/player/full/4.png");

  // The scene is reused, so rebuild the UI from nothing
  ui.root.clear_children();
  ui.ctx.theme.font = font_button;
  ui.ctx.navigation = controls::menu_navigation(controls::BACK);
  ui.on_back = []() { asw::core::exit(); };

  auto& title = ui.root.add_child<asw::ui::Label>();
  title.font = font;
  title.text = "J1M:";
  title.anchor = asw::ui::Anchor::TopLeft;
  title.anchor_margin = {40.0F, 520.0F};

  auto& subtitle = ui.root.add_child<asw::ui::Label>();
  subtitle.font = font_small;
  subtitle.text = "Surface Protocol";
  subtitle.anchor = asw::ui::Anchor::TopLeft;
  subtitle.anchor_margin = {40.0F, 640.0F};

  // Buttons, bottom left
  auto& list = ui.root.add_child<asw::ui::Stack>();
  list.anchor = asw::ui::Anchor::BottomLeft;
  list.anchor_margin = {40.0F, 20.0F};
  list.align = asw::ui::Align::Start;
  list.gap = 0.0F;

  auto& continue_button = addButton(list, "Continue", [this]() {
    save_game::load_requested = true;
    manager.set_next_scene(ProgramState::Game);
  });
  continue_button.enabled = save_game::exists();

  auto& quickplay_button =
      addButton(list, "Quickplay", [this]() { startNewGame(40, 600.0F); });
  addButton(list, "Full Game", [this]() {
    startNewGame(80, asw::random::between(0.0F, 10000.0F));
  });
  addButton(list, "Fullscreen", [this]() { toggleFullscreen(); });
  addButton(list, "Exit", []() { asw::core::exit(); });

  // Size the list to its buttons, so the anchor can place it
  list.transform.size = {
      BUTTON_WIDTH, BUTTON_HEIGHT * static_cast<float>(list.children().size())};

  // Controllers start on the first button they can press
  ui.focus(continue_button.enabled ? continue_button : quickplay_button);
}

asw::ui::Button& Menu::addButton(asw::ui::Stack& list,
                                 const std::string& text,
                                 std::function<void()> on_click) {
  asw::ui::ButtonStyle style;
  style.bg = asw::Color(0, 0, 0, 0);
  style.text_hover = asw::color::yellow;
  style.text_disabled = asw::Color(110, 110, 110);
  style.text_align = asw::TextJustify::Left;

  auto& button = list.add_child<asw::ui::Button>();
  button.text = text;
  button.font = font_button;
  button.draw_background = false;
  button.focus_ring = false;
  button.style = style;
  button.transform.size = {BUTTON_WIDTH, BUTTON_HEIGHT};
  button.on_click = std::move(on_click);
  return button;
}

void Menu::startNewGame(int size, float seed) {
  size = asw::config::get_int("game.map_size").value_or(size);
  size = std::clamp(size, 10, std::min(MAX_MAP_WIDTH, MAX_MAP_DEPTH));

  TileMap::MAP_WIDTH = size;
  TileMap::MAP_DEPTH = size;
  TileMap::SEED = asw::config::get_float("game.seed").value_or(seed);
  save_game::load_requested = false;
  manager.set_next_scene(ProgramState::Game);
}

void Menu::toggleFullscreen() {
  fullscreen = !fullscreen;
  asw::display::set_fullscreen(fullscreen);
}

void Menu::update(float dt) {
  timer += dt;
  frame = static_cast<int>(std::floor(timer)) % 6;

  ui.update();

  if (asw::input::get_action_down(controls::FULLSCREEN)) {
    toggleFullscreen();
  }
}

void Menu::draw() {
  asw::draw::sprite(background, asw::Vec2(0.0F, 0.0F));

  asw::draw::set_blend_mode(jim, asw::BlendMode::Multiply);
  asw::draw::stretch_sprite(
      jim,
      asw::Quad(-200.0F, (std::sin(timer) + 1) * 200.0F, 2000.0F, 2000.0F));

  asw::draw::stretch_sprite(jims[frame],
                            asw::Quad(700.0F, 500.0F, 500.0F, 500.0F));
  asw::draw::set_blend_mode(jim, asw::BlendMode::Blend);

  ui.draw();
}

void Menu::cleanup() {
  asw::sound::stop_music();
}