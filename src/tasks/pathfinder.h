#pragma once

#include <asw/asw.h>
#include <optional>
#include <vector>

class TileMap;

// Routes for the worker over the map's columns
namespace pathfinding {

// Highest step up or down the worker can drive, in blocks
inline constexpr int MAX_STEP = 1;

// True if the worker can drive on a column: land or clean water with nothing
// solid on it. Toxic water and solid tiles such as crates and walls block it.
// Items, such as plants and junk, are driven through
bool isWalkable(TileMap& map, const asw::Vec2<int>& column);

// Columns from start to goal, not counting start. If the goal is blocked,
// e.g. a crate or water, the route ends next to it. Empty if there is no
// route
std::optional<std::vector<asw::Vec2<int>>> findPath(TileMap& map,
                                                    const asw::Vec2<int>& start,
                                                    const asw::Vec2<int>& goal);

// Nearest dry land to a column, searching outward. Empty if the map has none
std::optional<asw::Vec2<int>> nearestLand(TileMap& map,
                                          const asw::Vec2<int>& around);

}  // namespace pathfinding
