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

    // Parse name
    std::string id_str = name;
    std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
    std::replace(id_str.begin(), id_str.end(), ' ', '_');

    std::cout << "~ ID: " << id << '\n';
    std::cout << "  Name: " << name << '\n';
    std::cout << "  ID String: " << id_str << '\n';

    // Create tile
    auto tile = std::make_shared<TileType>(id, name, id_str);

    // Images
    int image_count = 0;
    for (auto const& image : cTile["images"]) {
      auto tex = asw::assets::loadTexture(image);
      tile->addImage(tex);
      image_count++;
    }

    std::cout << "  Images: " << image_count << '\n';

    // Render mode
    TileRenderMode render_mode = TileRenderMode::CUBE;
    if (cTile.contains("render_mode")) {
      auto mode = cTile["render_mode"];
      if (mode == "flat") {
        render_mode = TileRenderMode::FLAT;
      } else if (mode == "cube") {
        render_mode = TileRenderMode::CUBE;
      } else if (mode == "cube_unique_top") {
        render_mode = TileRenderMode::CUBE_UNIQUE_TOP;
      } else {
        std::cerr << "Error: Unknown render mode " << mode << '\n';
      }
    }

    std::cout << "  Render Mode: " << static_cast<int>(render_mode) << '\n';

    // Alpha
    float alpha = 1.0F;
    if (cTile.contains("alpha")) {
      alpha = cTile["alpha"];
    }

    // Bake texture
    tile->bakeTexture(render_mode, alpha);

    // Add to types
    types.push_back(tile);
  }

  std::cout << "Loaded " << types.size() << " tiles" << '\n';

  // Close
  file.close();
}
