#pragma once

#include "IPathfindingSystem.h"
#include "PathNode.h"
#include <SFML/Graphics/Color.hpp>
#include <queue>
#include <unordered_map>
#include <vector>

class DijkstrasPathfindingSystem : public IPathfindingSystem {
  public:
    std::vector<sf::Vector2i> findPath(sf::Vector2i start,
                                       sf::Vector2i end,
                                       const TileMap &map) override;
    void drawDebug(sf::RenderWindow &window, const TileMap &map) override;

  private:
    std::vector<sf::Vector2i> lastClosedSet_;
    std::vector<sf::Vector2i> lastOpenSet_;
    std::vector<sf::Vector2i> lastPath_;
};

