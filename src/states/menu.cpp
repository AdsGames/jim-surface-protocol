#include "menu.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"
#include "../tiles/tile_map.h"

void Menu::init() {
  background = asw::assets::loadTexture("assets/images/ui/menu.png");
  font = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 96);
  font_small = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 64);
  font_button = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 32);
  jim = asw::assets::loadTexture("assets/images/ui/j1m.png");

  // Load jims
  jims[0] = asw::assets::loadTexture("assets/images/player/full/1.png");
  jims[1] = asw::assets::loadTexture("assets/images/player/full/6.png");
  jims[2] = asw::assets::loadTexture("assets/images/player/full/2.png");
  jims[3] = asw::assets::loadTexture("assets/images/player/full/3.png");
  jims[4] = asw::assets::loadTexture("assets/images/player/full/5.png");
  jims[5] = asw::assets::loadTexture("assets/images/player/full/4.png");
}

void Menu::update(float dt) {
  timer += dt;
  frame = static_cast<int>(std::floor(timer / 1000.0F)) % 6;

  if (asw::input::wasKeyPressed(asw::input::Key::ESCAPE)) {
    asw::core::exit = true;
  }

  if (asw::input::wasButtonPressed(asw::input::MouseButton::LEFT)) {
    auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
    if (start_easy_transform.contains(mouse_pos)) {
      TileMap::MAP_WIDTH = 40;
      TileMap::MAP_DEPTH = 40;
      TileMap::SEED = 600.0F;
      sceneManager.setNextScene(ProgramState::Game);
    } else if (start_hard_transform.contains(mouse_pos)) {
      TileMap::MAP_WIDTH = 80;
      TileMap::MAP_DEPTH = 80;
      TileMap::SEED = asw::random::between(0.0F, 10000.0F);
      sceneManager.setNextScene(ProgramState::Game);
    } else if (exit_transform.contains(mouse_pos)) {
      asw::core::exit = true;
    } else if (fullscreen_transform.contains(mouse_pos)) {
      fullscreen = !fullscreen;
      asw::display::setFullscreen(fullscreen);
    }
  }

  if (asw::input::wasKeyPressed(asw::input::Key::F11)) {
    fullscreen = !fullscreen;
    asw::display::setFullscreen(fullscreen);
  }
}

void Menu::draw() {
  asw::draw::sprite(background, asw::Vec2(0.0F, 0.0F));

  asw::draw::setBlendMode(jim, asw::BlendMode::MULTIPLY);
  asw::draw::stretchSprite(
      jim, asw::Quad(-200.0F, (std::sin(timer / 1000.0F) + 1) * 200.0F, 2000.0F,
                     2000.0F));

  asw::draw::stretchSprite(jims[frame],
                           asw::Quad(700.0F, 500.0F, 500.0F, 500.0F));
  asw::draw::setBlendMode(jim, asw::BlendMode::BLEND);

  asw::draw::text(font, "J1M:", asw::Vec2(40.0F, 520.0F),
                  asw::util::makeColor(255, 255, 255));
  asw::draw::text(font_small, "Surface Protocol", asw::Vec2(40.0F, 640.0F),
                  asw::util::makeColor(255, 255, 255));

  // Start easy
  asw::draw::text(font_button, "Quickplay", start_easy_transform.position,
                  asw::util::makeColor(255, 255, 255));

  // Start hard
  asw::draw::text(font_button, "Full Game", start_hard_transform.position,
                  asw::util::makeColor(255, 255, 255));

  // Fullscreen
  asw::draw::text(font_button, "Fullscreen", fullscreen_transform.position,
                  asw::util::makeColor(255, 255, 255));

  // Exit
  asw::draw::text(font_button, "Exit", exit_transform.position,
                  asw::util::makeColor(255, 255, 255));
}

void Menu::cleanup() {
  asw::sound::stopMusic();
}