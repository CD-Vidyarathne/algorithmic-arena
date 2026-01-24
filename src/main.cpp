#include "Game.h"
#include <spdlog/spdlog.h>

int main() {

  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

  Game game;
  game.run();

  return 0;
}
