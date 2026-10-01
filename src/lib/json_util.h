#pragma once

#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

// Reads JSON without exceptions, so a bad asset or save logs an error
// instead of crashing. Web builds can not catch exceptions
namespace json_util {

// Parse a file, or nothing if it is missing or not valid JSON
std::optional<nlohmann::json> parse_file(const std::string& path);

// Get a string field, or the fallback if it is missing or not a string
inline std::string get_string(const nlohmann::json& object,
                              const char* key,
                              const std::string& fallback = "") {
  if (!object.is_object()) {
    return fallback;
  }
  const auto it = object.find(key);
  return it != object.end() && it->is_string() ? it->get<std::string>()
                                               : fallback;
}

// Get a number field, or the fallback if it is missing or not a number
template <typename T>
T get_number(const nlohmann::json& object, const char* key, T fallback) {
  if (!object.is_object()) {
    return fallback;
  }
  const auto it = object.find(key);
  return it != object.end() && it->is_number() ? it->get<T>() : fallback;
}

// Get an array field, or an empty array if it is missing or not an array
inline nlohmann::json get_array(const nlohmann::json& object, const char* key) {
  if (!object.is_object()) {
    return nlohmann::json::array();
  }
  const auto it = object.find(key);
  return it != object.end() && it->is_array() ? *it : nlohmann::json::array();
}

// Make an id from a display name, e.g. "Toxic Water" to "toxic_water"
inline std::string to_id_string(std::string name) {
  std::ranges::transform(name, name.begin(), [](unsigned char c) {
    return c == ' ' ? '_' : static_cast<char>(std::tolower(c));
  });
  return name;
}

}  // namespace json_util
