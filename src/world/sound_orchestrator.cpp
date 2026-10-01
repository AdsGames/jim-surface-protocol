#include "sound_orchestrator.h"
#include "world.h"

namespace {
// Seconds to wait for the progression sting before music starts again
constexpr float PROGRESSION_STING_TIME = 18.0F;
}  // namespace

void SoundOrchestrator::init() {
  music["early"] = asw::assets::load_music("assets/music/early.ogg");
  music["mid"] = asw::assets::load_music("assets/music/intro.ogg");
  music["late"] = asw::assets::load_music("assets/music/late.ogg");

  samples["progression"] =
      asw::assets::load_sample("assets/music/progression.ogg");

  last_progression = 0.0F;
  progression_timer = 0.0F;
  primed = false;
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
