#include "toolbar.h"

void Toolbar::init() {
  font = asw::assets::loadFont("assets/fonts/ariblk.ttf", 16);
};

void Toolbar::update(float dt, World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();

  auto world_pos = camera.position +
                   asw::Vec2<float>(asw::input::mouse.x, asw::input::mouse.y);

  auto world_idx = tile_map.getIndexAt(world_pos);

  selected_tile = tile_map.getTileAt(world_pos);

  // Spawn worker
  if (asw::input::wasKeyPressed(asw::input::Key::W)) {
    world.addWorker(world_idx);
  }
}

void Toolbar::draw(World& world) {
  auto& camera = world.getCamera();

  if (selected_tile != nullptr && selected_tile->getType() != nullptr) {
    asw::draw::text(font, "Tile: " + selected_tile->getType()->getName(),
                    asw::Vec2<float>(10, 80),
                    asw::util::makeColor(255, 255, 255));

    selected_tile->drawWireframe(camera.position);
  }

  asw::draw::rectFill(
      asw::Quad<float>(0, camera.size.y - 80, camera.size.x, camera.size.y),
      asw::util::makeColor(255, 0, 255));
}