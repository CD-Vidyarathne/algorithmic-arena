#pragma once

#include "World/TileMap.h"
#include <SFML/Graphics.hpp>

class Game {
  public:
    Game();
    void run();

  private:
    void processEvents();
    void update(float dt);
    void render();

    void initializeTileMap();

    sf::RenderWindow window_;
    std::unique_ptr<TileMap> tileMap_;
    sf::Clock clock_;
};
