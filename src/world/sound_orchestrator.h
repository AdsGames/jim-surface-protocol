#pragma once

#include <asw/asw.h>
#include <string>
#include <unordered_map>

class World;

class SoundOrchestrator {
 public:
  SoundOrchestrator() = default;

  void init();

  void update(float dt, World& world);

  // Play a sound effect from assets/sfx, panned by where it happens on screen
  void play(const std::string& name, float screen_x);

  // Play a sound effect from assets/sfx, not panned
  void play(const std::string& name);

  // Keep the drill loop going while drilling is true. progress is from 0 to
  // 1 and raises the pitch as the tile is about to break
  void setDrilling(bool drilling, float progress = 0.0F);

  // Stop every loop, when leaving the game
  void stop();

 private:
  std::unordered_map<std::string, asw::Sample> samples;
  std::unordered_map<std::string, asw::Music> music;

  float last_progression{0.0F};
  float progression_timer{0.0F};

  asw::sound::SoundHandle drill_loop;

  // Ambience bed, fades in as the world is purified
  asw::sound::SoundHandle clean_ambience;

  // Waves, louder the closer water is to the middle of the view
  asw::sound::SoundHandle water_ambience;
  float water_volume{0.0F};
  float water_target{0.0F};
  float water_timer{0.0F};

  // Volume for the waves from the nearest water to the middle of the view
  float waterTarget(World& world);

  // Keep an ambience bed looping at this volume
  void keepAmbience(asw::sound::SoundHandle& handle,
                    const std::string& name,
                    float volume);

  // False until the first update, so a loaded world plays no old stings
  bool primed{false};
};