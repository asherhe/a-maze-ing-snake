#include "snake.h"

int main()
{
  SDL_SetMainReady();

  Game game;

  game.init();

  while (game.isRunning())
  {
    game.handleEvents();
    game.update();
    game.render();
  }

  game.clean();
  return 0;
}
