#pragma once

#include <array>
#include <string_view>

// Ids of tile types the code uses by name. They must match assets/tiles.json,
// which TileDictionary::load checks
namespace tile_id {

inline constexpr short NONE = 0;
inline constexpr short WATER = 5;
inline constexpr short ROCKS = 11;
inline constexpr short GROUND_GRASS = 12;
inline constexpr short GROUND = 13;
inline constexpr short PURIFIER = 14;
inline constexpr short TOXIC_GRASS = 16;
inline constexpr short TOXIC_SOIL = 17;
inline constexpr short TOXIC_WATER = 18;
inline constexpr short SAPLING = 28;
inline constexpr short TRUNK_PURIFIER = 29;

struct Named {
  short id;
  std::string_view id_str;
};

// Every id above with the name it has in assets/tiles.json
inline constexpr std::array<Named, 10> ALL = {{
    {WATER, "water"},
    {ROCKS, "rocks"},
    {GROUND_GRASS, "ground_grass"},
    {GROUND, "ground"},
    {PURIFIER, "purifier"},
    {TOXIC_GRASS, "toxic_grass"},
    {TOXIC_SOIL, "toxic_soil"},
    {TOXIC_WATER, "toxic_water"},
    {SAPLING, "sapling"},
    {TRUNK_PURIFIER, "trunk_purifier"},
}};

}  // namespace tile_id
