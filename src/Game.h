#pragma once

#include <SFML/Graphics.hpp>

class Game {
public:
  Game();
  void run();

private:
  void processEvents();
  void render();

  sf::RenderWindow window_;
};
