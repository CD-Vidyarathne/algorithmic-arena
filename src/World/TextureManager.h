#pragma once

#include "TileMap.h"
#include <SFML/Graphics/Texture.hpp>
#include <array>
#include <string>

class TextureManager {
  public:
    TextureManager() = default;

    bool loadFromPath(const std::string &basePath);

    const sf::Texture &getTexture(TileType type) const;

    bool isLoaded() const {
        return loaded_;
    }

  private:
    std::array<sf::Texture, 6> textures_{};
    bool loaded_ = false;

    static std::size_t tileTypeToIndex(TileType type);
};
