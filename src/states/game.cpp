#include "game.h"

#include <string>
#include <vector>

#include "../tiles/tile_dictionary.h"

void Game::init() {
  world.init();
  toolbar.init();

  font = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 32);
  font_small = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 16);
}

void Game::update(float dt) {
  world.update(dt);
  toolbar.update(dt, world);

  if (asw::input::wasKeyPressed(asw::input::Key::ESCAPE)) {
    sceneManager.setNextScene(ProgramState::Menu);
  }
}

void Game::draw() {
  world.draw();
  toolbar.draw(world);

  asw::draw::text(font, "J1M Surface Protocol", asw::Vec2(10.0F, 10.0F),
                  asw::util::makeColor(255, 255, 255));

  if (world.getProgression() < 0.33F) {
    asw::draw::text(font_small, "The world is toxic. Clean it up.",
                    asw::Vec2(10.0F, 45.0F), asw::util::makeColor(255, 0, 0));
  } else if (world.getProgression() < 0.66F) {
    asw::draw::text(font_small, "The world is getting cleaner.",
                    asw::Vec2(10.0F, 45.0F), asw::util::makeColor(255, 255, 0));
  } else if (world.getProgression() < 0.99F) {
    asw::draw::text(font_small, "Almost there! Keep going.",
                    asw::Vec2(10.0F, 45.0F), asw::util::makeColor(0, 255, 255));
  } else {
    asw::draw::text(font_small, "You win! The world is clean.",
                    asw::Vec2(10.0F, 45.0F), asw::util::makeColor(0, 255, 0));
  }
}
