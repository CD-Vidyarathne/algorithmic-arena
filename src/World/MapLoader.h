#pragma once
#include "TileMap.h"
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <string>
#include <vector>

constexpr uint8_t FLAG_DEPLOY = 1 << 0;
constexpr uint8_t FLAG_ENTRANCE = 1 << 1;
constexpr uint8_t FLAG_COMMANDER_START = 1 << 2;

struct MapData {
    unsigned int width;
    unsigned int height;
    unsigned int tileSize;
    std::string name;
    int timeLimitSeconds;
    int minionCap;

    std::vector<TileType> tiles;
    std::vector<uint8_t> flags;
    std::vector<sf::Vector2i> flagTiles;
    std::vector<sf::Vector2i> deployZone;
    sf::Vector2i commanderStart;
};

class MapLoader {
  public:
    static MapData load(const std::string &path);
};
