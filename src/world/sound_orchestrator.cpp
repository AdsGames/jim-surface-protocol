#include "sound_orchestrator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>

#include "../tiles/tile_ids.h"
#include "toolbar.h"
#include "world.h"

namespace {
// Seconds to wait for the progression sting before music starts again
constexpr float PROGRESSION_STING_TIME = 18.0F;

// Sound effects in assets/sfx, by file name
constexpr std::array<const char*, 7> SFX = {
    "blocked", "destroy", "drill", "human-impact", "place", "three_tone_2",
    "ui"};

// Volume of the clean ambience when the world is clean, under the music
constexpr float AMBIENCE_VOLUME = 0.35F;

// Seconds for the ambience to fade in at the start and out at the end
constexpr float AMBIENCE_FADE_TIME = 2.0F;

// Volume of the waves when the view is on water
constexpr float WATER_VOLUME = 0.4F;

// Waves are heard from water up to this many tiles from the view middle
constexpr int WATER_RANGE = 8;

// Seconds between searches for water, it changes slowly
constexpr float WATER_SEARCH_TIME = 0.25F;

// How fast the waves volume follows the view, per second
constexpr float WATER_FADE_SPEED = 2.0F;

// Volume and pitch for each effect. Frequent ones vary, so they do not
// sound the same every time
asw::sound::PlayOptions sfx_options(const std::string& name) {
  asw::sound::PlayOptions options;
  options.volume = 0.7F;
  if (name == "destroy") {
    options.volume = 0.25F;
    options.pitch_variation = 0.1F;
    options.volume_variation = 0.1F;
  } else if (name == "human-impact") {
    options.pitch_variation = 0.05F;
  } else if (name == "three_tone_2") {
    options.volume = 0.15F;
  } else if (name == "blocked") {
    options.volume = 0.5F;
  } else if (name == "ui") {
    // Full volume, like UI clicks. The Ui bus sets the level
    options.volume = 1.0F;
    options.bus = asw::sound::Bus::Ui;
  }
  return options;
}
}  // namespace

void SoundOrchestrator::init() {
  music["early"] = asw::assets::load_music("assets/music/early.ogg");
  music["mid"] = asw::assets::load_music("assets/music/intro.ogg");
  music["late"] = asw::assets::load_music("assets/music/late.ogg");

  samples["progression"] =
      asw::assets::load_sample("assets/music/progression.ogg");

  for (const auto* name : SFX) {
    samples[name] =
        asw::assets::load_sample(std::format("assets/sfx/{}.ogg", name));
  }
  samples["clean_ambience"] =
      asw::assets::load_sample("assets/sfx/ambience/forest.ogg");
  samples["water_ambience"] =
      asw::assets::load_sample("assets/sfx/ambience/waves.ogg");

  stop();

  last_progression = 0.0F;
  progression_timer = 0.0F;
  primed = false;
  water_volume = 0.0F;
  water_target = 0.0F;
  water_timer = 0.0F;
}

void SoundOrchestrator::stop() {
  setDrilling(false);
  clean_ambience.stop(AMBIENCE_FADE_TIME);
  water_ambience.stop(AMBIENCE_FADE_TIME);
}

float SoundOrchestrator::waterTarget(World& world) {
  auto& tile_map = world.getTileMap();

  // Middle of the view above the toolbar, in world space
  const auto view = world.getCamera().get_view();
  const auto middle =
      view.position +
      asw::Vec2(view.size.x / 2.0F, (view.size.y - TOOLBAR_HEIGHT) / 2.0F);
  const auto centre = tile_map.getIndexAt(middle);
  if (centre.x < 0) {
    return 0.0F;  // Looking past the edge of the map
  }

  // Nearest water column, in tiles
  float nearest = static_cast<float>(WATER_RANGE) + 1.0F;
  for (int dx = -WATER_RANGE; dx <= WATER_RANGE; ++dx) {
    for (int dy = -WATER_RANGE; dy <= WATER_RANGE; ++dy) {
      const auto surface =
          tile_map.getSurface(asw::Vec2(centre.x + dx, centre.y + dy));
      if (!surface) {
        continue;
      }

      const auto id = tile_map.getTileAtIndex(surface->index)->getTypeId();
      if (id == tile_id::WATER || id == tile_id::TOXIC_WATER) {
        nearest = std::min(nearest, std::hypot(static_cast<float>(dx),
                                               static_cast<float>(dy)));
      }
    }
  }

  const float closeness =
      1.0F - (nearest / static_cast<float>(WATER_RANGE + 1));
  return WATER_VOLUME * std::clamp(closeness, 0.0F, 1.0F);
}

void SoundOrchestrator::keepAmbience(asw::sound::SoundHandle& handle,
                                     const std::string& name,
                                     float volume) {
  if (!handle.is_playing()) {
    asw::sound::PlayOptions options;
    options.volume = volume;
    options.loop = true;
    options.bus = asw::sound::Bus::Ambient;
    options.fade_in_s = AMBIENCE_FADE_TIME;
    handle = asw::sound::play(samples.at(name), options);
  }
  handle.set_volume(volume);
}

void SoundOrchestrator::play(const std::string& name, float screen_x) {
  asw::sound::play_at(samples.at(name), screen_x, sfx_options(name));
}

void SoundOrchestrator::play(const std::string& name) {
  asw::sound::play(samples.at(name), sfx_options(name));
}

void SoundOrchestrator::setDrilling(bool drilling, float progress) {
  if (!drilling) {
    drill_loop.stop(0.1F);
    return;
  }

  if (!drill_loop.is_playing()) {
    auto options = sfx_options("drill");
    options.volume = 0.4F;
    options.loop = true;
    options.fade_in_s = 0.05F;
    drill_loop = asw::sound::play(samples.at("drill"), options);
  }
  drill_loop.set_pitch(0.9F + (0.25F * std::clamp(progress, 0.0F, 1.0F)));
}

void SoundOrchestrator::update(float dt, World& world) {
  auto progression = world.getProgression();

  if (!primed) {
    primed = true;
    last_progression = progression;
  }

  if (progression_timer > 0.0F) {
    progression_timer -= dt;
  }

  // Transition mode
  if (progression >= 0.33F && last_progression < 0.33F) {
    progression_timer = PROGRESSION_STING_TIME;
    asw::sound::stop_music();
    asw::sound::play(samples.at("progression"), 0.5F);
  }

  if (progression >= 0.66F && last_progression < 0.66F) {
    progression_timer = PROGRESSION_STING_TIME;
    asw::sound::stop_music();
    asw::sound::play(samples.at("progression"), 0.5F);
  }

  // Ambience follows how clean the world is
  const float clean = std::clamp(progression, 0.0F, 1.0F);
  keepAmbience(clean_ambience, "clean_ambience", AMBIENCE_VOLUME * clean);

  water_timer -= dt;
  if (water_timer <= 0.0F) {
    water_timer = WATER_SEARCH_TIME;
    water_target = waterTarget(world);
  }
  water_volume += (water_target - water_volume) *
                  std::min(1.0F, dt * WATER_FADE_SPEED);
  keepAmbience(water_ambience, "water_ambience", water_volume);

  // Play music based on progression
  if (!asw::sound::is_music_playing() && progression_timer <= 0.0F) {
    if (progression < 0.33F) {
      asw::sound::play_music(music.at("early"));
    } else if (progression < 0.66F) {
      asw::sound::play_music(music.at("mid"));
    } else {
      asw::sound::play_music(music.at("late"));
    }
  }

  last_progression = progression;
}
