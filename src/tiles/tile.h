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

  // Position
  void setPosition(const asw::Vec3<int>& position);
  const asw::Vec3<int>& getPosition() const;

  // Type ref
  void setType(short type);
  void setType(std::string type);
  std::shared_ptr<TileType> getType() const;
  short getTypeId() const;

  // Structure ref
  void setStructure(std::shared_ptr<Structure> structure);
  std::shared_ptr<Structure> getStructure() const;

  void draw(const asw::Vec2<float>& offset,
            bool left_border,
            bool right_border);

 private:
  short type{0};

  asw::Vec3<int> position{0, 0, 0};

  std::shared_ptr<TileType> t_type{nullptr};

  std::shared_ptr<Structure> structure{nullptr};
};
