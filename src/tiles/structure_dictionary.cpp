#include "structure_dictionary.h"

#include <asw/asw.h>
#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>

#include "../lib/json_util.h"

/// Structure Instance
///
int Structure::id_counter = 0;

Structure::Structure() : id(id_counter++) {}

void Structure::setPosition(const asw::Vec3<int>& position) {
  this->position = position;
}

std::shared_ptr<StructureType> Structure::getType() const {
  return type;
}

void Structure::setType(const std::string& type) {
  this->type = StructureDictionary::getStructure(type);
}

int Structure::getId() const {
  return id;
}

/// Structure Type
///
std::vector<std::shared_ptr<StructureType>> StructureDictionary::structures;
std::unordered_map<std::string, std::shared_ptr<StructureType>>
    StructureDictionary::structures_by_name;

/// Structure Dictionary
///
std::shared_ptr<StructureType> StructureDictionary::getStructure(int id) {
  auto found = std::find_if(structures.begin(), structures.end(),
                            [&id](auto& t) { return t->id == id; });

  if (found != structures.end()) {
    return *found;
  }

  asw::log::warn("Structure {} not found", id);

  return nullptr;
}

std::shared_ptr<StructureType> StructureDictionary::getStructure(
    const std::string& id_str) {
  const auto found = structures_by_name.find(id_str);
  if (found != structures_by_name.end()) {
    return found->second;
  }

  asw::log::warn("Structure {} not found", id_str);

  return nullptr;
}

void StructureDictionary::load(const std::string& path) {
  const auto data = json_util::parse_file(path);
  if (!data || !data->is_array()) {
    asw::log::error("Structures in {} must be a list", path);
    return;
  }

  asw::log::info("Loading structures...");

  structures.clear();
  structures_by_name.clear();

  for (const auto& cStructure : *data) {
    const int id = json_util::get_number(cStructure, "id", 0);
    const auto name = json_util::get_string(cStructure, "name");
    if (name.empty()) {
      asw::log::error("Structure {} has no name, skipped", id);
      continue;
    }

    auto structure = std::make_shared<StructureType>();
    structure->id = id;
    structure->id_str = json_util::to_id_string(name);
    structure->name = name;
    structure->description = json_util::get_string(cStructure, "description");

    // Load tile ids (3d array) into 1d vector. Every row and layer must be
    // the same size, since the ids are read back by position
    const auto layout = json_util::get_array(cStructure, "layout");
    bool valid = !layout.empty();
    structure->dimensions.z = static_cast<int>(layout.size());

    for (const auto& z_layer : layout) {
      if (!z_layer.is_array() || z_layer.empty()) {
        valid = false;
        break;
      }
      structure->dimensions.y = static_cast<int>(z_layer.size());

      for (const auto& y_layer : z_layer) {
        if (!y_layer.is_array() ||
            (structure->dimensions.x != 0 &&
             static_cast<int>(y_layer.size()) != structure->dimensions.x)) {
          valid = false;
          break;
        }
        structure->dimensions.x = static_cast<int>(y_layer.size());

        for (const auto& value : y_layer) {
          const int tile = value.is_number_integer() ? value.get<int>() : 0;
          structure->tiles.push_back(tile);
          if (tile != 0) {
            structure->tile_count++;
          }
        }
      }
    }

    const auto expected = static_cast<std::size_t>(structure->dimensions.x) *
                          structure->dimensions.y * structure->dimensions.z;
    if (!valid || structure->tiles.size() != expected) {
      asw::log::error("Structure {} has a bad layout, skipped",
                      structure->id_str);
      continue;
    }

    asw::log::debug("Structure {} ({}): {}, {}x{}x{}", id, structure->id_str,
                    name, structure->dimensions.x, structure->dimensions.y,
                    structure->dimensions.z);

    // Add to types
    structures.push_back(structure);
    structures_by_name[structure->id_str] = structure;
  }

  asw::log::info("Loaded {} structures", structures.size());
}
