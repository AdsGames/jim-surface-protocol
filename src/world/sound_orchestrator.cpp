#include "sound_orchestrator.h"
#include "world.h"

void SoundOrchestrator::init() {
  music["early"] = asw::assets::loadMusic("assets/music/early.ogg");
  music["mid"] = asw::assets::loadMusic("assets/music/intro.ogg");
  music["late"] = asw::assets::loadMusic("assets/music/late.ogg");

  samples["progression"] =
      asw::assets::loadSample("assets/music/progression.ogg");
}

void SoundOrchestrator::update(World& world) {
  auto progression = world.getProgression();

  // Transition mode
  if (progression >= 0.33F && last_progression < 0.33F) {
    asw::sound::stopMusic();
    asw::sound::play(samples.at("progression"), 128, 128, 0);
  }

  if (progression >= 0.66F && last_progression < 0.66F) {
    asw::sound::stopMusic();
    asw::sound::play(samples.at("progression"), 128, 128, 0);
  }

  // Play music based on progression
  if (!Mix_PlayingMusic()) {
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
