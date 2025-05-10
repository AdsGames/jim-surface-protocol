#pragma once

#include <string>

enum class ActionType {
  DESTROY,
  PURIFY,
};

class ActionResult {
 public:
  ActionType type;
  std::string transition_tile_id;
  std::string drop_resource_id;
};