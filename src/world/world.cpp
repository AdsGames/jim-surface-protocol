#include "world.h"

void World::init() {
  tile_map.generate();
}

void World::update(float dt) {
  // Keybo movement
  if (asw::input::isKeyDown(asw::input::Key::A)) {
    camera.position.x -= 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::D)) {
    camera.position.x += 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::W)) {
    camera.position.y -= 10;
  }
  if (asw::input::isKeyDown(asw::input::Key::S)) {
    camera.position.y += 10;
  }

  // Mouse movement
  auto screen_size = asw::display::getLogicalSize();
  screen_size.y -= 80;  // Remove title bar height

  if (asw::input::mouse.x >= 0 && asw::input::mouse.x <= screen_size.x &&
      asw::input::mouse.y >= 0 && asw::input::mouse.y <= screen_size.y) {
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
  }

  // Regenerate map
  if (asw::input::wasKeyPressed(asw::input::Key::G)) {
    tile_map.generate();
  }

  tile_map.update(dt);

  player.update(dt,*this);
}

void World::draw() {
  asw::draw::rectFill(asw::Quad(0.0F, 0.0F, camera.size.x, camera.size.y),
                      asw::util::makeColor(0, 64, 128));

  tile_map.draw(camera);
  player.draw(camera.position);

  // Draw workers
  for (auto& [id, worker] : workers) {
    worker.draw(camera.position);
  }
}

WorkerId World::addWorker(const asw::Vec3<int>& position) {
  auto worker = Worker();
  worker.setPosition(position);
  workers[worker.getId()] = worker;

  return worker.getId();
}

Worker* World::getWorker(WorkerId id) {
  auto it = workers.find(id);
  if (it != workers.end()) {
    return &it->second;
  }
  return nullptr;
}