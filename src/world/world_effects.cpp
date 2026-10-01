#include "world_effects.h"

#include <numbers>

#include "../lib/project.h"
#include "../tiles/tile_ids.h"
#include "world.h"

namespace {
constexpr float PI = std::numbers::pi_v<float>;
constexpr float UP = -PI / 2.0F;

// Seconds between scans for bubbling tiles
constexpr float SCAN_INTERVAL = 1.0F;

// Particles per second over the tiles in view
constexpr float BUBBLE_RATE = 14.0F;
constexpr float FOG_RATE = 4.0F;
constexpr float DEBRIS_RATE = 30.0F;
constexpr float WORKER_DUST_RATE = 14.0F;

// Shake when a tile breaks, in pixels
constexpr float BREAK_SHAKE = 5.0F;

// Seconds a text popup lasts, and how far it rises in pixels
constexpr float POPUP_TIME = 1.2F;
constexpr float POPUP_RISE = 40.0F;

// Soft round texture, tinted, for particles that should look like light
asw::Texture soft_dot(asw::Color color, bool additive) {
  auto texture = asw::assets::create_radial_gradient(
      32, color, asw::Color(color.r, color.g, color.b, 0));
  if (additive) {
    asw::draw::set_blend_mode(texture, asw::BlendMode::Add);
  }
  return texture;
}
}  // namespace

void WorldEffects::init(TileMap& map) {
  tile_map = &map;

  // Chips thrown up by the drill, falling back down
  asw::ParticleConfig debris_config;
  debris_config.lifetime_min = 0.35F;
  debris_config.lifetime_max = 0.7F;
  debris_config.speed_min = 60.0F;
  debris_config.speed_max = 150.0F;
  debris_config.angle_min = UP - 1.1F;
  debris_config.angle_max = UP + 1.1F;
  debris_config.color_start = asw::Color(215, 185, 130);
  debris_config.color_end = asw::Color(110, 90, 65);
  debris_config.size_start = 7.0F;
  debris_config.size_end = 3.0F;
  debris_config.gravity = {0.0F, 420.0F};
  debris = asw::ParticleEmitter(debris_config, 256);

  // Puffs from breaking, building and the worker's tracks
  asw::ParticleConfig dust_config;
  dust_config.lifetime_min = 0.5F;
  dust_config.lifetime_max = 0.9F;
  dust_config.speed_min = 10.0F;
  dust_config.speed_max = 45.0F;
  dust_config.alpha_start = 0.55F;
  dust_config.size_start = 10.0F;
  dust_config.size_end = 26.0F;
  dust_config.gravity = {0.0F, -15.0F};
  dust_config.texture = soft_dot(asw::Color(170, 150, 120), false);
  dust = asw::ParticleEmitter(dust_config, 256);

  // Bubbles that rise off toxic water and pop
  asw::ParticleConfig bubble_config;
  bubble_config.lifetime_min = 0.7F;
  bubble_config.lifetime_max = 1.5F;
  bubble_config.speed_min = 6.0F;
  bubble_config.speed_max = 18.0F;
  bubble_config.angle_min = UP - 0.3F;
  bubble_config.angle_max = UP + 0.3F;
  bubble_config.color_start = asw::Color(150, 230, 70, 220);
  bubble_config.color_end = asw::Color(200, 255, 140, 0);
  bubble_config.size_start = 2.0F;
  bubble_config.size_end = 6.0F;
  bubbles = asw::ParticleEmitter(bubble_config, 256);

  // Green fog drifting over toxic water
  asw::ParticleConfig fog_config;
  fog_config.lifetime_min = 3.0F;
  fog_config.lifetime_max = 5.0F;
  fog_config.speed_min = 4.0F;
  fog_config.speed_max = 12.0F;
  fog_config.alpha_start = 0.3F;
  fog_config.size_start = 40.0F;
  fog_config.size_end = 110.0F;
  fog_config.texture = soft_dot(asw::Color(110, 170, 50), false);
  fog = asw::ParticleEmitter(fog_config, 128);

  // Light rising from purified tiles
  asw::ParticleConfig sparkle_config;
  sparkle_config.lifetime_min = 0.6F;
  sparkle_config.lifetime_max = 1.1F;
  sparkle_config.speed_min = 10.0F;
  sparkle_config.speed_max = 35.0F;
  sparkle_config.angle_min = UP - 0.7F;
  sparkle_config.angle_max = UP + 0.7F;
  sparkle_config.size_start = 12.0F;
  sparkle_config.size_end = 2.0F;
  sparkle_config.gravity = {0.0F, -25.0F};
  sparkle_config.texture = soft_dot(asw::Color(90, 210, 120), true);
  sparkles = asw::ParticleEmitter(sparkle_config, 512);

  toxic_tops.clear();
  scan_timer = SCAN_INTERVAL;

  popup_font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 20);
  popups.clear();
}

