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

  font = asw::assets::loadFont("assets/fonts/ariblk.ttf", 16);
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

  asw::Vec3<int> positionInt(static_cast<int>(position.x),
                             static_cast<int>(position.y),
                             static_cast<int>(position.z));
  auto iso_x = isoX(positionInt);
  auto iso_y = isoY(positionInt);

  auto screen_pos = asw::Vec2(iso_x, iso_y) * 32;

  auto waypoint_x = isoX(waypoint);
  auto waypoint_y = isoY(waypoint);

  auto waypoint_screen_pos = asw::Vec2(waypoint_x, waypoint_y) * 32;

  angle = screen_pos.angle(waypoint_screen_pos);

  float pi = 3.14159265358979323846F;

  if (angle > 0 && angle < pi / 4)
    direction = 3;
  else if (angle > pi / 4 && angle < (3 * pi) / 4)
    direction = 4;
  else if (angle > (3 * pi) / 4)
    direction = 2;
  else if (angle < (-3 * pi) / 4)
    direction = 1;
  else if (angle > (-3 * pi) / 4 && angle < (-pi / 4))
    direction = 5;
  else if (angle < 0 && angle > -pi / 4)
    direction = 0;

  float speed = 0.01F;
  float move = speed * dt;
  if (position.distance(waypointF) > move) {
    left = false;
    right = false;
    up = false;
    down = false;

    auto dir = asw::Vec3<float>(0, 0, 0);
    if (position.x < waypoint.x) {
      dir.x = move;
    } else if (position.x > waypointF.x) {
      dir.x = -move;
    }

    if (position.y < waypointF.y) {
      dir.y = move;
    } else if (position.y > waypointF.y) {
      dir.y = -move;
    }

    if (position.z < waypointF.z) {
      dir.z = move;
    } else if (position.z > waypointF.z) {
      dir.z = -move;
    }

    position += dir;
  } else {
    position = waypointF;
    world.setWaypointActive(false);
  }
}

void Worker::draw(const asw::Vec2<float>& offset) {
  // Calc screen position

  auto iso_x = isoXf(position);
  auto iso_y = isoYf(position);

  auto screen_size =
      asw::Quad(iso_x * TILE_HEIGHT_F - offset.x,
                iso_y * TILE_HEIGHT_F - offset.y - TILE_HEIGHT_F * 0.75F,
                TILE_WIDTH_F, TILE_WIDTH_F);

  asw::draw::stretchSprite(shadow, screen_size);
  asw::draw::stretchSprite(textures[direction], screen_size);

  std::string l = left ? "True" : "False";
  std::string r = right ? "True" : "False";
  std::string u = up ? "True" : "False";
  std::string d = down ? "True" : "False";

  auto t_pos = asw::Vec2<float>(0, 104);

  std::string angleStr = std::to_string(angle);
  std::string directionStr = std::to_string(direction);

  asw::draw::text(font, "Direction:  " + directionStr,
                  t_pos + asw::Vec2<float>(0, 60),
                  asw::util::makeColor(255, 255, 255));

  asw::draw::text(font, "Angle:  " + angleStr, t_pos + asw::Vec2<float>(0, 80),
                  asw::util::makeColor(255, 255, 255));
}
