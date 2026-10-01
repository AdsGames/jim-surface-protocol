#include "tile_map.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <utility>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_ids.h"
#include "../vendor/simplex_noise.h"

int TileMap::MAP_WIDTH = MAX_MAP_WIDTH;
int TileMap::MAP_DEPTH = MAX_MAP_DEPTH;
float TileMap::SEED = 0.0F;

namespace {
// Seconds for a new tile to grow in
constexpr float POP_TIME = 0.35F;
}  // namespace

void TileMap::addPop(const asw::Vec3<int>& index) {
  pops.push_back({index, 0.0F});
}

std::vector<asw::Vec3<int>> TileMap::takePurified() {
  return std::exchange(purified, {});
}

void TileMap::update(float dt) {
  // Grow tiles in every frame, not only on ticks
  for (auto& pop : pops) {
    pop.age += dt;
  }
  std::erase_if(pops, [](const Pop& pop) { return pop.age >= POP_TIME; });

  // Tick timer
  tick_timer += dt;
  if (tick_timer < TICK_TIME) {
    return;
  }

  // Subtract tick timer. Drop the backlog after a long pause, such as a
  // hidden browser tab, so the world does not race to catch up
  tick_timer = std::min(tick_timer - TICK_TIME, TICK_TIME);

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

void TileMap::recount() {
  tileCount.fill(0);
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        tileCount[mapTiles[i][j][k].getTypeId()] += 1;
      }
    }
  }
  tileCount[tile_id::NONE] = 0;
}

void TileMap::tick_tile(const asw::Vec3<int>& index) {
  // Tick actions
  auto type = mapTiles[index.x][index.y][index.z].getType();
  if (type->getActions().empty()) {
    return;
  }

  // Random point selection
  auto point = index;
  point.x += asw::random::between(-PURIFY_RANGE, PURIFY_RANGE);
  point.y += asw::random::between(-PURIFY_RANGE, PURIFY_RANGE);

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
            purified.push_back(point);
          }
        }
      }
    }

    if (action.tick_type == TickType::STRUCTURE) {
      auto point = index;
      point.z += 1;

      if (action.spawn_structure_id.empty()) {
        asw::log::error("No structure ID provided");
        continue;
      }

      // Centre the structure on the tile, e.g. a tree's trunk over its
      // sapling
      const auto structure_def =
          StructureDictionary::getStructure(action.spawn_structure_id);
      if (structure_def != nullptr) {
        point.x -= structure_def->dimensions.x / 2;
        point.y -= structure_def->dimensions.y / 2;
      }

      // Wait for the worker to move away. A later tick tries again
      if (coversReservedColumn(action.spawn_structure_id, point)) {
        continue;
      }

      // The tile keeps ticking under its structure, so a tree regrows
      // leaves that were drilled, without covering anything built since
      generateStructure(action.spawn_structure_id, point, true);
    }

    if (action.tick_type == TickType::GROWTH) {
      auto point = index;
      point.z += 1;

      if (action.transition_tile_id.empty()) {
        asw::log::error("No tile ID provided");
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
  pops.clear();
  purified.clear();

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
            mapTiles[i][j][k].setType(tile_id::TOXIC_GRASS);
          } else if (k + 3 >= height_val) {
            mapTiles[i][j][k].setType(tile_id::TOXIC_SOIL);
          } else {
            mapTiles[i][j][k].setType(tile_id::ROCKS);
          }
        }

        else if (k < 4) {
          mapTiles[i][j][k].setType(tile_id::TOXIC_WATER);
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
          } else if (asw::random::chance(0.03F)) {
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
          } else if (asw::random::chance(0.001F)) {
            generateStructure("statue", {i, j, k});
          } else if (asw::random::chance(0.001F)) {
            generateStructure("tire_pile", {i, j, k});
          }
        }
      }
    }
  }

  recount();
  tick_timer = 0.0F;
}

void TileMap::generateStructure(const std::string& id_str,
                                const asw::Vec3<int>& position,
                                bool only_empty) {
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
        if (tile == nullptr || (only_empty && tile->getType() != nullptr)) {
          continue;
        }

        tile->setType(id);
        tile->setStructure(structure);
      }
    }
  }
}

