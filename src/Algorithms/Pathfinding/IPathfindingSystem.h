#pragma once

#include "../../World/TileMap.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class IPathfindingSystem {
  public:
    virtual ~IPathfindingSystem() = default;

  
    virtual std::vector<sf::Vector2i> findPath(sf::Vector2i start,
                                               sf::Vector2i end,
                                               const TileMap &map) = 0;

   
    virtual void drawDebug(sf::RenderWindow &window, const TileMap &map) = 0;
};

