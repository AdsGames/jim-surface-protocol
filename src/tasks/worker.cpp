#include "worker.h"
#include "../world/world.h"

WorkerId Worker::idCounter = 0;

Worker::Worker() : id(idCounter++) {
  textures[0] = asw::assets::loadTexture("assets/images/player/128/0.png");
  textures[1] = asw::assets::loadTexture("assets/images/player/128/1.png");
  textures[2] = asw::assets::loadTexture("assets/images/player/128/2.png");
  textures[3] = asw::assets::loadTexture("assets/images/player/128/3.png");
  textures[4] = asw::assets::loadTexture("assets/images/player/128/4.png");
  textures[5] = asw::assets::loadTexture("assets/images/player/128/5.png");

  shadow = asw::assets::loadTexture("assets/images/player/128/shadow.png");
}

void Worker::setPosition(const asw::Vec3<int>& pos) {
  asw::Vec3<float> posF(static_cast<float>(pos.x), static_cast<float>(pos.y),
                        static_cast<float>(pos.z));
  position = posF;
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

  asw::Vec3<float> waypointF(waypoint.x, waypoint.y, waypoint.z);

  if (position.distance(waypointF) > 1) {
    float speed = 0.01F;
    float move = speed * dt;

    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    auto dir = asw::Vec3<float>(0, 0, 0);
    if (position.x < waypoint.x) {
      dir.x = move;
      right = true;

    } else if (position.x > waypointF.x) {
      dir.x = -move;
      left = true;
    }

    if (position.y < waypointF.y) {
      dir.y = move;
      up = true;
    } else if (position.y > waypointF.y) {
      dir.y = -move;
      down = true;
    }

    if (up && left)
      direction = 3;
    if (up && !left)
      direction = 0;
    if (!up && left)
      direction = 2;
    if (!up && !left)
      direction = 1;

    position += dir;
  }
}

void Worker::draw(const asw::Vec2<float>& offset) {
  // Calc screen position

  asw::Vec3<int> positionInt(static_cast<int>(position.x),
                             static_cast<int>(position.y),
                             static_cast<int>(position.z));

  auto iso_x = isoX(positionInt);
  auto iso_y = isoY(positionInt);

  auto screen_pos = asw::Vec2(iso_x, iso_y) * 32 - offset;

  asw::draw::sprite(shadow, screen_pos);

  asw::draw::sprite(textures[direction], screen_pos);
}
