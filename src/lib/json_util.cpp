#include "json_util.h"

#include <asw/asw.h>
#include <fstream>

std::optional<nlohmann::json> json_util::parse_file(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    asw::log::error("Could not open file {}", path);
    return std::nullopt;
  }

  auto data = nlohmann::json::parse(file, nullptr, false);
  if (data.is_discarded()) {
    asw::log::error("File {} is not valid JSON", path);
    return std::nullopt;
  }

  return data;
}
