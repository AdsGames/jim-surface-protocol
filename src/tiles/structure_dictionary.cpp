#include "structure_dictionary.h"

#include <asw/asw.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

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

  std::cout << "Warning: Structure " << id << " not found" << '\n';

  return nullptr;
}

std::shared_ptr<StructureType> StructureDictionary::getStructure(
    const std::string& id_str) {
  auto found = std::find_if(structures.begin(), structures.end(),
                            [&id_str](auto& t) { return t->id_str == id_str; });

  if (found != structures.end()) {
    return *found;
  }

  std::cout << "Warning: Structure " << id_str << " not found" << '\n';

  return nullptr;
}

void StructureDictionary::load(const std::string& path) {
  // Open file or abort if it does not exist
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << path << '\n';
    return;
  }

  // Get first node
  std::cout << "Loading structures..." << '\n';

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

    std::cout << "  Z: " << structure->dimensions.z << '\n';
    std::cout << "  Y: " << structure->dimensions.y << '\n';
    std::cout << "  X: " << structure->dimensions.x << '\n';

    // Add to types
    structures.push_back(structure);
  }

  std::cout << "Loaded " << structures.size() << " structures" << '\n';

  // Close
  file.close();
}
