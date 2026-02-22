#include "worker.h"

#include <numbers>

#include "../world/world.h"

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
}

const asw::Vec3<float>& Worker::getPosition() const {
  return position;
}

WorkerId Worker::getId() const {
  return id;
}

void Worker::update(float dt, World& world) {
  auto waypoint = world.getPlayerWaypoint();
  auto& player = world.getPlayer();
  auto& tile_map = world.getTileMap();

  // Move to waypoint
  const auto waypoint_xy = asw::Vec2<float>(waypoint.x, waypoint.y);
  const auto position_xy = asw::Vec2<float>(position.x, position.y);
  const float speed = 3.0F * player.getMoveSpeed();
  const float move = speed * dt;

  if (position_xy.distance(waypoint_xy) > 0.1F) {
    auto angle = asw::Vec2(position.x, position.y).angle(waypoint_xy);
    if (angle < 0) {
      angle += std::numbers::pi_v<float> * 2;
    }

    direction =
        static_cast<int>(std::round(angle / (std::numbers::pi_v<float> / 4))) %
        8;

    auto dir = asw::Vec3<float>(0, 0, 0);

    if (std::abs(position.x - waypoint_xy.x) < move) {
      position.x = waypoint_xy.x;
    } else if (position.x < waypoint.x) {
      dir.x = move;
    } else if (position.x > waypoint_xy.x) {
      dir.x = -move;
    }

    if (std::abs(position.y - waypoint_xy.y) < move) {
      position.y = waypoint_xy.y;
    } else if (position.y < waypoint_xy.y) {
      dir.y = move;
    } else if (position.y > waypoint_xy.y) {
      dir.y = -move;
    }

    position += dir;
  } else {
    position.x = waypoint_xy.x;
    position.y = waypoint_xy.y;
    world.setWaypointActive(false);
  }

  // Snap z to top tile
  const auto* top_tile = tile_map.getTopTileAt(
      asw::Vec2(static_cast<int>(std::round(position.x)),
                static_cast<int>(std::round(position.y))));

  if (top_tile != nullptr) {
    position.z = top_tile->getPosition().z + 1.0F;
  }
}

void Worker::draw(const asw::Vec2<float>& offset) {
  auto iso_x = isoXf(position);
  auto iso_y = isoYf(position);

  auto screen_size =
      asw::Quad(iso_x * TILE_HEIGHT_F - offset.x,
                iso_y * TILE_HEIGHT_F - offset.y + TILE_HEIGHT_F * 0.25F,
                TILE_WIDTH_F, TILE_WIDTH_F);

  asw::draw::stretch_sprite(shadow, screen_size);
  asw::draw::stretch_sprite(textures[direction], screen_size);
}
