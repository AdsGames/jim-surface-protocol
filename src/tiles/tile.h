#pragma once

#include <asw/asw.h>
#include <string>
#include <vector>

#include "structure_dictionary.h"
#include "tile_type.h"

class Tile {
 public:
  Tile() = default;

  explicit Tile(short type);

  void setPosition(const asw::Vec3<int>& position);

  bool containsAttribute(int newAttribute);

  // Type ref
  void setType(short type);
  void setType(std::string type);
  std::shared_ptr<TileType> getType() const;

  // Structure ref
  void setStructure(std::shared_ptr<Structure> structure);
  std::shared_ptr<Structure> getStructure() const;

  void draw(const asw::Vec2<float>& offset,
            bool hidden,
            bool left_border,
            bool right_border);

 private:
  asw::Vec3<int> position{0, 0, 0};

  std::shared_ptr<TileType> t_type{nullptr};

  std::shared_ptr<Structure> structure{nullptr};
};
