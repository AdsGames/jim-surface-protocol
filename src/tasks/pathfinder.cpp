#include "pathfinder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>

#include "../tiles/tile_ids.h"
#include "../tiles/tile_map.h"

namespace {
constexpr float DIAGONAL_COST = 1.41421356F;

// Extra cost for each block up or down, so flat routes win when close
constexpr float CLIMB_COST = 0.5F;

constexpr std::array<std::pair<int, int>, 8> STEPS = {{
    {1, 0},
    {-1, 0},
    {0, 1},
    {0, -1},
    {1, 1},
    {1, -1},
    {-1, 1},
    {-1, -1},
}};

bool in_map(const asw::Vec2<int>& column) {
  return column.x >= 0 && column.x < TileMap::MAP_WIDTH && column.y >= 0 &&
         column.y < TileMap::MAP_DEPTH;
}

// Straight and diagonal distance with no obstacles, never more than the
// real cost, so A* finds the shortest route
float estimate(const asw::Vec2<int>& from, const asw::Vec2<int>& to) {
  const auto dx = static_cast<float>(std::abs(from.x - to.x));
  const auto dy = static_cast<float>(std::abs(from.y - to.y));
  return (std::max(dx, dy) - std::min(dx, dy)) +
         (std::min(dx, dy) * DIAGONAL_COST);
}

bool is_water(TileMap& map, const asw::Vec2<int>& column) {
  const auto surface = map.getSurface(column);
  if (!surface) {
    return false;
  }
  const auto id = map.getTileAtIndex(surface->index)->getTypeId();
  return id == tile_id::WATER || id == tile_id::TOXIC_WATER;
}

int ground_height(TileMap& map, const asw::Vec2<int>& column) {
  const auto surface = map.getSurface(column);
  return surface ? surface->index.z : 0;
}
}  // namespace

bool pathfinding::isWalkable(TileMap& map, const asw::Vec2<int>& column) {
  if (!in_map(column)) {
    return false;
  }

  const auto surface = map.getSurface(column);
  if (!surface) {
    return false;
  }

  // Clean water is shallow enough to drive through, to reach purifier spots
  const auto* tile = map.getTileAtIndex(surface->index);
  return tile->getTypeId() != tile_id::TOXIC_WATER &&
         tile->getType()->getDensity() == 0;
}

std::optional<std::vector<asw::Vec2<int>>> pathfinding::findPath(
    TileMap& map,
    const asw::Vec2<int>& start,
    const asw::Vec2<int>& goal) {
  if (!in_map(start) || !in_map(goal)) {
    return std::nullopt;
  }

  // A blocked goal, such as a crate to drill or water to purify, is
  // reached from any column next to it
  const bool goal_walkable = isWalkable(map, goal);
  const auto reached = [&](const asw::Vec2<int>& column) {
    if (goal_walkable) {
      return column.x == goal.x && column.y == goal.y;
    }
    return std::max(std::abs(column.x - goal.x), std::abs(column.y - goal.y)) ==
           1;
  };

  if (reached(start)) {
    return std::vector<asw::Vec2<int>>{};
  }

  const int width = TileMap::MAP_WIDTH;
  const int depth = TileMap::MAP_DEPTH;
  const auto index = [width](const asw::Vec2<int>& c) {
    return (c.y * width) + c.x;
  };

  const auto cells = static_cast<std::size_t>(width * depth);
  std::vector<float> cost(cells, std::numeric_limits<float>::infinity());
  std::vector<int> came_from(cells, -1);
  std::vector<int> height(cells, -1);

  const auto height_at = [&](const asw::Vec2<int>& c) {
    auto& h = height[index(c)];
    if (h < 0) {
      h = ground_height(map, c);
    }
    return h;
  };

  // Lowest estimated total first
  using Entry = std::pair<float, int>;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> open;

  cost[index(start)] = 0.0F;
  open.push({estimate(start, goal), index(start)});

  while (!open.empty()) {
    const auto [priority, current_index] = open.top();
    open.pop();

    const asw::Vec2<int> current{current_index % width, current_index / width};
    if (priority > cost[current_index] + estimate(current, goal) + 0.001F) {
      continue;  // A better route to this column was found already
    }

    if (reached(current)) {
      std::vector<asw::Vec2<int>> path;
      for (int at = current_index; at != index(start); at = came_from[at]) {
        path.push_back({at % width, at / width});
      }
      std::ranges::reverse(path);
      return path;
    }

    for (const auto& [step_x, step_y] : STEPS) {
      const asw::Vec2<int> next{current.x + step_x, current.y + step_y};
      if (!isWalkable(map, next)) {
        continue;
      }

      const int climb = std::abs(height_at(next) - height_at(current));
      if (climb > MAX_STEP) {
        continue;
      }

      // Diagonals may not cut a corner past something solid
      const bool diagonal = step_x != 0 && step_y != 0;
      if (diagonal && (!isWalkable(map, {current.x + step_x, current.y}) ||
                       !isWalkable(map, {current.x, current.y + step_y}))) {
        continue;
      }

      const float next_cost = cost[current_index] +
                              (diagonal ? DIAGONAL_COST : 1.0F) +
                              (static_cast<float>(climb) * CLIMB_COST);
      const int next_index = index(next);
      if (next_cost < cost[next_index]) {
        cost[next_index] = next_cost;
        came_from[next_index] = current_index;
        open.push({next_cost + estimate(next, goal), next_index});
      }
    }
  }

  return std::nullopt;
}

std::optional<asw::Vec2<int>> pathfinding::nearestLand(
    TileMap& map,
    const asw::Vec2<int>& around) {
  const int max_radius = std::max(TileMap::MAP_WIDTH, TileMap::MAP_DEPTH);

  // Rings of growing size around the column
  for (int radius = 0; radius <= max_radius; ++radius) {
    for (int dx = -radius; dx <= radius; ++dx) {
      for (int dy = -radius; dy <= radius; ++dy) {
        if (std::max(std::abs(dx), std::abs(dy)) != radius) {
          continue;
        }

        const asw::Vec2<int> column{around.x + dx, around.y + dy};
        if (isWalkable(map, column) && !is_water(map, column)) {
          return column;
        }
      }
    }
  }

  return std::nullopt;
}
