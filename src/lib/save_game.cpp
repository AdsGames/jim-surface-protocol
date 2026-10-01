#include "save_game.h"

#include <asw/asw.h>
#include <algorithm>

namespace {
constexpr const char* ORG = "adsgames";
constexpr const char* APP = "j1m-surface-protocol";
constexpr const char* NAME = "world";

// Raise when the save format changes, so old saves are not read wrong
constexpr int VERSION = 1;

bool is_number_array(const nlohmann::json& data, std::size_t size) {
  if (!data.is_array() || data.size() != size) {
    return false;
  }
  return std::ranges::all_of(data, [](const auto& v) { return v.is_number(); });
}

bool is_int_runs(const nlohmann::json& data) {
  return data.is_array() && data.size() % 2 == 0 &&
         std::ranges::all_of(
             data, [](const auto& v) { return v.is_number_integer(); });
}

// A structure is [type, x, y, z]
bool is_structure(const nlohmann::json& data) {
  return data.is_array() && data.size() == 4 && data[0].is_string() &&
         data[1].is_number_integer() && data[2].is_number_integer() &&
         data[3].is_number_integer();
}

// Check every field the game reads, so loading can not throw on a bad save
bool is_valid(const nlohmann::json& data) {
  if (!data.is_object()) {
    return false;
  }

  const auto map = data.value("map", nlohmann::json());
  const auto player = data.value("player", nlohmann::json());
  const auto costs = data.value("upgrade_costs", nlohmann::json());
  const auto resources = data.value("resources", nlohmann::json());
  if (!map.is_object() || !player.is_object() || !costs.is_object() ||
      !resources.is_object()) {
    return false;
  }

  // Structure links are optional, older saves do not have them
  const auto structures = map.value("structures", nlohmann::json::array());
  const auto structure_tiles =
      map.value("structure_tiles", nlohmann::json::array());
  const bool structures_valid = structures.is_array() &&
                                std::ranges::all_of(structures, is_structure) &&
                                is_int_runs(structure_tiles);

  return map.value("width", nlohmann::json()).is_number_integer() &&
         map.value("depth", nlohmann::json()).is_number_integer() &&
         map.value("seed", nlohmann::json()).is_number() &&
         is_int_runs(map.value("tiles", nlohmann::json())) &&
         structures_valid &&
         is_number_array(player.value("position", nlohmann::json()), 3) &&
         player.value("drill_speed", nlohmann::json()).is_number_integer() &&
         player.value("move_speed", nlohmann::json()).is_number_integer() &&
         is_number_array(data.value("camera", nlohmann::json()), 2) &&
         data.value("time", nlohmann::json()).is_number() &&
         costs.value("drill", nlohmann::json()).is_number_integer() &&
         costs.value("move", nlohmann::json()).is_number_integer() &&
         std::ranges::all_of(
             resources, [](const auto& v) { return v.is_number_integer(); });
}
}  // namespace

bool save_game::exists() {
  return read().has_value();
}

std::optional<nlohmann::json> save_game::read() {
  const auto text = asw::assets::read_save(ORG, APP, NAME);
  if (text.empty()) {
    return std::nullopt;
  }

  auto data = nlohmann::json::parse(text, nullptr, false);
  if (data.is_discarded() || !data.is_object() ||
      data.value("version", 0) != VERSION || !is_valid(data)) {
    asw::log::warn("Save is not readable, it is ignored");
    return std::nullopt;
  }

  return data;
}

bool save_game::write(const nlohmann::json& data) {
  auto versioned = data;
  versioned["version"] = VERSION;

  if (!asw::assets::write_save(ORG, APP, NAME, versioned.dump())) {
    asw::log::error("Could not write save");
    return false;
  }

  return true;
}
