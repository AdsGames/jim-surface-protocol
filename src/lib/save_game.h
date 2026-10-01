#pragma once

#include <nlohmann/json.hpp>
#include <optional>

// Game saves. Desktop builds write a file, web builds use browser storage
namespace save_game {

// Set by the menu so the game loads the save instead of a new map
inline bool load_requested = false;

// Check if there is a save to continue
bool exists();

// Read the save, or nothing if there is none or it can not be read
std::optional<nlohmann::json> read();

// Write the save, replacing the last one
bool write(const nlohmann::json& data);

}  // namespace save_game
