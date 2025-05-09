#include <asw/asw.h>

#include "./states/init.h"
#include "./states/intro.h"
#include "./states/state.h"

// Main function
int main() {
  // Load allegro library
  asw::core::init(1280, 960);

  auto app = asw::scene::SceneManager<ProgramState>();
  app.registerScene<Init>(ProgramState::Init, app);
  app.registerScene<Intro>(ProgramState::Intro, app);
  app.setNextScene(ProgramState::Init);

  app.start();

  return 0;
}
