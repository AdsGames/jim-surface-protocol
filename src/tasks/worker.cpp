#include "worker.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "../world/world.h"

namespace {
// How fast the worker eases up and down steps, per second
constexpr float HEIGHT_EASE_RATE = 18.0F;

// Height changes bigger than this, in blocks, snap, e.g. after a rescue
constexpr float MAX_EASED_STEP = 2.0F;
}  // namespace

WorkerId Worker::idCounter = 0;

Worker::Worker() : id(idCounter++) {
  textures[0] = asw::assets::load_texture("assets/images/player/128/3.png");
  textures[1] = asw::assets::load_texture("assets/images/player/128/4.png");
  textures[2] = asw::assets::load_texture("assets/images/player/128/2.png");
  textures[3] = asw::assets::load_texture("assets/images/player/128/2.png");
  textures[4] = asw::assets::load_texture("assets/images/player/128/1.png");
  textures[5] = asw::assets::load_texture("assets/images/player/128/5.png");
  textures[6] = asw::assets::load_texture("assets/images/player/128/0.png");
  textures[7] = asw::assets::load_texture("assets/images/player/128/0.png");

  shadow = asw::assets::load_texture("assets/images/player/128/shadow.png");

  font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 16);
}

void Worker::setPosition(const asw::Vec3<int>& pos) {
  asw::Vec3<float> posF(static_cast<float>(pos.x), static_cast<float>(pos.y),
                        static_cast<float>(pos.z));
  position = posF;
  path.clear();
  moving = false;
}

const asw::Vec3<float>& Worker::getPosition() const {
  return position;
}

WorkerId Worker::getId() const {
  return id;
}

void Worker::setPath(const std::vector<asw::Vec2<int>>& columns) {
  path.assign(columns.begin(), columns.end());
}

std::vector<asw::Vec2<int>> Worker::getColumns() const {
  std::vector<asw::Vec2<int>> columns = {
      {static_cast<int>(std::round(position.x)),
       static_cast<int>(std::round(position.y))}};
  columns.insert(columns.end(), path.begin(), path.end());
  return columns;
}

void Worker::update(float dt, World& world) {
  auto& tile_map = world.getTileMap();

  // Drive along the route, through as many columns as this frame allows
  float left = 3.0F * static_cast<float>(moveSpeed) * dt;
  moving = false;
  while (!path.empty() && left > 0.0F) {
    const auto target = asw::Vec2<float>(static_cast<float>(path.front().x),
                                         static_cast<float>(path.front().y));
    const auto here = asw::Vec2<float>(position.x, position.y);
    const float distance = here.distance(target);

    if (distance > 0.001F) {
      auto angle = here.angle(target);
      if (angle < 0) {
        angle += std::numbers::pi_v<float> * 2;
      }
      direction = static_cast<int>(
                      std::round(angle / (std::numbers::pi_v<float> / 4))) %
                  8;
    }

    if (distance <= left) {
      position.x = target.x;
      position.y = target.y;
      left -= distance;
      path.pop_front();
    } else {
      const auto step = (target - here) * (left / distance);
      position.x += step.x;
      position.y += step.y;
      left = 0.0F;
    }
    moving = true;
  }

  if (moving) {
    bob_time += dt;
  } else if (world.getWaypointActive()) {
    world.setWaypointActive(false);
  }

  // Stand on the ground: on top of blocks and water, not on items, which
  // the worker drives through
  const auto surface =
      tile_map.getSurface(asw::Vec2(static_cast<int>(std::round(position.x)),
                                    static_cast<int>(std::round(position.y))));

  // Ease up and down steps, so the worker does not pop between heights
  if (surface) {
    const float target_z = static_cast<float>(surface->index.z) + 1.0F;
    const float ease = std::min(1.0F, dt * HEIGHT_EASE_RATE);
    if (std::abs(target_z - position.z) > MAX_EASED_STEP) {
      position.z = target_z;
      surface_drop = surface->drop;
    } else {
      position.z += (target_z - position.z) * ease;
      surface_drop += (surface->drop - surface_drop) * ease;
    }
  }
}

void Worker::draw(const asw::Vec2<float>& offset) {
  auto iso_x = isoXf(position);
  auto iso_y = isoYf(position);

  auto screen_size = asw::Quad(
      iso_x * TILE_HEIGHT_F - offset.x,
      iso_y * TILE_HEIGHT_F - offset.y + TILE_HEIGHT_F * 0.25F + surface_drop,
      TILE_WIDTH_F, TILE_WIDTH_F);

  asw::draw::stretch_sprite(shadow, screen_size);

  // Bob on the tracks while walking. The shadow stays on the ground
  auto body = screen_size;
  if (moving) {
    body.position.y -= std::abs(std::sin(bob_time * 14.0F)) * 3.0F;
  }
  asw::draw::stretch_sprite(textures[direction], body);
}
