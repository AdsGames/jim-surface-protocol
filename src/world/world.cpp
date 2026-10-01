#include "world.h"

#include <algorithm>
#include <cmath>

#include "../lib/controls.h"
#include "../tasks/pathfinder.h"
#include "../tiles/tile_ids.h"
#include "toolbar.h"

namespace {
// Camera scroll at full speed, in pixels per second
constexpr float CAMERA_PAN_SPEED = 600.0F;

// Seconds for the camera pan to reach full speed, or to stop
constexpr float CAMERA_PAN_RAMP_TIME = 0.2F;

// Seconds in one day, unless the config file sets game.day_length
constexpr float DEFAULT_DAY_LENGTH = 240.0F;

// Seconds for the waypoint marker to drop in, and how far it falls
constexpr float WAYPOINT_DROP_TIME = 0.6F;
constexpr float WAYPOINT_DROP_HEIGHT = 90.0F;

// Route marks: pixels apart, how fast they march to the waypoint in pixels
// per second, and how far they take to fade in at each end
constexpr float ROUTE_SPACING = 18.0F;
constexpr float ROUTE_SPEED = 22.0F;
constexpr float ROUTE_FADE = 28.0F;

// Half the size of a route mark, a small diamond flat on the ground
constexpr float ROUTE_MARK_WIDTH = 5.0F;
constexpr float ROUTE_MARK_HEIGHT = 2.5F;

const asw::Color ROUTE_COLOR(190, 255, 190);

constexpr int STAR_COUNT = 140;

// How far the stars move with the camera, for depth
constexpr float STAR_PARALLAX = 0.03F;

// Sky from top to bottom, toxic and clean
const asw::Color TOXIC_SKY_TOP(30, 40, 25);
const asw::Color TOXIC_SKY_BOTTOM(90, 105, 40);
const asw::Color CLEAN_SKY_TOP(35, 80, 160);
const asw::Color CLEAN_SKY_BOTTOM(150, 195, 230);
constexpr int SKY_BANDS = 48;
}  // namespace

void World::init() {
  resource_manager.load("assets/resources.json");
  tile_map.generate();
  updateProgression();
  sound_orchestrator.init();

  // Start on dry land near the middle of the map, never in water
  const auto spawn =
      pathfinding::nearestLand(tile_map,
                               {TileMap::MAP_WIDTH / 2, TileMap::MAP_DEPTH / 2})
          .value_or(asw::Vec2<int>(0, 0));
  const auto ground = tile_map.getSurface(spawn);
  player.setPosition({spawn.x, spawn.y, ground ? ground->index.z + 1 : 0});
  player.setDrillSpeed(1);
  player.setMoveSpeed(1);
  playerWaypoint = {spawn.x, spawn.y, ground ? ground->index.z : 0};
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
  centerCameraOnPlayer();

  effects.init(tile_map);

  stars.clear();
  for (int i = 0; i < STAR_COUNT; ++i) {
    stars.push_back({.position = {asw::random::between(0.0F, 1.0F),
                                  asw::random::between(0.0F, 1.0F)},
                     .size = asw::random::between(1.0F, 2.5F),
                     .phase = asw::random::between(0.0F, 6.28F)});
  }
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

  camera.update(dt);

  tile_map.setReservedColumns(player.getColumns());
  tile_map.update(dt);
  for (const auto& tile : tile_map.takePurified()) {
    effects.tilePurified(tile);
  }

  rescuePlayer();
  player.update(dt, *this);

  updateProgression();

  // Orch
  sound_orchestrator.update(dt, *this);

  lighting.update(dt, *this);
  effects.update(dt, *this);
  waypoint_age += dt;
}

void World::updateProgression() {
  // Count toxins
  const float toxic_count = tile_map.countByType(tile_id::TOXIC_WATER) +
                            tile_map.countByType(tile_id::TOXIC_GRASS) +
                            tile_map.countByType(tile_id::TOXIC_SOIL);

  const float non_toxic_count = tile_map.countByType(tile_id::WATER) +
                                tile_map.countByType(tile_id::GROUND_GRASS) +
                                tile_map.countByType(tile_id::GROUND);

  // Calculate progression
  progression = (non_toxic_count) / (toxic_count + non_toxic_count + 0.1F);
}

