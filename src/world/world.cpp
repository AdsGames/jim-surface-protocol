#include "world.h"

void World::init() {
  resource_manager.load("assets/resources.json");
  tile_map.generate();
  sound_orchestrator.init();

  waypointTexture =
      asw::assets::loadTexture("assets/images/player/waypoint.png");
  shadowTexture =
      asw::assets::loadTexture("assets/images/player/128/shadow.png");
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
  if (asw::input::mouse.x >= screen_size.x - 4) {
    camera.position.x += 10;
  }
  if (asw::input::mouse.x <= 4) {
    camera.position.x -= 10;
  }
  if (asw::input::mouse.y >= screen_size.y - 4) {
    camera.position.y += 10;
  }
  if (asw::input::mouse.y <= 4) {
    camera.position.y -= 10;
  }

  // Topmost
  const auto min_x = -(TileMap::MAP_DEPTH / 2.0F) * TILE_SIZE;
  const auto max_x = (TileMap::MAP_WIDTH / 2.0F + 1) * TILE_SIZE;
  const auto min_y = -(MAP_HEIGHT / 2.0F) * TILE_SIZE;
  const auto max_y =
      std::max(TileMap::MAP_WIDTH / 2.0F + 1, TileMap::MAP_DEPTH / 2.0F + 1) *
          TILE_SIZE +
      160.0F;

  if (camera.position.y + camera.size.y > max_y) {
    camera.position.y = max_y - camera.size.y;
  }
  if (camera.position.y < min_y) {
    camera.position.y = min_y;
  }
  if (camera.position.x + camera.size.x > max_x) {
    camera.position.x = max_x - camera.size.x;
  }
  if (camera.position.x < min_x) {
    camera.position.x = min_x;
  }

  // Regenerate map
  if (asw::input::wasKeyPressed(asw::input::Key::G)) {
    tile_map.generate();
  }

  tile_map.update(dt);

  player.update(dt, *this);

  // Maybe this should go somewhere else :thonk:
  // Count toxins
  const float toxic_count = tile_map.countByType(18) +
                            tile_map.countByType(16) + tile_map.countByType(17);

  const float non_toxic_count = tile_map.countByType(5) +
                                tile_map.countByType(12) +
                                tile_map.countByType(13);

  // Calculate progression
  progression = (non_toxic_count) / (toxic_count + non_toxic_count + 0.1F);

  // Orch
  sound_orchestrator.update(dt, *this);
}

void World::draw() {
  const int blue_percent = 64 * (progression + 1.0F);
  asw::draw::rectFill(asw::Quad(0.0F, 0.0F, camera.size.x, camera.size.y),
                      asw::util::makeColor(0, 64, blue_percent));

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
