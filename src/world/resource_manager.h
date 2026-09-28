#pragma once

#include <asw/asw.h>
#include <array>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

class ResourceType {
 public:
  int id;
  std::string id_str;
  std::string name;
  std::string description;
  asw::Texture icon;
  int amount;
};

class ResourceManager {
 public:
  ResourceManager() = default;

  void load(const std::string& path) {
    // Open file or abort if it does not exist
    std::ifstream file(path);
    if (!file.is_open()) {
      asw::log::error("Could not open file {}", path);
      return;
    }

    // Get first node
    asw::log::info("Loading resources...");

    for (auto const& cTile : nlohmann::json::parse(file)) {
      // Numeric identifier
      const short id = cTile["id"];
      std::string name = cTile["name"];
      std::string description = cTile["description"];
      std::string icon_path = cTile["icon"];

      // Parse name
      std::string id_str = name;
      std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
      std::replace(id_str.begin(), id_str.end(), ' ', '_');

      asw::log::debug("Resource {} ({}): {}, icon {}", id, id_str, name,
                      icon_path);

      // Create resource
      auto resource = std::make_shared<ResourceType>();
      resource->name = name;
      resource->description = description;
      resource->icon = asw::assets::load_texture(icon_path);
      resource->id = id;
      resource->id_str = id_str;
      resource->amount = 0;

      // Add to types
      resources[id_str] = resource;
    }

    asw::log::info("Loaded {} resources", resources.size());

    // Close
    file.close();
  }

  void addResourceCount(const std::string& type, int amount) {
    if (resources.find(type) == resources.end()) {
      asw::log::error("Resource type {} not found", type);
      return;
    }

    resources.at(type)->amount += amount;
  }

  int getResourceCount(const std::string& type) const {
    if (resources.find(type) == resources.end()) {
      asw::log::error("Resource type {} not found", type);
      return 0;
    }

    return resources.at(type)->amount;
  }

  std::shared_ptr<ResourceType> getResource(const std::string& type) const {
    if (resources.find(type) == resources.end()) {
      asw::log::error("Resource type {} not found", type);
      return nullptr;
    }

    return resources.at(type);
  }

  const std::map<std::string, std::shared_ptr<ResourceType>>& getResources()
      const {
    return resources;
  }

 private:
  std::map<std::string, std::shared_ptr<ResourceType>> resources;
};