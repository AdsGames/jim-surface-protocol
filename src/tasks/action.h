#pragma once

#include <string>

enum class ActionType {
  DESTROY,
  PURIFY,
  TICK,
};

enum class TickType {
  NONE,
  PURIFY,
  GROWTH,
  STRUCTURE,
};

class ActionResult {
 public:
  ActionType type;
  std::string transition_tile_id;
  std::string drop_resource_id;
  std::string spawn_structure_id;
  TickType tick_type{TickType::NONE};
  float chance{0.0f};
};