#pragma once

#include "Entities/EntityManager.h"
#include "World/MapLoader.h"
#include "World/TextureManager.h"
#include "World/TileMap.h"
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <memory>
#include <vector>

class Entity;
class Minion;
class PlayerCommander;

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
    void renderMinimap();
    bool isMouseInMinimap(sf::Vector2i pixel) const;
    void moveCameraToMinimapPosition(sf::Vector2i pixel);
    void moveCameraToNextFlag();

    void initializeTileMap();
    void spawnMinion();

    sf::RenderWindow window_;
    std::unique_ptr<TextureManager> textureManager_;
    std::unique_ptr<TileMap> tileMap_;
    sf::Clock clock_;
    EntityManager entityManager_;

    sf::View gameView_;
    sf::FloatRect minimapViewport_{sf::Vector2f(0.75f, 0.75f), sf::Vector2f(0.25f, 0.25f)};  // normalized 0-1
    sf::FloatRect minimapPixelBounds_;  // updated each frame in renderMinimap for exact hit test
    bool minimapVisible_ = false;  // default off, toggle with M
    std::vector<sf::Vector2i> flagTilePositions_;
    size_t flagCycleIndex_ = 0;

    Entity* cameraTarget_ = nullptr;
    PlayerCommander* commander_ = nullptr;

    int maxMinions_ = 100;
    std::vector<sf::Vector2i> deployZone_;
    std::vector<Minion*> minions_;
    float spawnCooldown_ = 0.f;
};
