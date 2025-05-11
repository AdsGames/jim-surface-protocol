#include "tile_map.h"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "../tiles/structure_dictionary.h"
#include "../vendor/simplex_noise.h"

int TileMap::MAP_WIDTH = MAX_MAP_WIDTH;
int TileMap::MAP_DEPTH = MAX_MAP_DEPTH;
float TileMap::SEED = 0.0F;

void TileMap::update(float dt) {
  // Tick timer
  tick_timer += dt;
  if (tick_timer < TICK_TIME) {
    return;
  }

  // Subtract tick timer
  tick_timer -= TICK_TIME;

  // Reset tile count
  for (unsigned int i = 0; i < tileCount.size(); i++) {
    tileCount[i] = 0;
  }

  // Update tiles
  for (int i = 0; i < MAP_WIDTH; i++) {
    for (int j = 0; j < MAP_DEPTH; j++) {
      for (int k = 0; k < MAP_HEIGHT; k++) {
        const int tile_id = mapTiles[i][j][k].getTypeId();
        if (tile_id == 0) {
          continue;
        }
        tileCount[tile_id] += 1;

        tick_tile({i, j, k});
      }
    }
  }
}

void TileMap::tick_tile(const asw::Vec3<int>& index) {
  // Tick actions
  auto type = mapTiles[index.x][index.y][index.z].getType();
  if (type->getActions().empty()) {
    return;
  }

  // Random point selection
  auto point = index;
  point.x += asw::random::between(-10, 10);
  point.y += asw::random::between(-10, 10);

  for (const auto& action : type->getActions()) {
    if (action.type != ActionType::TICK) {
      continue;
    }

    // Chance
    if (!asw::random::chance(action.chance)) {
      continue;
    }

    if (action.tick_type == TickType::PURIFY) {
      for (int i = 0; i < MAP_HEIGHT; i++) {
        point.z = i;
        auto* tile = getTileAtIndex(point);
        if (tile == nullptr || tile->getType() == nullptr ||
            tile->getType()->getActions().empty() ||
            tile->getType()->getIdString() != action.transition_tile_id) {
          continue;
        }

        for (const auto& action2 : tile->getType()->getActions()) {
          if (action2.type == ActionType::PURIFY) {
            mapTiles[point.x][point.y][point.z].setType(
                action2.transition_tile_id);
          }
        }
      }
    }

    if (action.tick_type == TickType::STRUCTURE) {
      auto point = index;
      point.z += 1;

      if (action.spawn_structure_id.empty()) {
        std::cerr << "Error: No structure ID provided" << std::endl;
        continue;
      }

      generateStructure(action.spawn_structure_id, point);
    }

    if (action.tick_type == TickType::GROWTH) {
      auto point = index;
      point.z += 1;

      if (action.transition_tile_id.empty()) {
        std::cerr << "Error: No tile ID provided" << std::endl;
        continue;
      }

      auto* tile = getTileAtIndex(point);
      if (tile != nullptr && tile->getType() == nullptr) {
        mapTiles[point.x][point.y][point.z].setType(action.transition_tile_id);
      }
    }
  }
}

void TileMap::generate() {
  // Init map
  for (int i = 0; i < MAX_MAP_WIDTH; ++i) {
    for (int j = 0; j < MAX_MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        mapTiles[i][j][k] = Tile();
        auto position = asw::Vec3<int>(i, j, k);
        mapTiles[i][j][k].setPosition(position);
      }
    }
  }

  // Height
  auto height_map = SimplexNoise(0.01f, 0.01f, 2.0f, 0.47f);

  // Dirt grass and rocks
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      auto frac = height_map.fractal(100, SEED + i, SEED + j);
      auto height_val =
          static_cast<int>((MAP_HEIGHT / 2.0F) * (frac + 1.0F) / 2.0F);

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
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("tire");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("junk");

          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("scrap_pile");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("dead_shrub");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("tire_stack");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("garbage");

          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("bike");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("crate");
          } else if (asw::random::chance(0.01F)) {
            mapTiles[i][j][k].setType("barrel");
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_1", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_2", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pile_3", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("junk_pyramid", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("bike_shop", {i, j, k});
          } else if (asw::random::chance(0.0005F)) {
            generateStructure("statue", {i, j, k});
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
        if (id == 0) {
          continue;
        }

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
      index.y >= MAP_DEPTH || index.z < 0 || index.z >= MAP_HEIGHT) {
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

      if (i == selected_index.x && j == selected_index.y &&
          layer == selected_index.z) {
        tile.draw(camera.position, empty_left, empty_right, true);
      } else {
        tile.draw(camera.position, empty_left, empty_right, false);
      }
    }
  }
}

int TileMap::countByType(int type) const {
  if (type < 0 || type >= tileCount.size()) {
    return 0;
  }
  return tileCount[type];
}