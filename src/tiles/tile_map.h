#pragma once

#include <asw/asw.h>
#include <array>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "tile.h"

constexpr int MAX_MAP_WIDTH = 100;
constexpr int MAX_MAP_DEPTH = 100;
constexpr int MAP_HEIGHT = 16;

constexpr float TICK_TIME = 0.1F;

// Purifiers and trees work on tiles up to this many columns away, in x and y
constexpr int PURIFY_RANGE = 10;

// The visible ground at a spot: the tile whose top shows, and how far that
// top sits below a full cube
struct Surface {
  asw::Vec3<int> index;
  float drop{0.0F};

  // Top face in world space
  asw::Polygonf face() const {
    auto points = isoDiamond(index, TILE_HEIGHT_F);
    for (auto& point : points) {
      point.y += drop;
    }
    return points;
  }

  // Middle of the top face in world space
  asw::Vec2<float> center() const {
    return asw::Vec2(
        isoX(index) * TILE_HEIGHT_F + TILE_HEIGHT_F,
        isoY(index) * TILE_HEIGHT_F + (TILE_HEIGHT_F / 2.0F) + drop);
  }
};

class TileMap {
 public:
  TileMap() = default;

  void update(float dt);

  void draw(const asw::Quad<float>& camera,
            const asw::Vec3<int>& start,
            const asw::Vec3<int>& end);

  void generate();
  // only_empty: fill only empty cells, e.g. a tree regrowing its leaves
  void generateStructure(const std::string& id_str,
                         const asw::Vec3<int>& position,
                         bool only_empty = false);

  Tile* getTileAt(const asw::Vec2<float>& position);
  Tile* getTileAtIndex(const asw::Vec3<int>& index);
  Tile* getTopTileAt(const asw::Vec2<int>& index);

  // Ground of a column, the top tile that is not an item. Empty if the
  // column has no ground
  std::optional<Surface> getSurface(const asw::Vec2<int>& column);

  // Ground a tile shows: its own top, or for an item, the tile it stands on
  Surface getSurfaceOf(const asw::Vec3<int>& index);

  asw::Vec3<int> getIndexAt(const asw::Vec2<float>& position);

  void setSelectedIndex(const asw::Vec3<int>& index) { selected_index = index; }

  // Columns the worker is on or will drive through. Structures do not grow
  // over them, so the worker is never trapped on top of a new tree
  void setReservedColumns(std::vector<asw::Vec2<int>> columns) {
    reserved_columns = std::move(columns);
  }

  int countByType(int type) const;

  // Grow a tile in, e.g. a building the player just placed
  void addPop(const asw::Vec3<int>& index);

  // Tiles purified since the last call, for effects
  std::vector<asw::Vec3<int>> takePurified();

  // Map size, seed and tile types. Structure links are not kept
  nlohmann::json save() const;
  void load(const nlohmann::json& data);

  static int MAP_WIDTH;
  static int MAP_DEPTH;
  static float SEED;

 private:
  void draw_layer(const asw::Quad<float>& camera,
                  int layer,
                  const asw::Vec3<int>& start,
                  const asw::Vec3<int>& end);

  void tick_tile(const asw::Vec3<int>& index);

  // True if the structure would cover a reserved column
  bool coversReservedColumn(const std::string& id_str,
                            const asw::Vec3<int>& position) const;

  // Count every tile type. Ticks recount as they go, a new or loaded map
  // needs it at once, or it reads the last map's counts
  void recount();

  // 3D Map
  std::array<std::array<std::array<Tile, MAP_HEIGHT>, MAX_MAP_DEPTH>,
             MAX_MAP_WIDTH>
      mapTiles;

  // Tile count
  std::array<int, 256> tileCount{0};

  // Selected index
  asw::Vec3<int> selected_index{-1, -1, -1};

  std::vector<asw::Vec2<int>> reserved_columns;

  // Tick
  float tick_timer{0.0F};

  // Tiles growing in, with how long they have grown
  struct Pop {
    asw::Vec3<int> index;
    float age;
  };
  std::vector<Pop> pops;

  std::vector<asw::Vec3<int>> purified;
};
