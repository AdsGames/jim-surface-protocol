#include "world.h"

#include <algorithm>
#include <cmath>

#include "../lib/controls.h"
#include "../tiles/tile_ids.h"
#include "toolbar.h"

namespace {
// Camera scroll at full speed, in pixels per second
constexpr float CAMERA_PAN_SPEED = 600.0F;

// Seconds for the camera pan to reach full speed, or to stop
constexpr float CAMERA_PAN_RAMP_TIME = 0.2F;

// Seconds in one day, unless the config file sets game.day_length
constexpr float DEFAULT_DAY_LENGTH = 240.0F;
}  // namespace

void World::init() {
  resource_manager.load("assets/resources.json");
  tile_map.generate();
  sound_orchestrator.init();

  player.setPosition({0, 0, 0});
  player.setDrillSpeed(1);
  player.setMoveSpeed(1);
  playerWaypoint = {0, 0, 0};
  waypointActive = false;

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
  camera.set_bounds(
      asw::Quad<float>(min_x, min_y, max_x - min_x, max_y - min_y));
  camera.set_position(asw::Vec2<float>(-640.0F, -480.0F));

  pan_direction = asw::Vec2<float>(0.0F, 0.0F);
  pan_ramp = 0.0F;

  const auto day_length =
      asw::config::get_float("game.day_length").value_or(DEFAULT_DAY_LENGTH);
  lighting.init(camera, day_length > 0.0F ? day_length : DEFAULT_DAY_LENGTH);
}

void World::update(float dt, bool input) {
  auto pan = asw::Vec2<float>(0.0F, 0.0F);

  if (input) {
    // Keyboard and controller movement
    if (asw::input::get_action(controls::CAMERA_LEFT)) {
      pan.x -= 1.0F;
    }
    if (asw::input::get_action(controls::CAMERA_RIGHT)) {
      pan.x += 1.0F;
    }
    if (asw::input::get_action(controls::CAMERA_UP)) {
      pan.y -= 1.0F;
    }
    if (asw::input::get_action(controls::CAMERA_DOWN)) {
      pan.y += 1.0F;
    }

    // Pointer at the screen edge, mouse or controller cursor
    const auto screen_size = asw::display::get_logical_size();
    const auto pointer = controls::pointer();
    if (pointer.x >= screen_size.x - 4) {
      pan.x += 1.0F;
    }
    if (pointer.x <= 4) {
      pan.x -= 1.0F;
    }
    if (pointer.y >= screen_size.y - 4) {
      pan.y += 1.0F;
    }
    if (pointer.y <= 4) {
      pan.y -= 1.0F;
    }
  }

  // Ease the pan in and out. On release, keep going the last way to stop
  pan.x = std::clamp(pan.x, -1.0F, 1.0F);
  pan.y = std::clamp(pan.y, -1.0F, 1.0F);
  const float ramp_step = dt / CAMERA_PAN_RAMP_TIME;
  if (pan.x != 0.0F || pan.y != 0.0F) {
    pan_direction = pan;
    pan_ramp = std::min(pan_ramp + ramp_step, 1.0F);
  } else {
    pan_ramp = std::max(pan_ramp - ramp_step, 0.0F);
  }

  const float pan_speed =
      CAMERA_PAN_SPEED * asw::easing::smoothstep(pan_ramp) * dt;
  camera.set_position(camera.get_position() + pan_direction * pan_speed);

  if (input && asw::input::get_action_down(controls::CENTER_CAMERA)) {
    centerCameraOnPlayer();
  }

  // Regenerate map
  if (asw::input::get_key_down(asw::input::Key::G)) {
    tile_map.generate();
  }

  tile_map.update(dt);

  player.update(dt, *this);

  // Maybe this should go somewhere else :thonk:
  // Count toxins
  const float toxic_count = tile_map.countByType(tile_id::TOXIC_WATER) +
                            tile_map.countByType(tile_id::TOXIC_GRASS) +
                            tile_map.countByType(tile_id::TOXIC_SOIL);

  const float non_toxic_count = tile_map.countByType(tile_id::WATER) +
                                tile_map.countByType(tile_id::GROUND_GRASS) +
                                tile_map.countByType(tile_id::GROUND);

  // Calculate progression
  progression = (non_toxic_count) / (toxic_count + non_toxic_count + 0.1F);

  // Orch
  sound_orchestrator.update(dt, *this);

  lighting.update(dt, *this);
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

  lighting.draw();
}

void World::centerCameraOnPlayer() {
  const auto& position = player.getPosition();
  const auto player_world =
      asw::Vec2(isoXf(position) * TILE_HEIGHT_F + TILE_HEIGHT_F,
                isoYf(position) * TILE_HEIGHT_F + TILE_HEIGHT_F);

  // The toolbar covers the bottom of the view, so centre on the rest
  const auto view_size = camera.get_view().size;
  const auto visible_center =
      asw::Vec2(view_size.x / 2.0F, (view_size.y - TOOLBAR_HEIGHT) / 2.0F);

  camera.set_position(player_world - visible_center);

  // Stop any pan that was easing out, so the jump holds
  pan_ramp = 0.0F;
}

Worker& World::getPlayer() {
  return player;
}

void World::save(nlohmann::json& data) const {
  const auto& position = player.getPosition();
  const auto camera_position = camera.get_position();

  data["map"] = tile_map.save();
  data["player"] = {
      {"position", {position.x, position.y, position.z}},
      {"drill_speed", player.getDrillSpeed()},
      {"move_speed", player.getMoveSpeed()},
  };
  data["camera"] = {camera_position.x, camera_position.y};
  data["time"] = lighting.getTime();

  for (const auto& [id, resource] : resource_manager.getResources()) {
    data["resources"][id] = resource->amount;
  }
}

void World::load(const nlohmann::json& data) {
  tile_map.load(data.at("map"));

  const auto& player_data = data.at("player");
  const auto& position = player_data.at("position");
  const auto player_index =
      asw::Vec3<int>(static_cast<int>(std::round(position.at(0).get<float>())),
                     static_cast<int>(std::round(position.at(1).get<float>())),
                     static_cast<int>(std::round(position.at(2).get<float>())));
  player.setPosition(player_index);
  player.setDrillSpeed(player_data.at("drill_speed").get<int>());
  player.setMoveSpeed(player_data.at("move_speed").get<int>());

  // Stay put rather than walk to the old waypoint
  playerWaypoint = player_index;
  waypointActive = false;

  const auto& camera_position = data.at("camera");
  camera.set_position(asw::Vec2<float>(camera_position.at(0).get<float>(),
                                       camera_position.at(1).get<float>()));

  lighting.setTime(data.at("time").get<float>());

  for (const auto& [id, amount] : data.at("resources").items()) {
    const auto current = resource_manager.getResourceCount(id);
    resource_manager.addResourceCount(id, amount.get<int>() - current);
  }
}
