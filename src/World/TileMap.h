#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class TileType { Walkable, Blocked };

class TileMap {
  public:
    TileMap(unsigned int width, unsigned int height, unsigned int tileSize);

    void setTile(unsigned int x, unsigned int y, TileType type);
    TileType getTile(unsigned int x, unsigned int y) const;
    bool isWalkable(unsigned int x, unsigned int y) const;

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

  private:
    unsigned int width_;
    unsigned int height_;
    unsigned int tileSize_;
    std::vector<TileType> tiles_;
    sf::VertexArray vertices_;

    void updateVertices();
    sf::Color getTileColor(TileType type) const;
};
