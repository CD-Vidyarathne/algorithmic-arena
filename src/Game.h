#pragma once

#include "Entities/EntityManager.h"
#include "World/MapLoader.h"
#include "World/TileMap.h"
#include <SFML/Graphics.hpp>

class Entity;

class Game {
  public:
    Game();
    void run();

   
    void setCameraTarget(Entity* entity) { cameraTarget_ = entity; }


    const sf::View& getGameView() const { return gameView_; }

  private:
    void processEvents();
    void update(float dt);
    void render();
    void updateCamera();

    void initializeTileMap();

    sf::RenderWindow window_;
    std::unique_ptr<TileMap> tileMap_;
    sf::Clock clock_;
    EntityManager entityManager_;

    sf::View gameView_;
    Entity* cameraTarget_ = nullptr;
};
