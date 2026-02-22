#include "menu.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"
#include "../tiles/tile_map.h"

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
}

void Menu::update(float dt) {
  timer += dt;
  frame = static_cast<int>(std::floor(timer)) % 6;

  if (asw::input::get_key_down(asw::input::Key::Escape)) {
    asw::core::exit = true;
  }

  if (asw::input::get_mouse_button_down(asw::input::MouseButton::Left)) {
    const auto& mouse_pos = asw::input::mouse.position;
    if (start_easy_transform.contains(mouse_pos)) {
      TileMap::MAP_WIDTH = 40;
      TileMap::MAP_DEPTH = 40;
      TileMap::SEED = 600.0F;
      manager.set_next_scene(ProgramState::Game);
    } else if (start_hard_transform.contains(mouse_pos)) {
      TileMap::MAP_WIDTH = 80;
      TileMap::MAP_DEPTH = 80;
      TileMap::SEED = asw::random::between(0.0F, 10000.0F);
      manager.set_next_scene(ProgramState::Game);
    } else if (exit_transform.contains(mouse_pos)) {
      asw::core::exit = true;
    } else if (fullscreen_transform.contains(mouse_pos)) {
      fullscreen = !fullscreen;
      asw::display::set_fullscreen(fullscreen);
    }
  }

  if (asw::input::get_key_down(asw::input::Key::F11)) {
    fullscreen = !fullscreen;
    asw::display::set_fullscreen(fullscreen);
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

  asw::draw::text(font, "J1M:", asw::Vec2(40.0F, 520.0F), asw::color::white);
  asw::draw::text(font_small, "Surface Protocol", asw::Vec2(40.0F, 640.0F),
                  asw::color::white);

  // Start easy
  asw::draw::text(font_button, "Quickplay", start_easy_transform.position,
                  asw::color::white);

  // Start hard
  asw::draw::text(font_button, "Full Game", start_hard_transform.position,
                  asw::color::white);

  // Fullscreen
  asw::draw::text(font_button, "Fullscreen", fullscreen_transform.position,
                  asw::color::white);

  // Exit
  asw::draw::text(font_button, "Exit", exit_transform.position,
                  asw::color::white);
}

void Menu::cleanup() {
  asw::sound::stop_music();
}