void World::drawSky(const asw::Quad<float>& view) {
  // Bands from top to bottom, toxic green turning to blue as it is cleaned
  const float clean = asw::easing::smoothstep(progression);
  const auto top = TOXIC_SKY_TOP.lerp(CLEAN_SKY_TOP, clean);
  const auto bottom = TOXIC_SKY_BOTTOM.lerp(CLEAN_SKY_BOTTOM, clean);
  const float band = view.size.y / SKY_BANDS;
  for (int i = 0; i < SKY_BANDS; ++i) {
    const float t = static_cast<float>(i) / (SKY_BANDS - 1);
    asw::draw::rect_fill(asw::Quad(0.0F, i * band, view.size.x, band + 1.0F),
                         top.lerp(bottom, t));
  }

  // Stars come out at night and twinkle. The light map dims them with
  // everything else, so they start bright
  const float darkness = lighting.getDarkness();
  if (darkness <= 0.0F) {
    return;
  }

  const float time = lighting.getTime();
  const auto drift = view.position * STAR_PARALLAX;
  for (const auto& star : stars) {
    const float x =
        std::fmod((star.position.x * view.size.x) - drift.x + view.size.x * 8,
                  view.size.x);
    const float y =
        std::fmod((star.position.y * view.size.y) - drift.y + view.size.y * 8,
                  view.size.y);
    const float twinkle =
        0.65F + (0.35F * std::sin((time * 2.0F) + star.phase));
    const auto alpha = static_cast<uint8_t>(255.0F * darkness * twinkle);
    asw::draw::circle_fill(asw::Vec2(x, y), star.size,
                           asw::Color(255, 255, 240, alpha));
  }
}

void World::draw() {
  const auto view = camera.get_view();
  drawSky(view);

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
    // Draw player waypoint. It drops in and bounces, its shadow growing
    // as it lands
    // On the ground the clicked tile shows, under items and on water
    const auto ground = tile_map.getSurfaceOf(getPlayerWaypoint());
    auto player_waypoint_screen = camera.world_to_screen(
        asw::Vec2(isoX(ground.index), isoY(ground.index)) * TILE_HEIGHT +
        asw::Vec2(0.0F, ground.drop));

    const float landed = asw::easing::ease_out_bounce(
        std::min(waypoint_age / WAYPOINT_DROP_TIME, 1.0F));
    const float drop = WAYPOINT_DROP_HEIGHT * (1.0F - landed);

    const float shadow_size = 48.0F * (0.5F + (0.5F * landed));
    auto loc = player_waypoint_screen +
               asw::Vec2<float>(6 + ((48.0F - shadow_size) / 2.0F),
                                -16 + ((48.0F - shadow_size) / 2.0F));
    asw::draw::stretch_sprite(
        shadowTexture,
        asw::Quad<float>(loc.x, loc.y, shadow_size, shadow_size));

    asw::draw::sprite(waypointTexture,
                      player_waypoint_screen + asw::Vec2<float>(0, -48 - drop));
  }

  // Dust and bubbles are lit with the scene. Sparkles give off light, so
  // they go over the light map
  effects.drawLit(camera);
  lighting.draw();
  effects.drawGlow(camera);

  // Over the light map, so they read at night
  drawRoute();
  effects.drawText(camera);
}

