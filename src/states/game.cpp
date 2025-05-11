#include "game.h"

#include <string>
#include <vector>

#include "../tiles/tile_dictionary.h"

void Game::init() {
  world.init();
  toolbar.init();

  font = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 32);
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
}
