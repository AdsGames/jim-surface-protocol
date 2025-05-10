#pragma once

#include <asw/asw.h>
#include <memory>
#include <string>
#include <vector>

class StructureType {
 public:
  int id;
  std::string id_str;
  std::string name;
  std::string description;
  asw::Vec3<int> dimensions;
  std::vector<int> tiles;
  int tile_count;
};

class Structure {
 public:
  Structure();

  void setPosition(const asw::Vec3<int>& position);

  std::shared_ptr<StructureType> getType() const;

  void setType(const std::string& type);

  int getId() const;

 private:
  static int id_counter;

  int id;

  std::shared_ptr<StructureType> type{nullptr};

  asw::Vec3<int> position{0, 0, 0};
};

class StructureDictionary {
 public:
  static void load(const std::string& path);
  static std::shared_ptr<StructureType> getStructure(int id);
  static std::shared_ptr<StructureType> getStructure(const std::string& id_str);

 private:
  static std::vector<std::shared_ptr<StructureType>> structures;
};
