#include "tile.h"

#include "tile_dictionary.h"

Tile::Tile(short type) {
  setType(type);
}

void Tile::setPosition(const asw::Vec3<float>& position) {
  this->position = position;
}

std::shared_ptr<TileType> Tile::getType() const {
  return t_type;
}

// Contains Attribute
auto Tile::containsAttribute(int newAttribute) -> bool {
  if (t_type != nullptr) {
    return t_type->hasAttribute(newAttribute);
  }

  return false;
}

void Tile::setType(std::string type) {
  t_type = TileDictionary::getTile(type);
}

// Set type
void Tile::setType(short type) {
  t_type = TileDictionary::getTile(type);
}

// Draw tile
void Tile::draw(const asw::Vec2<float>& offset) {
  if (t_type == nullptr) {
    return;
  }

  t_type->draw(position, offset);
}

void Tile::drawWireframe(const asw::Vec2<float>& offset) {
  if (t_type == nullptr) {
    return;
  }

  t_type->drawWireframe(position, offset);
}
