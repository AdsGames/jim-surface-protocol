#include "worker.h"
#include "../world/world.h"

WorkerId Worker::idCounter = 0;

Worker::Worker() : id(idCounter++) {
  textures[0] = asw::assets::loadTexture("assets/images/player/128/0.png");
  textures[1] = asw::assets::loadTexture("assets/images/player/128/1.png");
  textures[2] = asw::assets::loadTexture("assets/images/player/128/2.png");
  textures[3] = asw::assets::loadTexture("assets/images/player/128/3.png");

  shadow = asw::assets::loadTexture("assets/images/player/128/shadow.png");
}

void Worker::setPosition(const asw::Vec3<int>& pos) {
  position = pos;
}

WorkerId Worker::getId() const {
  return id;
}

void Worker::update(float dt, World& world) {
  // Update position based on direction
  if (asw::input::wasKeyPressed(asw::input::Key::LEFT)) {
    direction = 0;
  }
  if (asw::input::wasKeyPressed(asw::input::Key::DOWN)) {
    direction = 1;
  }

  if (asw::input::wasKeyPressed(asw::input::Key::RIGHT)) {
    direction = 2;
  }

  if (asw::input::wasKeyPressed(asw::input::Key::UP)) {
    direction = 3;
  }

  auto waypoint = world.getPlayerWaypoint();

  if (position.distance(waypoint) > 1) {
    auto dir = asw::Vec3<int>(0, 0, 0);
    if (position.x < waypoint.x) {
      dir.x = 1;
    } else if (position.x > waypoint.x) {
      dir.x = -1;
    }

    if (position.y < waypoint.y) {
      dir.y = 1;
    } else if (position.y > waypoint.y) {
      dir.y = -1;
    }

    position += dir;
  }
}

void Worker::draw(const asw::Vec2<float>& offset) {
  // Calc screen position
  auto iso_x = isoX(position);
  auto iso_y = isoY(position);

  auto screen_pos = asw::Vec2(iso_x, iso_y) * 32 - offset;

  asw::draw::sprite(shadow, screen_pos);

  asw::draw::sprite(textures[direction], screen_pos);
}