void World::drawRoute() {
  if (!waypointActive) {
    return;
  }

  // The ground under the worker, then the middle of each column on its route
  const auto& position = player.getPosition();
  const asw::Vec3<float> ground(position.x, position.y, position.z - 1.0F);
  std::vector<asw::Vec2<float>> points = {camera.world_to_screen(
      asw::Vec2(isoXf(ground) * TILE_HEIGHT_F + TILE_HEIGHT_F,
                isoYf(ground) * TILE_HEIGHT_F + (TILE_HEIGHT_F / 2.0F) +
                    player.getSurfaceDrop()))};

  const auto columns = player.getColumns();
  for (std::size_t i = 1; i < columns.size(); ++i) {
    const auto surface = tile_map.getSurface(columns[i]);
    if (surface) {
      points.push_back(camera.world_to_screen(surface->center()));
    }
  }

  float total = 0.0F;
  for (std::size_t i = 1; i < points.size(); ++i) {
    total += points[i - 1].distance(points[i]);
  }
  if (total <= 0.0F) {
    return;
  }

  // Point a distance along the route
  const auto point_at = [&points](float distance) {
    for (std::size_t i = 1; i < points.size(); ++i) {
      const float length = points[i - 1].distance(points[i]);
      if (distance <= length && length > 0.0F) {
        return points[i - 1] + (points[i] - points[i - 1]) * (distance / length);
      }
      distance -= length;
    }
    return points.back();
  };

  // Marks are spaced back from the waypoint, so they stay on the ground as
  // the worker drives over them, and march towards the waypoint
  const float phase = std::fmod(waypoint_age * ROUTE_SPEED, ROUTE_SPACING);
  for (float from_end = ROUTE_SPACING - phase; from_end < total;
       from_end += ROUTE_SPACING) {
    const float from_start = total - from_end;
    const float fade =
        std::min({1.0F, from_start / ROUTE_FADE, from_end / ROUTE_FADE});
    const auto alpha = [fade](float a) {
      return static_cast<uint8_t>(a * asw::easing::smoothstep(fade));
    };

    const auto at = point_at(from_start);
    const auto diamond = [&at](float scale, float lift) {
      const float w = ROUTE_MARK_WIDTH * scale;
      const float h = ROUTE_MARK_HEIGHT * scale;
      const auto c = at + asw::Vec2(0.0F, lift);
      return asw::Polygonf{c + asw::Vec2(-w, 0.0F), c + asw::Vec2(0.0F, -h),
                           c + asw::Vec2(w, 0.0F), c + asw::Vec2(0.0F, h)};
    };

    // Soft shadow, then the mark
    asw::draw::polygon_fill(diamond(1.5F, 1.5F), asw::Color(0, 0, 0, alpha(70)));
    asw::draw::polygon_fill(
        diamond(1.0F, 0.0F),
        asw::Color(ROUTE_COLOR.r, ROUTE_COLOR.g, ROUTE_COLOR.b, alpha(230)));
  }
}

void World::rescuePlayer() {
  const auto& position = player.getPosition();
  const asw::Vec2<int> column{static_cast<int>(std::round(position.x)),
                              static_cast<int>(std::round(position.y))};
  if (pathfinding::isWalkable(tile_map, column)) {
    return;
  }

  const auto land = pathfinding::nearestLand(tile_map, column);
  if (!land) {
    return;
  }

  const auto ground = tile_map.getSurface(*land);
  player.setPosition({land->x, land->y, ground ? ground->index.z + 1 : 0});
  waypointActive = false;
  effects.built({land->x, land->y, ground ? ground->index.z : 0});
}

bool World::setPlayerWaypoint(const asw::Vec3<int>& waypoint) {
  const bool same = waypoint.x == playerWaypoint.x &&
                    waypoint.y == playerWaypoint.y &&
                    waypoint.z == playerWaypoint.z;

  // Holding the button on one tile keeps the route already planned
  if (same && waypointActive) {
    return true;
  }

  const auto& position = player.getPosition();
  const auto start = asw::Vec2<int>(static_cast<int>(std::round(position.x)),
                                    static_cast<int>(std::round(position.y)));
  const auto path =
      pathfinding::findPath(tile_map, start, {waypoint.x, waypoint.y});
  if (!path) {
    return false;
  }

  // A new spot drops the marker in again
  if (!same) {
    waypoint_age = 0.0F;
  }
  playerWaypoint = waypoint;
  player.setPath(*path);
  return true;
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
  updateProgression();

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
