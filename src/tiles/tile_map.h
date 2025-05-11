#pragma once

#include <asw/asw.h>
#include <asw/util/Timer.h>
#include <array>
#include <string>
#include <vector>

#include "tile.h"

constexpr int MAP_WIDTH = 100;
constexpr int MAP_DEPTH = 100;
constexpr int MAP_HEIGHT = 10;

constexpr float TICK_TIME = 100.0F;

class TileMap {
 public:
  TileMap() = default;

  asw::Vec3<int> getSize() const;

  void update(float dt);

  void draw(const asw::Quad<float>& camera);

  void generate();
  void generateStructure(const std::string& id_str,
                         const asw::Vec3<int>& position);

  Tile* getTileAt(const asw::Vec2<float>& position);
  Tile* getTileAtIndex(const asw::Vec3<int>& index);

  asw::Vec3<int> getIndexAt(const asw::Vec2<float>& position);

  int countByType(int type) const;

 private:
  void draw_layer(const asw::Quad<float>& camera, int layer);

  void tick_tile(const asw::Vec3<int>& index);

  // 3D Map
  std::array<std::array<std::array<Tile, MAP_HEIGHT>, MAP_DEPTH>, MAP_WIDTH>
      mapTiles;

  // Tile count
  std::array<int, 256> tileCount{0};

  // Tick
  float tick_timer{0.0F};
};
