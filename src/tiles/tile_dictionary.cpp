#include "tile_dictionary.h"

#include <asw/asw.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

#include "../lib/json_util.h"
#include "tile_ids.h"

std::vector<std::shared_ptr<TileType>> TileDictionary::types_by_id;
std::unordered_map<std::string, std::shared_ptr<TileType>>
    TileDictionary::types_by_name;

namespace {
// Largest tile id, so a bad id can not make a huge lookup table
constexpr int MAX_TILE_ID = 255;

TileRenderMode parse_render_mode(const std::string& mode) {
  if (mode == "flat") {
    return TileRenderMode::FLAT;
  }
  if (mode == "cube_unique_top") {
    return TileRenderMode::CUBE_UNIQUE_TOP;
  }
  if (mode == "cube_top_only") {
    return TileRenderMode::CUBE_TOP_ONLY;
  }
  if (mode != "cube") {
    asw::log::error("Unknown render mode {}", mode);
  }
  return TileRenderMode::CUBE;
}

std::optional<ActionResult> parse_action(const nlohmann::json& action) {
  ActionResult result;

  const auto type = json_util::get_string(action, "type");
  if (type == "destroy") {
    result.type = ActionType::DESTROY;
  } else if (type == "purify") {
    result.type = ActionType::PURIFY;
  } else if (type == "tick") {
    result.type = ActionType::TICK;

    const auto tick_type = json_util::get_string(action, "action");
    if (tick_type == "purify") {
      result.tick_type = TickType::PURIFY;
    } else if (tick_type == "growth") {
      result.tick_type = TickType::GROWTH;
    } else if (tick_type == "structure") {
      result.tick_type = TickType::STRUCTURE;
    } else if (!tick_type.empty()) {
      asw::log::error("Unknown tick type {}", tick_type);
    }
  } else {
    asw::log::error("Unknown action type {}", type);
    return std::nullopt;
  }

  result.transition_tile_id =
      json_util::get_string(action, "transition_tile_id");
  result.spawn_structure_id =
      json_util::get_string(action, "spawn_structure_id");
  result.drop_resource_id = json_util::get_string(action, "drop_resource_id");
  result.chance = json_util::get_number(action, "chance", 0.0F);

  return result;
}
}  // namespace

std::shared_ptr<TileType> TileDictionary::getTile(int id) {
  if (id == tile_id::NONE) {
    return nullptr;
  }

  if (id > 0 && static_cast<std::size_t>(id) < types_by_id.size() &&
      types_by_id[id] != nullptr) {
    return types_by_id[id];
  }

  asw::log::warn("Tile {} not found", id);

  return nullptr;
}

std::shared_ptr<TileType> TileDictionary::getTile(const std::string& id_str) {
  const auto found = types_by_name.find(id_str);
  if (found != types_by_name.end()) {
    return found->second;
  }

  asw::log::warn("Tile {} not found", id_str);

  return nullptr;
}

void TileDictionary::load(const std::string& path) {
  const auto data = json_util::parse_file(path);
  if (!data || !data->is_array()) {
    asw::log::error("Tiles in {} must be a list", path);
    return;
  }

  asw::log::info("Loading tiles...");

  types_by_id.clear();
  types_by_name.clear();

  for (const auto& cTile : *data) {
    // Numeric identifier
    const int id = json_util::get_number(cTile, "id", 0);
    const auto name = json_util::get_string(cTile, "name");
    if (id <= 0 || id > MAX_TILE_ID || name.empty()) {
      asw::log::error("Tile needs an id from 1 to {} and a name, skipped",
                      MAX_TILE_ID);
      continue;
    }

    const auto id_str = json_util::to_id_string(name);
    auto tile =
        std::make_shared<TileType>(static_cast<short>(id), name, id_str);

    // Images
    for (const auto& image : json_util::get_array(cTile, "images")) {
      if (image.is_string()) {
        tile->addImage(asw::assets::load_texture(image.get<std::string>()));
      }
    }

    if (tile->getImageCount() == 0) {
      asw::log::error("Tile {} has no images, skipped", id_str);
      continue;
    }

    const auto render_mode =
        parse_render_mode(json_util::get_string(cTile, "render_mode", "cube"));
    const float alpha = json_util::get_number(cTile, "alpha", 1.0F);

    // Actions
    for (const auto& action : json_util::get_array(cTile, "actions")) {
      if (auto result = parse_action(action)) {
        tile->addAction(*result);
      }
    }

    tile->setDensity(json_util::get_number(cTile, "density", 0));

    asw::log::debug(
        "Tile {} ({}): {}, {} images, render mode {}, {} actions, density {}",
        id, id_str, name, tile->getImageCount(), static_cast<int>(render_mode),
        tile->getActions().size(), tile->getDensity());

    // Bake texture
    tile->bakeTexture(render_mode, alpha);

    // Add to types
    if (types_by_id.size() <= static_cast<std::size_t>(id)) {
      types_by_id.resize(id + 1);
    }
    types_by_id[id] = tile;
    types_by_name[id_str] = tile;
  }

  asw::log::info("Loaded {} tiles", types_by_name.size());

  checkNamedIds();
}

void TileDictionary::checkNamedIds() {
  for (const auto& [id, id_str] : tile_id::ALL) {
    const auto found = types_by_name.find(std::string(id_str));
    if (found == types_by_name.end() || found->second->getId() != id) {
      asw::log::error("Tile {} must have id {} for the game to work", id_str,
                      id);
    }
  }
}
