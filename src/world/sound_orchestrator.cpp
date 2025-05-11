#include "sound_orchestrator.h"
#include "world.h"

void SoundOrchestrator::init() {
  music["early"] = asw::assets::loadMusic("assets/music/early.ogg");
  music["mid"] = asw::assets::loadMusic("assets/music/intro.ogg");
  music["late"] = asw::assets::loadMusic("assets/music/late.ogg");

  samples["progression"] =
      asw::assets::loadSample("assets/music/progression.ogg");
}

void SoundOrchestrator::update(float dt, World& world) {
  auto progression = world.getProgression();

  if (progression_timer > 0.0F) {
    progression_timer -= dt;
  }

  // Transition mode
  if (progression >= 0.33F && last_progression < 0.33F) {
    progression_timer = 10000.0F;
    asw::sound::stopMusic();
    asw::sound::play(samples.at("progression"), 64, 128, 0);
  }

  if (progression >= 0.66F && last_progression < 0.66F) {
    progression_timer = 10000.0F;
    asw::sound::stopMusic();
    asw::sound::play(samples.at("progression"), 64, 128, 0);
  }

  // Play music based on progression
  if (!Mix_PlayingMusic() && progression_timer <= 0.0F) {
    if (progression < 0.33F) {
      asw::sound::playMusic(music.at("early"));
    } else if (progression < 0.66F) {
      asw::sound::playMusic(music.at("mid"));
    } else {
      asw::sound::playMusic(music.at("late"));
    }
  }

  last_progression = progression;
}
