#pragma once

#include "../../Entities/EntityManager.h"
#include "../../World/TileMap.h"
#include <SFML/Graphics/RenderWindow.hpp>

class ICollisionSystem {
  public:
    virtual ~ICollisionSystem() = default;
    
    virtual void update(EntityManager& entities, const TileMap& map) = 0;
    virtual void drawDebug(sf::RenderWindow& window) = 0;
};

