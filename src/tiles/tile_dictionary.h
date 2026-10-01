#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "tile_type.h"

class TileDictionary {
 public:
  static void load(const std::string& path);
  static std::shared_ptr<TileType> getTile(int id);
  static std::shared_ptr<TileType> getTile(const std::string& id_str);

 private:
  // Check the ids in tile_ids.h match the loaded tiles
  static void checkNamedIds();

  // Indexed by id, empty where no tile has that id
  static std::vector<std::shared_ptr<TileType>> types_by_id;
  static std::unordered_map<std::string, std::shared_ptr<TileType>>
      types_by_name;
};
