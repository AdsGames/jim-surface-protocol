#include "tile_map.h"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "../tiles/structure_dictionary.h"
#include "../vendor/simplex_noise.h"

asw::Vec3<int> TileMap::getSize() const {
  return {MAP_WIDTH, MAP_HEIGHT, MAP_DEPTH};
}

void TileMap::update(float dt) {}

void TileMap::generate() {
  // Init map
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        mapTiles[i][j][k] = Tile();
        auto position = asw::Vec3<int>(i, j, k);
        mapTiles[i][j][k].setPosition(position);
      }
    }
  }

  // Height
  auto height_map = SimplexNoise(0.01f, 0.01f, 2.0f, 0.47f);
  auto seed = asw::random::between(0.0F, 10000.0F);

  // Dirt grass and rocks
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      auto frac = height_map.fractal(100, seed + i, seed + j);
      auto height_val = static_cast<int>(MAP_HEIGHT * (frac + 1.0F) / 2.0F);

      for (int k = 0; k < MAP_HEIGHT; ++k) {
        if (mapTiles[i][j][k].getType() != nullptr) {
          continue;
        }

        if (k < height_val) {
          // Grass if at top of ground, unless water
          if (k + 1 >= height_val && k >= 3) {
            mapTiles[i][j][k].setType("toxic_grass");
          } else if (k + 3 >= height_val) {
            mapTiles[i][j][k].setType("toxic_soil");
          } else {
            mapTiles[i][j][k].setType("rocks");
          }
        }

        else if (k < 4) {
          mapTiles[i][j][k].setType("toxic_water");
        }

        // Structures
        else if (k - 1 < height_val) {
          if (asw::random::chance(0.001F)) {
            generateStructure("house", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("dead_tree", {i, j, k});
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("barrel");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("bike");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("truck");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("tire");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("junk");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("crate");
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_1", {i, j, k});

          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_2", {i, j, k});

          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_3", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pyramid", {i, j, k});
          }
        }
      }
    }
  }
}

void TileMap::generateStructure(const std::string& id_str,
                                const asw::Vec3<int>& position) {
  auto structure_def = StructureDictionary::getStructure(id_str);
  if (structure_def == nullptr) {
    return;
  }

  auto structure = std::make_shared<Structure>();
  structure->setType(id_str);
  structure->setPosition(position);

  int index = 0;
  auto tile_offset = asw::Vec3(0, 0, 0);

  for (int i = 0; i < structure_def->dimensions.z; ++i) {
    tile_offset.z = i;

    for (int j = 0; j < structure_def->dimensions.y; ++j) {
      tile_offset.y = j;

      for (int k = 0; k < structure_def->dimensions.x; ++k) {
        auto id = structure_def->tiles[index++];
        tile_offset.x = k;

        auto tile = getTileAtIndex(position + tile_offset);
        if (tile == nullptr) {
          continue;
        }

        tile->setType(id);
        tile->setStructure(structure);
      }
    }
  }
}

Tile* TileMap::getTileAt(const asw::Vec2<float>& position) {
  return getTileAtIndex(getIndexAt(position));
}

Tile* TileMap::getTileAtIndex(const asw::Vec3<int>& index) {
  // Check bounds
  if (index.x < 0 || index.x >= MAP_WIDTH || index.y < 0 ||
      index.y >= MAP_DEPTH) {
    return nullptr;
  }

  return &mapTiles[index.x][index.y][index.z];
}

asw::Vec3<int> TileMap::getIndexAt(const asw::Vec2<float>& position) {
  // "Clamp" position coords to tile coords
  auto sx = position.x / (TILE_SIZE / 2.0F) - 0.5F;
  auto sy = position.y / (TILE_SIZE / 2.0F) - 0.5F;

  // Start index
  auto z_focus = MAP_HEIGHT - 1;

  // Calculate screen coordinates to world coordinates using iso coords
  while (z_focus >= 0) {
    auto iso_y = static_cast<int>(std::round((2 * (sy + z_focus) - sx) / 2.0F));
    auto iso_x = static_cast<int>(std::round(iso_y + sx));
    auto tile = getTileAtIndex({iso_x, iso_y, z_focus});

    // Check for top tile at index
    if (tile != nullptr && tile->getType() != nullptr) {
      return {iso_x, iso_y, z_focus};
    }

    // Decrease z_focus
    z_focus -= 1;
  }

  // Return vec
  return {-1, -1, -1};
}

// Draw at position
void TileMap::draw(const asw::Quad<float>& camera) {
  for (int i = 0; i < MAP_HEIGHT; ++i) {
    draw_layer(camera, i);
  }
}

// Draw a layer
void TileMap::draw_layer(const asw::Quad<float>& camera, int layer) {
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      auto tile = mapTiles[i][j][layer];

      if (tile.getType() == nullptr) {
        continue;
      }

      auto empty_left = i == 0 ||
                        mapTiles[i - 1][j][layer].getType() == nullptr ||
                        !mapTiles[i - 1][j][layer].getType()->isOpaque();

      auto empty_right = j == 0 ||
                         mapTiles[i][j - 1][layer].getType() == nullptr ||
                         !mapTiles[i][j - 1][layer].getType()->isOpaque();

      tile.draw(camera.position, false, empty_left, empty_right);
    }
  }
}