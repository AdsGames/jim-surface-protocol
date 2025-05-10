#include "world.h"

World::World() {
  waypointTexture =
      asw::assets::loadTexture("assets/images/player/waypoint.png");
  shadowTexture =
      asw::assets::loadTexture("assets/images/player/128/shadow.png");
}

void World::init() {
  resource_manager.load("assets/resources.json");
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

  if (asw::input::mouse.x >= screen_size.x - 2) {
    camera.position.x += 10;
  }
  if (asw::input::mouse.x <= 2) {
    camera.position.x -= 10;
  }
  if (asw::input::mouse.y >= screen_size.y - 2) {
    camera.position.y += 10;
  }
  if (asw::input::mouse.y <= 2) {
    camera.position.y -= 10;
  }

  // Regenerate map
  if (asw::input::wasKeyPressed(asw::input::Key::G)) {
    tile_map.generate();
  }

  tile_map.update(dt);

  player.update(dt, *this);
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

  if (waypointActive) {
    // Draw player waypoint
    auto player_waypoint = getPlayerWaypoint();
    auto player_waypoint_screen =
        asw::Vec2(isoX(player_waypoint), isoY(player_waypoint)) * TILE_HEIGHT -
        camera.position;

    asw::draw::sprite(waypointTexture,
                      player_waypoint_screen + asw::Vec2<float>(0, -48));

    auto loc = player_waypoint_screen + asw::Vec2<float>(6, -16);
    auto shadow_transform = asw::Quad<float>(loc.x, loc.y, 48, 48);
    asw::draw::stretchSprite(shadowTexture,
                             shadow_transform);  // Shadow
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

Worker* World::getPlayer() {
  return &player;
}