void WorldEffects::update(float dt, World& world) {
  scan_timer += dt;
  if (scan_timer >= SCAN_INTERVAL) {
    scan_timer = 0.0F;
    scanTiles(world);
  }

  const auto view = world.getCamera().get_view();
  emitOver(bubbles, toxic_tops, view, bubbles_pending, BUBBLE_RATE * dt, 4.0F);
  emitOver(fog, toxic_tops, view, fog_pending, FOG_RATE * dt, 8.0F);

  // Dust behind the worker's tracks
  const auto& worker = world.getPlayer();
  if (worker.isMoving()) {
    dust_pending += WORKER_DUST_RATE * dt;
    const auto& position = worker.getPosition();
    const auto feet = asw::Vec2(isoXf(position) * TILE_HEIGHT_F + TILE_HEIGHT_F,
                                isoYf(position) * TILE_HEIGHT_F + TILE_WIDTH_F +
                                    worker.getSurfaceDrop());
    while (dust_pending >= 1.0F) {
      dust_pending -= 1.0F;
      emitAt(dust, feet + asw::Vec2(asw::random::between(-10.0F, 10.0F), 0.0F),
             1);
    }
  }

  for (auto* emitter : {&debris, &dust, &bubbles, &fog, &sparkles}) {
    emitter->update(dt);
  }

  for (auto& popup : popups) {
    popup.age += dt;
  }
  std::erase_if(popups, [](const Popup& p) { return p.age >= POPUP_TIME; });
}

void WorldEffects::drawLit(const asw::Camera& camera) {
  fog.draw(camera);
  bubbles.draw(camera);
  dust.draw(camera);
  debris.draw(camera);
}

void WorldEffects::drawGlow(const asw::Camera& camera) {
  sparkles.draw(camera);
}

void WorldEffects::drawText(asw::Camera& camera) {
  for (const auto& popup : popups) {
    // Rise fast then slow, and fade out at the end
    const float t = popup.age / POPUP_TIME;
    const float rise = POPUP_RISE * asw::easing::ease_out_cubic(t);
    auto color = popup.color;
    color.a = static_cast<uint8_t>(255.0F * (1.0F - (t * t)));

    const auto position = camera.world_to_screen(popup.position) -
                          asw::Vec2(0.0F, 20.0F + rise);
    asw::draw::text_shadow(popup_font, popup.text, position, color,
                           asw::Color(0, 0, 0), asw::Vec2(2.0F, 2.0F),
                           asw::TextJustify::Center);
  }
}

void WorldEffects::popup(const asw::Vec3<int>& tile,
                         const std::string& text,
                         asw::Color color) {
  popups.push_back({.position = groundOf(tile), .text = text, .color = color});
}

void WorldEffects::drilling(const asw::Vec3<int>& tile, float dt) {
  debris_pending += DEBRIS_RATE * dt;
  const auto count = static_cast<uint32_t>(debris_pending);
  debris_pending -= static_cast<float>(count);
  emitAt(debris, groundOf(tile), count);
}

void WorldEffects::tileBroken(const asw::Vec3<int>& tile, asw::Camera& camera) {
  const auto top = groundOf(tile);
  emitAt(debris, top, 32);
  emitAt(dust, top, 10);
  camera.shake(BREAK_SHAKE);
}

void WorldEffects::tilePurified(const asw::Vec3<int>& tile) {
  emitAt(sparkles, groundOf(tile), 5);
}

void WorldEffects::built(const asw::Vec3<int>& tile) {
  // Saplings stand on the tile below, purifiers puff from their own top
  const auto ground = groundOf(tile);
  emitAt(dust, ground, 12);
  emitAt(sparkles, ground, 6);
}

void WorldEffects::scanTiles(World& world) {
  auto& map = world.getTileMap();
  toxic_tops.clear();

  for (int i = 0; i < TileMap::MAP_WIDTH; ++i) {
    for (int j = 0; j < TileMap::MAP_DEPTH; ++j) {
      const auto surface = map.getSurface({i, j});
      if (surface && map.getTileAtIndex(surface->index)->getTypeId() ==
                         tile_id::TOXIC_WATER) {
        toxic_tops.push_back(surface->center());
      }
    }
  }
}

asw::Vec2<float> WorldEffects::groundOf(const asw::Vec3<int>& tile) const {
  return tile_map->getSurfaceOf(tile).center();
}

void WorldEffects::emitOver(asw::ParticleEmitter& emitter,
                            const std::vector<asw::Vec2<float>>& tops,
                            const asw::Quad<float>& view,
                            float& pending,
                            float count,
                            float lift) {
  if (tops.empty()) {
    return;
  }

  pending += count;

  // Try a few random tiles for each particle, and keep the ones in view.
  // Rates are per screen, so they do not grow with the map
  constexpr int TRIES = 8;
  while (pending >= 1.0F) {
    pending -= 1.0F;
    for (int attempt = 0; attempt < TRIES; ++attempt) {
      const auto& top =
          tops[asw::random::between(0, static_cast<int>(tops.size()) - 1)];
      if (view.contains(top)) {
        emitAt(emitter,
               top + asw::Vec2(asw::random::between(-14.0F, 14.0F),
                               asw::random::between(-6.0F, 6.0F) - lift),
               1);
        break;
      }
    }
  }
}

void WorldEffects::emitAt(asw::ParticleEmitter& emitter,
                          const asw::Vec2<float>& position,
                          uint32_t count) {
  emitter.transform.position = position;
  emitter.emit(count);
}
