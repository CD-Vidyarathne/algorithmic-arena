#pragma once

#include "ICollisionSystem.h"

class QuadtreeCollisionSystem : public ICollisionSystem {
  public:
    void update(EntityManager& entities, const TileMap& map) override;
    void drawDebug(sf::RenderWindow& window) override;
};

