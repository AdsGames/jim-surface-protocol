#include "tile_dictionary.h"

#include <asw/asw.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

std::vector<std::shared_ptr<TileType>> TileDictionary::types;

std::shared_ptr<TileType> TileDictionary::getTile(int id) {
  if (id == 0) {
    return nullptr;
  }

  auto found = std::find_if(types.begin(), types.end(),
                            [&id](auto& t) { return t->getId() == id; });

  if (found != types.end()) {
    return *found;
  }

  asw::log::warn("Tile {} not found", id);

  return nullptr;
}

std::shared_ptr<TileType> TileDictionary::getTile(const std::string& id_str) {
  auto found = std::find_if(types.begin(), types.end(), [&id_str](auto& t) {
    return t->getIdString() == id_str;
  });

  if (found != types.end()) {
    return *found;
  }

  asw::log::warn("Tile {} not found", id_str);

  return nullptr;
}

void TileDictionary::load(const std::string& path) {
  // Open file or abort if it does not exist
  std::ifstream file(path);
  if (!file.is_open()) {
    asw::log::error("Could not open file {}", path);
    return;
  }

  // Get first node
  asw::log::info("Loading tiles...");

  for (auto const& cTile : nlohmann::json::parse(file)) {
    // Numeric identifier
    const short id = cTile["id"];
    std::string name = cTile["name"];

    // Parse name
    std::string id_str = name;
    std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
    std::replace(id_str.begin(), id_str.end(), ' ', '_');

    // Create tile
    auto tile = std::make_shared<TileType>(id, name, id_str);

    // Images
    int image_count = 0;
    for (auto const& image : cTile["images"]) {
      auto tex = asw::assets::load_texture(image);
      tile->addImage(tex);
      image_count++;
    }

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
      } else if (mode == "cube_top_only") {
        render_mode = TileRenderMode::CUBE_TOP_ONLY;
      } else {
        asw::log::error("Unknown render mode {}", mode.dump());
      }
    }

    // Alpha
    float alpha = 1.0F;
    if (cTile.contains("alpha")) {
      alpha = cTile["alpha"];
    }

    // Actions
    if (cTile.contains("actions")) {
      for (auto const& action : cTile["actions"]) {
        ActionResult result;

        if (!action.contains("type")) {
          asw::log::error("Action type not found");
          continue;
        }

        if (action["type"] == "destroy") {
          result.type = ActionType::DESTROY;
        } else if (action["type"] == "purify") {
          result.type = ActionType::PURIFY;
        } else if (action["type"] == "tick") {
          result.type = ActionType::TICK;
          if (action.contains("action")) {
            const auto& tick_type = action["action"];
            if (tick_type == "purify") {
              result.tick_type = TickType::PURIFY;
            } else if (tick_type == "growth") {
              result.tick_type = TickType::GROWTH;
            } else if (tick_type == "structure") {
              result.tick_type = TickType::STRUCTURE;
            } else {
              asw::log::error("Unknown tick type {}", tick_type.dump());
            }
          }
        } else {
          asw::log::error("Unknown action type {}",
                          action["type"].dump());
          continue;
        }

        if (action.contains("transition_tile_id")) {
          result.transition_tile_id = action["transition_tile_id"];
        }

        if (action.contains("spawn_structure_id")) {
          result.spawn_structure_id = action["spawn_structure_id"];
        }

        if (action.contains("drop_resource_id")) {
          result.drop_resource_id = action["drop_resource_id"];
        }

        if (action.contains("chance")) {
          result.chance = action["chance"];
        }

        tile->addAction(result);
      }
    }

    if (cTile.contains("density")) {
      auto density = cTile["density"];
      tile->setDensity(density);
    }

    asw::log::debug(
        "Tile {} ({}): {}, {} images, render mode {}, {} actions, density {}",
        id, id_str, name, image_count, static_cast<int>(render_mode),
        tile->getActions().size(), tile->getDensity());

    // Bake texture
    tile->bakeTexture(render_mode, alpha);

    // Add to types
    types.push_back(tile);
  }

  asw::log::info("Loaded {} tiles", types.size());

  // Close
  file.close();
}
