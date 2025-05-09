#pragma once

#include <asw/asw.h>
#include <asw/util/Timer.h>
#include <array>
#include <string>
#include <vector>

#include "tile.h"

constexpr int MAP_WIDTH = 128;
constexpr int MAP_DEPTH = 128;
constexpr int MAP_HEIGHT = 10;

class TileMap {
 public:
  TileMap() = default;

  asw::Vec3<int> getSize() const;

  void update(float deltaTime);

  void draw(const asw::Quad<float>& camera);

  void generate();

  Tile* getTileAt(const asw::Vec2<float>& position);
  Tile* getTileAtIndex(const asw::Vec3<int>& index);

  asw::Vec3<int> getIndexAt(const asw::Vec2<float>& position);

 private:
  void draw_layer(const asw::Quad<float>& camera, int layer);

  // 3D Map
  std::array<std::array<std::array<Tile, MAP_HEIGHT>, MAP_DEPTH>, MAP_WIDTH>
      mapTiles;

  int z_focus{0};
};
