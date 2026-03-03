#include "TileMap.h"
#include "../Util/Logger.h"
#include <algorithm>

TileMap::TileMap(unsigned int width, unsigned int height, unsigned int tileSize)
    : width_(width), height_(height), tileSize_(tileSize), tiles_(width * height, TileType::Grass),
      flags_(width * height, 0u), captureProgress_(width * height, 0.0f),
      vertices_(sf::PrimitiveType::Triangles, width * height * 6) {
    updateVertices();
    Logger::get()->info("Tilemap Initialized ({}x{}, tileSize={})", width_, height_, tileSize_);
};

void TileMap::setTile(unsigned int x, unsigned int y, TileType type) {
    if (x >= width_ || y >= height_) {
        return;
    }

    const std::size_t index = static_cast<std::size_t>(y) * width_ + x;
    tiles_[index] = type;

    captureProgress_[index] = 0.0f;

    updateTileVertices(x, y);
}

TileType TileMap::getTile(unsigned int x, unsigned int y) const {
    if (x < width_ && y < height_) {
        return tiles_[static_cast<std::size_t>(y) * width_ + x];
    }
    return TileType::Tree;
}

bool TileMap::isPassable(unsigned int x, unsigned int y) const {
    auto t = getTile(x, y);
    return t != TileType::Tree && t != TileType::Lava;
}

int TileMap::movementCost(unsigned int x, unsigned int y) const {
    switch (getTile(x, y)) {
        case TileType::Grass:
        case TileType::Flag:
            return 10;
        case TileType::Mud:
            return 17;
        case TileType::Tree:
        case TileType::Lava:
        default:
            return std::numeric_limits<int>::max();
    }
}

void TileMap::draw(sf::RenderWindow &window) {
    const sf::View &view = window.getView();
    const sf::Vector2f &center = view.getCenter();
    const sf::Vector2f &size = view.getSize();
    const float left = center.x - size.x * 0.5f;
    const float top = center.y - size.y * 0.5f;

    int xMin = static_cast<int>(left / static_cast<float>(tileSize_));
    int yMin = static_cast<int>(top / static_cast<float>(tileSize_));
    int xMax = static_cast<int>((left + size.x) / static_cast<float>(tileSize_)) + 1;
    int yMax = static_cast<int>((top + size.y) / static_cast<float>(tileSize_)) + 1;

    xMin = std::max(0, xMin);
    yMin = std::max(0, yMin);
    xMax = std::min(static_cast<int>(width_), xMax);
    yMax = std::min(static_cast<int>(height_), yMax);

    if (xMin >= xMax || yMin >= yMax) {
        return;
    }

    const unsigned int visW = static_cast<unsigned int>(xMax - xMin);
    const unsigned int visH = static_cast<unsigned int>(yMax - yMin);
    sf::VertexArray visible(sf::PrimitiveType::Triangles, visW * visH * 6);
    std::size_t idx = 0;
    for (unsigned int ty = static_cast<unsigned int>(yMin); ty < static_cast<unsigned int>(yMax); ++ty) {
        for (unsigned int tx = static_cast<unsigned int>(xMin); tx < static_cast<unsigned int>(xMax); ++tx) {
            const unsigned int vertexIndex = (ty * width_ + tx) * 6;
            for (int i = 0; i < 6; ++i) {
                visible[idx++] = vertices_[vertexIndex + i];
            }
        }
    }
    window.draw(visible);
}

bool TileMap::hasFlag(unsigned int x, unsigned int y, uint8_t bit) const {
    if (x >= width_ || y >= height_) {
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(y) * width_ + x;
    return (flags_[index] & bit) != 0;
}

void TileMap::setFlag(unsigned int x, unsigned int y, uint8_t bit) {
    if (x >= width_ || y >= height_) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(y) * width_ + x;
    flags_[index] |= bit;
}

float TileMap::getCaptureProgress(unsigned int x, unsigned int y) const {
    if (x >= width_ || y >= height_) {
        return 0.0f;
    }
    const std::size_t index = static_cast<std::size_t>(y) * width_ + x;
    return captureProgress_[index];
}

void TileMap::advanceCapture(unsigned int x, unsigned int y, float dt, float rate) {
    if (x >= width_ || y >= height_) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(y) * width_ + x;
    if (tiles_[index] != TileType::Flag) {
        return;
    }

    float value = captureProgress_[index];
    value += dt * rate;
    value = std::clamp(value, 0.0f, 1.0f);
    captureProgress_[index] = value;
}

bool TileMap::isCaptured(unsigned int x, unsigned int y) const {
    return getCaptureProgress(x, y) >= 1.0f;
}

void TileMap::updateVertices() {
    for (unsigned int y = 0; y < height_; ++y) {
        for (unsigned int x = 0; x < width_; ++x) {
            updateTileVertices(x, y);
        }
    }
}

void TileMap::updateTileVertices(unsigned int x, unsigned int y) {
    unsigned int tileIndex = y * width_ + x;
    unsigned int vertexIndex = tileIndex * 6;
    sf::Color color = getTileColor(tiles_[tileIndex]);
    float posX = static_cast<float>(x * tileSize_);
    float posY = static_cast<float>(y * tileSize_);
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

sf::Color TileMap::getTileColor(TileType type) const {
    switch (type) {
        case TileType::Grass:
            return sf::Color(34, 139, 34);
        case TileType::Mud:
            return sf::Color(139, 90, 43);
        case TileType::Tree:
            return sf::Color(34, 80, 34);
        case TileType::Lava:
            return sf::Color(200, 60, 10);
        case TileType::Flag:
            return sf::Color(200, 200, 50);
    }
    return sf::Color::White;
}
