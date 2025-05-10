#pragma once

#include <string>
#include <vector>

#include "tile_type.h"

class TileDictionary {
 public:
  static void load(const std::string& path);
  static std::shared_ptr<TileType> getTile(int id);
  static std::shared_ptr<TileType> getTile(const std::string& id_str);

 private:
  static std::vector<std::shared_ptr<TileType>> types;
};
