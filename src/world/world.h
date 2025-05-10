#pragma once

#include <unordered_map>

#include "../tasks/worker.h"
#include "../tiles/tile_map.h"

class World {
 public:
  World() = default;

  /// Base
  void init();

  void update(float dt);

  void draw();

  /// Worker Utils
  WorkerId addWorker(const asw::Vec3<int>& position);
  Worker* getWorker(WorkerId id);

  /// Camera Utils
  asw::Quad<float>& getCamera() { return camera; }

  /// Tile Utils
  TileMap& getTileMap() { return tile_map; }


 private:

  Worker player;

  TileMap tile_map;

  asw::Quad<float> camera{0, 0, 1280, 960};

  std::unordered_map<WorkerId, Worker> workers;
};