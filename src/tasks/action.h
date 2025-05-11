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
};

class ActionResult {
 public:
  ActionType type;
  std::string transition_tile_id;
  std::string drop_resource_id;
  TickType tick_type{TickType::NONE};
};