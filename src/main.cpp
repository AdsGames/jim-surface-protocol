#include <asw/asw.h>

#include "./states/game.h"
#include "./states/init.h"
#include "./states/intro.h"
#include "./states/menu.h"
#include "./states/state.h"

// Main function
int main() {
  // Load allegro library
  asw::core::init(1280, 960);

  asw::core::print_info();

  auto app = asw::scene::SceneManager<ProgramState>();
  app.register_scene<Init>(ProgramState::Init, app);
  app.register_scene<Game>(ProgramState::Game, app);
  app.register_scene<Intro>(ProgramState::Intro, app);
  app.register_scene<Menu>(ProgramState::Menu, app);
  app.set_next_scene(ProgramState::Init);

  app.start();

  return 0;
}
