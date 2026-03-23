#pragma once

#include "IPathfindingSystem.h"
#include "PathNode.h"
#include <queue>
#include <vector>

class AStarPathfindingSystem : public IPathfindingSystem {
  public:
    std::vector<sf::Vector2i> findPath(sf::Vector2i start,
                                       sf::Vector2i end,
                                       const TileMap &map) override;
    void drawDebug(sf::RenderWindow &window, const TileMap &map) override;

  private:
    int heuristic(const sf::Vector2i &a, const sf::Vector2i &b) const;

    std::vector<sf::Vector2i> lastClosedSet_;
    std::vector<sf::Vector2i> lastOpenSet_;
    std::vector<sf::Vector2i> lastPath_;
};

