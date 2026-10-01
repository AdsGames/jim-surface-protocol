#pragma once

#include <asw/asw.h>
#include <map>
#include <memory>
#include <string>

#include "../lib/json_util.h"

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
    resources.clear();

    const auto data = json_util::parse_file(path);
    if (!data || !data->is_array()) {
      asw::log::error("Resources in {} must be a list", path);
      return;
    }

    asw::log::info("Loading resources...");

    for (const auto& cResource : *data) {
      const auto name = json_util::get_string(cResource, "name");
      if (name.empty()) {
        asw::log::error("Resource has no name, skipped");
        continue;
      }

      auto resource = std::make_shared<ResourceType>();
      resource->id = json_util::get_number(cResource, "id", 0);
      resource->id_str = json_util::to_id_string(name);
      resource->name = name;
      resource->description = json_util::get_string(cResource, "description");
      resource->amount = 0;

      const auto icon_path = json_util::get_string(cResource, "icon");
      if (!icon_path.empty()) {
        resource->icon = asw::assets::load_texture(icon_path);
      }

      asw::log::debug("Resource {} ({}): {}, icon {}", resource->id,
                      resource->id_str, name, icon_path);

      resources[resource->id_str] = resource;
    }

    asw::log::info("Loaded {} resources", resources.size());
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