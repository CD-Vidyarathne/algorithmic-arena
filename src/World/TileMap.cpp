#include "TileMap.h"
#include "../Util/Logger.h"

TileMap::TileMap(unsigned int width, unsigned int height, unsigned int tileSize)
    : width_(width), height_(height), tileSize_(tileSize),
      tiles_(width * height, TileType::Walkable),
      vertices_(sf::PrimitiveType::Triangles, width * height * 6) {
    updateVertices();
    Logger::get()->info("Tilemap Initialized");
};

void TileMap::setTile(unsigned int x, unsigned int y, TileType type) {
    if (x < width_ && y < height_) {
        tiles_[y * width_ + x] = type;
        updateVertices();
    }
}

TileType TileMap::getTile(unsigned int x, unsigned int y) const {
    if (x < width_ && y < height_) {
        return tiles_[y * width_ + x];
    }
    return TileType::Blocked;
}

bool TileMap::isWalkable(unsigned int x, unsigned int y) const {
    TileType type = getTile(x, y);
    return type != TileType::Blocked;
}

void TileMap::draw(sf::RenderWindow &window) {
    window.draw(vertices_);
}

void TileMap::updateVertices() {
    for (unsigned int y = 0; y < height_; ++y) {
        for (unsigned int x = 0; x < width_; ++x) {
            unsigned int tileIndex = y * width_ + x;
            unsigned int vertexIndex = tileIndex * 6;

            TileType type = tiles_[tileIndex];
            sf::Color color = getTileColor(type);

            float posX = x * tileSize_;
            float posY = y * tileSize_;
            sf::Vector2f topLeft(posX, posY);
            sf::Vector2f topRight(posX + tileSize_, posY);
            sf::Vector2f bottomRight(posX + tileSize_, posY + tileSize_);
            sf::Vector2f bottomLeft(posX, posY + tileSize_);

            vertices_[vertexIndex + 0].position = topLeft;
            vertices_[vertexIndex + 1].position = bottomLeft;
            vertices_[vertexIndex + 2].position = topRight;

            vertices_[vertexIndex + 3].position = bottomLeft;
            vertices_[vertexIndex + 4].position = bottomRight;
            vertices_[vertexIndex + 5].position = topRight;

            for (int i = 0; i < 6; ++i) {
                vertices_[vertexIndex + i].color = color;
            }
        }
    }
}

sf::Color TileMap::getTileColor(TileType type) const {
    switch (type) {
        case TileType::Walkable:
            return sf::Color(34, 139, 34);
        case TileType::Blocked:
            return sf::Color(64, 64, 64);
        default:
            return sf::Color::White;
    }
}
