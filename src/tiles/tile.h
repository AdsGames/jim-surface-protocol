#pragma once

#include <asw/asw.h>
#include <string>
#include <vector>

#include "tile_type.h"

class Tile {
 public:
  Tile() = default;

  explicit Tile(short type);

  void setPosition(const asw::Vec3<int>& position);

  std::shared_ptr<TileType> getType() const;

  bool containsAttribute(int newAttribute);

  void setType(short type);
  void setType(std::string type);

  void draw(const asw::Vec2<float>& offset);
  void drawHidden(const asw::Vec2<float>& offset);

 private:
  asw::Vec3<int> position{0, 0, 0};

  std::shared_ptr<TileType> t_type{};
};
