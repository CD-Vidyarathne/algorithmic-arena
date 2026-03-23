#pragma once

#include <SFML/System/Vector2.hpp>

struct PathNode {
    sf::Vector2i pos{};
    int gCost = 0;   // cost from start
    int fCost = 0;   // g + h (A*) or just g (Dijkstra)
    sf::Vector2i parent{-1, -1};

    bool operator>(const PathNode &other) const {
        return fCost > other.fCost;
    }
};