bool TileMap::coversReservedColumn(const std::string& id_str,
                                   const asw::Vec3<int>& position) const {
  const auto structure_def = StructureDictionary::getStructure(id_str);
  if (structure_def == nullptr) {
    return false;
  }

  const auto& size = structure_def->dimensions;
  return std::ranges::any_of(reserved_columns, [&](const auto& column) {
    return column.x >= position.x && column.x < position.x + size.x &&
           column.y >= position.y && column.y < position.y + size.y;
  });
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

Tile* TileMap::getTopTileAt(const asw::Vec2<int>& index) {
  // Get top tile
  for (int i = MAP_HEIGHT - 1; i >= 0; --i) {
    auto* tile = getTileAtIndex({index.x, index.y, i});
    if (tile != nullptr && tile->getType() != nullptr) {
      return tile;
    }
  }

  return nullptr;
}

std::optional<Surface> TileMap::getSurface(const asw::Vec2<int>& column) {
  for (int k = MAP_HEIGHT - 1; k >= 0; --k) {
    const auto* tile = getTileAtIndex({column.x, column.y, k});
    if (tile != nullptr && tile->getType() != nullptr &&
        !tile->getType()->isItem()) {
      return Surface{tile->getPosition(), tile->getType()->surfaceDrop()};
    }
  }
  return std::nullopt;
}

Surface TileMap::getSurfaceOf(const asw::Vec3<int>& index) {
  const auto* tile = getTileAtIndex(index);
  if (tile == nullptr || tile->getType() == nullptr) {
    return {index, 0.0F};
  }

  // Items stand on the tile below
  if (tile->getType()->isItem()) {
    const auto* below = getTileAtIndex({index.x, index.y, index.z - 1});
    const float drop = below != nullptr && below->getType() != nullptr
                           ? below->getType()->surfaceDrop()
                           : 0.0F;
    return {{index.x, index.y, index.z - 1}, drop};
  }

  return {index, tile->getType()->surfaceDrop()};
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
void TileMap::draw(const asw::Quad<float>& camera,
                   const asw::Vec3<int>& start,
                   const asw::Vec3<int>& end) {
  for (int i = 0; i < MAP_HEIGHT; ++i) {
    draw_layer(camera, i, start, end);
  }
}

// Draw a layer
void TileMap::draw_layer(const asw::Quad<float>& camera,
                         int layer,
                         const asw::Vec3<int>& start,
                         const asw::Vec3<int>& end) {
  if (layer < start.z || layer > end.z) {
    return;
  }

  // Tiles on screen, from the iso projection. A tile is drawn at
  // x = (i - j) * TILE_HEIGHT and y = ((i + j) / 2 - layer) * TILE_HEIGHT, and
  // is at most TILE_SIZE across. One tile of padding covers rounding
  const float tile = TILE_HEIGHT_F;
  const auto diff_min =
      static_cast<int>(std::floor((camera.position.x - TILE_SIZE) / tile)) - 1;
  const auto diff_max =
      static_cast<int>(std::ceil((camera.position.x + camera.size.x) / tile)) +
      1;
  const auto sum_min =
      static_cast<int>(
          std::floor(2.0F * ((camera.position.y - TILE_SIZE) / tile + layer))) -
      1;
  const auto sum_max =
      static_cast<int>(std::ceil(
          2.0F * ((camera.position.y + camera.size.y) / tile + layer))) +
      1;

  for (int i = 0; i < TileMap::MAP_WIDTH; ++i) {
    if ((start.z == layer && i < start.x)) {
      continue;
    }

    const int j_min = std::max({0, i - diff_max, sum_min - i});
    const int j_max =
        std::min({TileMap::MAP_DEPTH - 1, i - diff_min, sum_max - i});

    for (int j = j_min; j <= j_max; ++j) {
      if ((start.z == layer && j < start.y)) {
        continue;
      }

      // The pass after the player draws these, so they cover the player
      if (end.z == layer && i >= end.x && j >= end.y) {
        continue;
      }

      const auto& tile = mapTiles[i][j][layer];

      if (tile.getType() == nullptr) {
        continue;
      }

      auto empty_left = i == 0 ||
                        mapTiles[i - 1][j][layer].getType() == nullptr ||
                        !mapTiles[i - 1][j][layer].getType()->isOpaque();

      auto empty_right = j == 0 ||
                         mapTiles[i][j - 1][layer].getType() == nullptr ||
                         !mapTiles[i][j - 1][layer].getType()->isOpaque();

      // Tiles growing in overshoot a little, then settle
      float scale = 1.0F;
      for (const auto& pop : pops) {
        if (pop.index.x == i && pop.index.y == j && pop.index.z == layer) {
          scale = asw::easing::ease_out_back(pop.age / POP_TIME);
        }
      }

      const bool selected = i == selected_index.x && j == selected_index.y &&
                            layer == selected_index.z;
      tile.draw(camera.position, empty_left, empty_right, selected, scale);
    }
  }
}

int TileMap::countByType(int type) const {
  if (type < 0 || static_cast<size_t>(type) >= tileCount.size()) {
    return 0;
  }
  return tileCount[type];
}

namespace {
// Writes values as [value, count] runs. Most of a column is one value
class RunWriter {
 public:
  void add(int value) {
    if (value == run_value && count > 0) {
      ++count;
      return;
    }
    flush();
    run_value = value;
    count = 1;
  }

  nlohmann::json finish() {
    flush();
    return std::move(runs);
  }

 private:
  void flush() {
    if (count > 0) {
      runs.push_back(run_value);
      runs.push_back(count);
    }
  }

  nlohmann::json runs = nlohmann::json::array();
  int run_value{0};
  int count{0};
};

// Reads [value, count] runs back one value at a time. Gives the fallback
// once the runs are used up
class RunReader {
 public:
  RunReader(const nlohmann::json& runs, int fallback)
      : runs(runs), fallback(fallback) {}

  int next() {
    if (left == 0 && index + 1 < runs.size()) {
      value = runs[index].get<int>();
      left = std::max(runs[index + 1].get<int>(), 0);
      index += 2;
    }
    if (left == 0) {
      return fallback;
    }
    --left;
    return value;
  }

 private:
  const nlohmann::json& runs;
  int fallback;
  std::size_t index{0};
  int value{0};
  int left{0};
};
}  // namespace

nlohmann::json TileMap::save() const {
  RunWriter tiles;
  RunWriter structure_tiles;

  // Each structure once, then each tile points at it by index + 1, 0 for none
  auto structures = nlohmann::json::array();
  std::unordered_map<const Structure*, int> structure_index;

  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        const auto& tile = mapTiles[i][j][k];
        tiles.add(tile.getTypeId());

        const auto structure = tile.getStructure();
        if (structure == nullptr || structure->getType() == nullptr) {
          structure_tiles.add(0);
          continue;
        }

        auto [found, added] = structure_index.try_emplace(
            structure.get(), static_cast<int>(structures.size()) + 1);
        if (added) {
          const auto& position = structure->getPosition();
          structures.push_back({structure->getType()->id_str, position.x,
                                position.y, position.z});
        }
        structure_tiles.add(found->second);
      }
    }
  }

  return {
      {"width", MAP_WIDTH},
      {"depth", MAP_DEPTH},
      {"seed", SEED},
      {"tiles", tiles.finish()},
      {"structures", structures},
      {"structure_tiles", structure_tiles.finish()},
  };
}

