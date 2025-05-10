#include "tile.h"

#include "tile_dictionary.h"

Tile::Tile(short type) {
  setType(type);
}

void Tile::setPosition(const asw::Vec3<int>& position) {
  this->position = position;
}

// Types
void Tile::setType(std::string type) {
  t_type = TileDictionary::getTile(type);
}

std::shared_ptr<TileType> Tile::getType() const {
  return t_type;
}

// Structures
void Tile::setStructure(std::shared_ptr<Structure> structure) {
  this->structure = structure;
}

std::shared_ptr<Structure> Tile::getStructure() const {
  return structure;
}

// Set type
void Tile::setType(short type) {
  t_type = TileDictionary::getTile(type);
}

// Draw tile
void Tile::draw(const asw::Vec2<float>& offset,
                bool hidden,
                bool left_border,
                bool right_border) {
  if (t_type == nullptr) {
    return;
  }

  t_type->draw(position, offset, hidden, left_border, right_border);
}
