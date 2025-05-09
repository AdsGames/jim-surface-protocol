#include "tile_dictionary.h"

#include <asw/asw.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

std::vector<std::shared_ptr<TileType>> TileDictionary::types;

std::shared_ptr<TileType> TileDictionary::getTile(int id) {
  auto found = std::find_if(types.begin(), types.end(),
                            [&id](auto& t) { return t->getId() == id; });

  if (found != types.end()) {
    return *found;
  }

  std::cout << "Warning: Tile " << id << " not found" << '\n';

  return nullptr;
}

std::shared_ptr<TileType> TileDictionary::getTile(const std::string& id_str) {
  auto found = std::find_if(types.begin(), types.end(), [&id_str](auto& t) {
    return t->getIdString() == id_str;
  });

  if (found != types.end()) {
    return *found;
  }

  std::cout << "Warning: Tile " << id_str << " not found" << '\n';

  return nullptr;
}

void TileDictionary::loadTypes(const std::string& path) {
  // Open file or abort if it does not exist
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << path << '\n';
    return;
  }

  // Get first node
  std::cout << "Loading tiles..." << '\n';

  for (auto const& cTile : nlohmann::json::parse(file)) {
    // Numeric identifier
    const short id = cTile["id"];
    std::string name = cTile["name"];

    std::cout << "- " << name << '\n';

    // Parse name
    std::string id_str = name;
    std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
    std::replace(id_str.begin(), id_str.end(), ' ', '_');

    // Create tile
    auto tile = std::make_shared<TileType>(id, name, id_str);

    // Images
    for (auto const& image : cTile["images"]) {
      auto tex = asw::assets::loadTexture(image);
      tile->addImage(tex);
    }

    // Render mode
    if (cTile.contains("render_mode")) {
      auto render_mode = cTile["render_mode"];
      if (render_mode == "flat") {
        tile->setRenderMode(TileRenderMode::FLAT);
      } else if (render_mode == "cube") {
        tile->setRenderMode(TileRenderMode::CUBE);
      } else if (render_mode == "cube_unique_top") {
        tile->setRenderMode(TileRenderMode::CUBE_UNIQUE_TOP);
      } else {
        std::cerr << "Error: Unknown render mode " << render_mode << '\n';
      }
    }

    // Bake texture
    tile->bakeTexture();

    // Add to types
    types.push_back(tile);
  }

  // Close
  file.close();
}