void TileMap::load(const nlohmann::json& data) {
  pops.clear();
  purified.clear();

  MAP_WIDTH = std::clamp(data.at("width").get<int>(), 1, MAX_MAP_WIDTH);
  MAP_DEPTH = std::clamp(data.at("depth").get<int>(), 1, MAX_MAP_DEPTH);
  SEED = data.at("seed").get<float>();

  // Saves from before structure links have neither field
  std::vector<std::shared_ptr<Structure>> structures;
  for (const auto& entry : data.value("structures", nlohmann::json::array())) {
    auto structure = std::make_shared<Structure>();
    structure->setType(entry.at(0).get<std::string>());
    structure->setPosition({entry.at(1).get<int>(), entry.at(2).get<int>(),
                            entry.at(3).get<int>()});
    structures.push_back(structure);
  }

  const auto structure_runs =
      data.value("structure_tiles", nlohmann::json::array());
  RunReader tiles(data.at("tiles"), tile_id::NONE);
  RunReader structure_tiles(structure_runs, 0);

  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        auto& tile = mapTiles[i][j][k];
        tile = Tile();
        tile.setPosition({i, j, k});
        tile.setType(static_cast<short>(tiles.next()));

        const int structure = structure_tiles.next();
        if (structure > 0 &&
            static_cast<std::size_t>(structure) <= structures.size()) {
          tile.setStructure(structures[structure - 1]);
        }
      }
    }
  }

  recount();
  tick_timer = 0.0F;
}
