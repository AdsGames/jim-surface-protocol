#include "world_lighting.h"

#include <algorithm>

#include "../lib/project.h"
#include "../tiles/tile_ids.h"
#include "world.h"

namespace {
// Columns this tall or taller block glow, e.g. hills and structures
constexpr int SOLID_HEIGHT = 7;

// Glow lost per tile, from 0 to 1
constexpr float GLOW_FALLOFF = 0.15F;

// Seconds between glow updates. Tiles change on ticks, not every frame
constexpr float GLOW_INTERVAL = 0.5F;

// Glow texture size relative to the world. Smooth scaling blends the tiles
constexpr float GLOW_SCALE = 0.25F;

const asw::Color TOXIC_GLOW(50, 110, 10);
const asw::Color CLEAN_GLOW(40, 120, 150);

const asw::Color WORKER_LIGHT(255, 220, 160);
const asw::Color WAYPOINT_LIGHT(120, 255, 160);
}  // namespace

WorldLighting::WorldLighting() : toxic_cycle(1.0F), clean_cycle(1.0F) {}

void WorldLighting::init(const asw::Camera& camera, float day_length) {
  light_map.set_camera(&camera);

  // Keys are fractions of a day. Noon, dusk, midnight, dawn
  toxic_cycle = asw::lighting::AmbientCycle(day_length);
  toxic_cycle.add(0.0F, asw::Color(205, 210, 180));
  toxic_cycle.add(day_length * 0.25F, asw::Color(150, 130, 110));
  toxic_cycle.add(day_length * 0.5F, asw::Color(55, 65, 60));
  toxic_cycle.add(day_length * 0.75F, asw::Color(140, 130, 115));

  clean_cycle = asw::lighting::AmbientCycle(day_length);
  clean_cycle.add(0.0F, asw::Color(255, 252, 240));
  clean_cycle.add(day_length * 0.25F, asw::Color(255, 200, 170));
  clean_cycle.add(day_length * 0.5F, asw::Color(90, 100, 150));
  clean_cycle.add(day_length * 0.75F, asw::Color(255, 215, 190));

  time = 0.0F;
  glow_timer = GLOW_INTERVAL;
}

void WorldLighting::update(float dt, World& world) {
  time += dt;
  light_map.update(dt);
  light_map.clear();

  // Ambient moves from the toxic day to the clean day as the world is purified
  const float clean = asw::easing::smoothstep(world.getProgression());
  const auto ambient = toxic_cycle.at(time).lerp(clean_cycle.at(time), clean);
  light_map.set_ambient(ambient);

  // Brightness of the ambient light, as the eye sees it
  const float brightness =
      ((0.299F * ambient.r) + (0.587F * ambient.g) + (0.114F * ambient.b)) /
      255.0F;
  darkness = std::clamp((0.7F - brightness) / 0.45F, 0.0F, 1.0F);

  glow_timer += dt;
  if (glow_timer >= GLOW_INTERVAL) {
    glow_timer = 0.0F;
    computeGlow(world);
    renderGlow();
  }

  if (glow != nullptr) {
    light_map.add_glow(glow, glow_bounds);
  }

  // Lamp on the worker
  const auto& position = world.getPlayer().getPosition();
  asw::lighting::Light lamp;
  lamp.position = asw::Vec2(isoXf(position) * TILE_HEIGHT_F + TILE_HEIGHT_F,
                            isoYf(position) * TILE_HEIGHT_F + TILE_HEIGHT_F +
                                world.getPlayer().getSurfaceDrop());
  lamp.radius = 180.0F;
  lamp.color = WORKER_LIGHT;
  lamp.intensity = 0.8F;
  lamp.flicker = 0.1F;
  lamp.shadows = false;
  lamp.seed = 1;
  light_map.add(lamp);

  // Beacon on the waypoint
  if (world.getWaypointActive()) {
    asw::lighting::Light beacon;
    beacon.position =
        world.getTileMap().getSurfaceOf(world.getPlayerWaypoint()).center();
    beacon.radius = 90.0F;
    beacon.color = WAYPOINT_LIGHT;
    beacon.pulse = 0.4F;
    beacon.pulse_speed = 1.5F;
    beacon.shadows = false;
    beacon.seed = 2;
    light_map.add(beacon);
  }
}

void WorldLighting::draw() {
  light_map.draw();
}

void WorldLighting::computeGlow(World& world) {
  auto& tile_map = world.getTileMap();
  const int width = TileMap::MAP_WIDTH;
  const int depth = TileMap::MAP_DEPTH;

  if (tile_light == nullptr || tile_light->get_size().x != width ||
      tile_light->get_size().y != depth) {
    tile_light = std::make_unique<asw::lighting::TileLight>(width, depth, 1.0F);
    tile_light->set_falloff(GLOW_FALLOFF);
  }

  surfaces.assign(static_cast<std::size_t>(width * depth), std::nullopt);
  tile_light->clear_lights();

  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < depth; ++j) {
      // Items do not block glow, so the height is the ground's
      const auto surface = tile_map.getSurface({i, j});
      surfaces[(i * depth) + j] = surface;
      const int z = surface ? surface->index.z : 0;
      tile_light->set_solid(i, j, z >= SOLID_HEIGHT);

      if (surface && tile_map.getTileAtIndex(surface->index)->getTypeId() ==
                         tile_id::TOXIC_WATER) {
        tile_light->add_light(i, j, TOXIC_GLOW);
      }

      // Purifiers can be under leaves, so check the whole column
      for (int k = z; k >= 0; --k) {
        const auto id = tile_map.getTileAtIndex({i, j, k})->getTypeId();
        if (id == tile_id::PURIFIER || id == tile_id::TRUNK_PURIFIER) {
          tile_light->add_light(i, j, CLEAN_GLOW);
          break;
        }
      }
    }
  }

  tile_light->compute();
}

void WorldLighting::renderGlow() {
  const int width = TileMap::MAP_WIDTH;
  const int depth = TileMap::MAP_DEPTH;

  // Area of the world that tile tops can cover
  const asw::Quad<float> bounds(
      -depth * TILE_HEIGHT_F, -MAP_HEIGHT * TILE_HEIGHT_F,
      (width + depth + 2) * TILE_HEIGHT_F,
      ((width + depth) / 2.0F + MAP_HEIGHT + 2) * TILE_HEIGHT_F);

  if (glow == nullptr || bounds.size != glow_bounds.size) {
    glow = asw::assets::create_texture(
        static_cast<int>(bounds.size.x * GLOW_SCALE),
        static_cast<int>(bounds.size.y * GLOW_SCALE));
    asw::draw::set_scale_mode(glow, asw::ScaleMode::Linear);
  }
  glow_bounds = bounds;

  asw::display::set_render_target(glow);
  asw::display::clear(asw::Color(0, 0, 0));

  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < depth; ++j) {
      const auto light = tile_light->get(i, j);
      if (light.r == 0 && light.g == 0 && light.b == 0) {
        continue;
      }

      const auto& surface = surfaces[(i * depth) + j];
      if (!surface) {
        continue;
      }

      auto face = surface->face();
      for (auto& point : face) {
        point = (point - glow_bounds.position) * GLOW_SCALE;
      }

      asw::draw::polygon_fill(face, asw::Color(light.r, light.g, light.b));
    }
  }

  asw::display::reset_render_target();
}
