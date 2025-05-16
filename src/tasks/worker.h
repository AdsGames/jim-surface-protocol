#pragma once

#include <asw/asw.h>
#include <array>
#include <iostream>
#include <string>

#include "../lib/project.h"
// #include "../world/world.h"
class World;

using WorkerId = int;

enum class WorkerStatus { IDLE, WORKING, EN_ROUTE, RETURNING };

class Worker {
 public:
  Worker();

  void setPosition(const asw::Vec3<int>& pos);
  const asw::Vec3<float>& getPosition() const;

  void update(float dt, World& world);

  WorkerId getId() const;

  void draw(const asw::Vec2<float>& offset);
  void update();

  // Stats
  int getDrillSpeed() const { return drillSpeed; }
  void setDrillSpeed(int speed) { drillSpeed = speed; }

  int getMoveSpeed() const { return moveSpeed; }
  void setMoveSpeed(int speed) { moveSpeed = speed; }

 private:
  int direction{0};

  asw::Font font;

  static WorkerId idCounter;

  asw::Vec3<float> position;

  std::array<asw::Texture, 8> textures;

  asw::Texture shadow;

  WorkerId id;

  WorkerStatus status{WorkerStatus::IDLE};

  int drillSpeed{1};

  int moveSpeed{1};
};