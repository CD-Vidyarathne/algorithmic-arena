#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class TileType { Grass, Mud, Tree, Lava, Flag };

class TextureManager;

class TileMap {
  public:
    TileMap(unsigned int width, unsigned int height, unsigned int tileSize,
            const TextureManager *textureManager = nullptr);

    void setTile(unsigned int x, unsigned int y, TileType type);
    TileType getTile(unsigned int x, unsigned int y) const;
    bool isPassable(unsigned int x, unsigned int y) const;
    int movementCost(unsigned int x, unsigned int y) const;
    void updateTileVertices(unsigned int x, unsigned int y);

    void draw(sf::RenderWindow &window);

    unsigned int getWidth() const {
        return width_;
    }
    unsigned int getHeight() const {
        return height_;
    }
    unsigned int getTileSize() const {
        return tileSize_;
    }

    bool hasFlag(unsigned int x, unsigned int y, uint8_t bit) const;
    void setFlag(unsigned int x, unsigned int y, uint8_t bit);

    float getCaptureProgress(unsigned int x, unsigned int y) const;
    void advanceCapture(unsigned int x, unsigned int y, float dt, float rate);
    bool isCaptured(unsigned int x, unsigned int y) const;

    sf::Vector2i worldToTile(sf::Vector2f worldPos) const {
        return {static_cast<int>(worldPos.x / tileSize_), static_cast<int>(worldPos.y / tileSize_)};
    }
    sf::Vector2f tileToWorld(sf::Vector2i tilePos) const {
        return {static_cast<float>(tilePos.x * tileSize_),
                static_cast<float>(tilePos.y * tileSize_)};
    }
    sf::Vector2f tileCentre(sf::Vector2i tilePos) const {
        float half = tileSize_ / 2.f;
        return {static_cast<float>(tilePos.x * tileSize_) + half,
                static_cast<float>(tilePos.y * tileSize_) + half};
    }

  private:
    unsigned int width_;
    unsigned int height_;
    unsigned int tileSize_;
    std::vector<TileType> tiles_;
    std::vector<uint8_t> flags_;
    std::vector<float> captureProgress_;
    sf::VertexArray vertices_;
    const TextureManager *textureManager_ = nullptr;

    void updateVertices();
    sf::Color getTileColor(TileType type) const;
};
