#include "structure_dictionary.h"

#include <asw/asw.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

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
  auto found = std::find_if(structures.begin(), structures.end(),
                            [&id_str](auto& t) { return t->id_str == id_str; });

  if (found != structures.end()) {
    return *found;
  }

  asw::log::warn("Structure {} not found", id_str);

  return nullptr;
}

void StructureDictionary::load(const std::string& path) {
  // Open file or abort if it does not exist
  std::ifstream file(path);
  if (!file.is_open()) {
    asw::log::error("Could not open file {}", path);
    return;
  }

  // Get first node
  asw::log::info("Loading structures...");

  for (auto const& cTile : nlohmann::json::parse(file)) {
    // Numeric identifier
    const short id = cTile["id"];
    std::string name = cTile["name"];

    // Parse name
    std::string id_str = name;
    std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
    std::replace(id_str.begin(), id_str.end(), ' ', '_');

    // Create structure
    auto structure = std::make_shared<StructureType>();
    structure->id = id;
    structure->id_str = id_str;
    structure->name = name;
    structure->description = cTile["description"];

    // Load tile ids (3d array) into 1d vector with height and width markers
    structure->dimensions.z = cTile["layout"].size();

    for (auto const& zLayer : cTile["layout"]) {
      structure->dimensions.y = zLayer.size();

      for (auto const& yLayer : zLayer) {
        structure->dimensions.x = yLayer.size();

        for (int const id : yLayer) {
          structure->tiles.push_back(id);

          if (id != 0) {
            structure->tile_count++;
          }
        }
      }
    }

    asw::log::debug("Structure {} ({}): {}, {}x{}x{}", id, id_str, name,
                    structure->dimensions.x, structure->dimensions.y,
                    structure->dimensions.z);

    // Add to types
    structures.push_back(structure);
  }

  asw::log::info("Loaded {} structures", structures.size());

  // Close
  file.close();
}
