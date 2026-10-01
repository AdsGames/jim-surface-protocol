#pragma once

#include <asw/asw.h>
#include <array>
#include <deque>
#include <string>
#include <vector>

#include "../lib/project.h"
// #include "../world/world.h"
class World;

using WorkerId = int;

enum class WorkerStatus { IDLE, WORKING, EN_ROUTE, RETURNING };

class Worker {
 public:
  Worker();

  // Move there at once, dropping any route
  void setPosition(const asw::Vec3<int>& pos);

  // Drive through these columns in order
  void setPath(const std::vector<asw::Vec2<int>>& columns);

  // The column the worker is on, then the columns still on its route
  std::vector<asw::Vec2<int>> getColumns() const;
  const asw::Vec3<float>& getPosition() const;

  void update(float dt, World& world);

  WorkerId getId() const;

  void draw(const asw::Vec2<float>& offset);
  void update();

  // Stats
  int getDrillSpeed() const { return drillSpeed; }
  void setDrillSpeed(int speed) { drillSpeed = speed; }

  int getMoveSpeed() const { return moveSpeed; }

  // True while walking to a waypoint
  bool isMoving() const { return moving; }

  // How far below a full block the ground under the worker is, in pixels,
  // e.g. on water
  float getSurfaceDrop() const { return surface_drop; }
  void setMoveSpeed(int speed) { moveSpeed = speed; }

 private:
  int direction{0};

  bool moving{false};

  // Columns still to drive through
  std::deque<asw::Vec2<int>> path;
  float surface_drop{0.0F};

  // Seconds spent walking, for the bob in its step
  float bob_time{0.0F};

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