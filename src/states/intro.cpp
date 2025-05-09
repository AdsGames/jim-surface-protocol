#include "intro.h"

#include <string>
#include <vector>

#include "../tiles/tile_dictionary.h"

void Intro::init() {
  tile_map.generate();
  timer.start();

  font = asw::assets::loadFont("assets/fonts/ariblk.ttf", 16);
}

void Intro::update(float dt) {
  frame = (timer.getElapsedTime<std::chrono::milliseconds>() - 3000) / 100;

  // Keybo movement
  if (asw::input::isKeyDown(asw::input::Key::LEFT)) {
    camera.position.x -= 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::RIGHT)) {
    camera.position.x += 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::UP)) {
    camera.position.y -= 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::DOWN)) {
    camera.position.y += 10;
  }

  // Mouse movement
  auto screen_size = asw::display::getLogicalSize();

  if (asw::input::mouse.x > screen_size.x * 0.9) {
    camera.position.x += 10;
  }
  if (asw::input::mouse.x < screen_size.x * 0.1) {
    camera.position.x -= 10;
  }
  if (asw::input::mouse.y > screen_size.y * 0.9) {
    camera.position.y += 10;
  }
  if (asw::input::mouse.y < screen_size.y * 0.1) {
    camera.position.y -= 10;
  }

  if (asw::input::wasKeyPressed(asw::input::Key::G)) {
    tile_map.generate();
  }

  tile_map.update(dt);
}

void Intro::draw() {
  asw::draw::rectFill(asw::Quad<float>(0, 0, camera.size.x, camera.size.y),
                      asw::util::makeColor(0, 64, 128));

  tile_map.draw(camera);

  asw::draw::text(font, "Tojam 2025", asw::Vec2<float>(400, 300),
                  asw::util::makeColor(255, 255, 255));

  asw::draw::text(font, "Mouse Z: " + std::to_string(asw::input::mouse.z),
                  asw::Vec2<float>(400, 400),
                  asw::util::makeColor(255, 255, 255));

  std::string name = "air";
  Tile* tileAtMouse =
      tile_map.tileAt(camera.position + asw::Vec2<float>(asw::input::mouse.x,
                                                         asw::input::mouse.y));
  if (tileAtMouse != nullptr && tileAtMouse->getType() != nullptr) {
    name = tileAtMouse->getType()->getName();
    tileAtMouse->drawWireframe(camera.position);
  }

  asw::draw::text(font, "Tile: " + name, asw::Vec2<float>(400, 450),
                  asw::util::makeColor(255, 255, 255));
}
