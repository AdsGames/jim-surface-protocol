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

  void update(float dt, World& world);

  WorkerId getId() const;

  void draw(const asw::Vec2<float>& offset);
  void update();

 private:
  int direction{0};

  bool up;
  bool down;
  bool left;
  bool right;

  asw::Font font;

  static WorkerId idCounter;

  asw::Vec3<float> position;

  std::array<asw::Texture, 6> textures;

  asw::Texture shadow;

  WorkerId id;

  WorkerStatus status{WorkerStatus::IDLE};
};