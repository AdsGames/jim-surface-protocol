#pragma once

#include <asw/asw.h>
#include <unordered_map>

class World;

class SoundOrchestrator {
 public:
  SoundOrchestrator() = default;

  void init();

  void update(World& world);

 private:
  std::unordered_map<std::string, asw::Sample> samples;
  std::unordered_map<std::string, asw::Music> music;

  float last_progression{0.0F};
};