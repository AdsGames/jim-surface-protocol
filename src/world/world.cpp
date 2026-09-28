#include "world.h"

#include <algorithm>

#include "../lib/controls.h"
#include "toolbar.h"

namespace {
// Camera scroll per update, in pixels
constexpr float CAMERA_PAN_SPEED = 10.0F;
}  // namespace

void World::init() {
  resource_manager.load("assets/resources.json");
  tile_map.generate();
  sound_orchestrator.init();

  waypointTexture =
      asw::assets::load_texture("assets/images/player/waypoint.png");
  shadowTexture =
      asw::assets::load_texture("assets/images/player/128/shadow.png");

  // Keep the view over the map, with room below it for the toolbar
  const auto min_x = -(TileMap::MAP_DEPTH / 2.0F) * TILE_SIZE;
  const auto max_x = (TileMap::MAP_WIDTH / 2.0F + 1) * TILE_SIZE;
  const auto min_y = -(MAP_HEIGHT / 2.0F) * TILE_SIZE;
  const auto max_y =
      std::max(TileMap::MAP_WIDTH / 2.0F + 1, TileMap::MAP_DEPTH / 2.0F + 1) *
          TILE_SIZE +
      TOOLBAR_HEIGHT;

  const auto screen_size = asw::display::get_logical_size();
  camera = asw::Camera(asw::Vec2<float>(static_cast<float>(screen_size.x),
                                        static_cast<float>(screen_size.y)));
  camera.set_bounds(asw::Quad<float>(min_x, min_y, max_x - min_x, max_y - min_y));
  camera.set_position(asw::Vec2<float>(-640.0F, -480.0F));
}

void World::update(float dt) {
  // Keyboard and controller movement
  auto pan = asw::Vec2<float>(0.0F, 0.0F);
  if (asw::input::get_action(controls::CAMERA_LEFT)) {
    pan.x -= CAMERA_PAN_SPEED;
  }
  if (asw::input::get_action(controls::CAMERA_RIGHT)) {
    pan.x += CAMERA_PAN_SPEED;
  }
  if (asw::input::get_action(controls::CAMERA_UP)) {
    pan.y -= CAMERA_PAN_SPEED;
  }
  if (asw::input::get_action(controls::CAMERA_DOWN)) {
    pan.y += CAMERA_PAN_SPEED;
  }

  // Mouse movement
  auto screen_size = asw::display::get_logical_size();
  const auto& mouse_pos = asw::input::get_mouse().position;
  if (mouse_pos.x >= screen_size.x - 4) {
    pan.x += CAMERA_PAN_SPEED;
  }
  if (mouse_pos.x <= 4) {
    pan.x -= CAMERA_PAN_SPEED;
  }
  if (mouse_pos.y >= screen_size.y - 4) {
    pan.y += CAMERA_PAN_SPEED;
  }
  if (mouse_pos.y <= 4) {
    pan.y -= CAMERA_PAN_SPEED;
  }

  camera.set_position(camera.get_position() + pan);

  // Regenerate map
  if (asw::input::get_key_down(asw::input::Key::G)) {
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
  const auto view = camera.get_view();
  asw::draw::rect_fill(asw::Quad(0.0F, 0.0F, view.size.x, view.size.y),
                       asw::Color(0, 64, blue_percent));

  // Get player z position
  const auto player_position =
      asw::Vec3(static_cast<int>(std::round(player.getPosition().x)),
                static_cast<int>(std::round(player.getPosition().y)),
                static_cast<int>(std::round(player.getPosition().z)));

  tile_map.draw(view, asw::Vec3(0, 0, 0), player_position);
  player.draw(view.position);
  tile_map.draw(view, player_position,
                asw::Vec3(TileMap::MAP_WIDTH, TileMap::MAP_DEPTH, MAP_HEIGHT));

  if (waypointActive) {
    // Draw player waypoint
    auto player_waypoint = getPlayerWaypoint();
    auto player_waypoint_screen = camera.world_to_screen(
        asw::Vec2(isoX(player_waypoint), isoY(player_waypoint)) * TILE_HEIGHT);

    asw::draw::sprite(waypointTexture,
                      player_waypoint_screen + asw::Vec2<float>(0, -48));

    auto loc = player_waypoint_screen + asw::Vec2<float>(6, -16);
    auto shadow_transform = asw::Quad<float>(loc.x, loc.y, 48, 48);
    asw::draw::stretch_sprite(shadowTexture,
                              shadow_transform);  // Shadow
  }
}

Worker& World::getPlayer() {
  return player;
}
