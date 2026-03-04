#include "TextureManager.h"
#include "../Util/Logger.h"

std::size_t TextureManager::tileTypeToIndex(TileType type) {
    switch (type) {
        case TileType::Grass:
            return 0;
        case TileType::Mud:
            return 1;
        case TileType::Tree:
            return 2;
        case TileType::Lava:
            return 3;
        case TileType::Flag:
            return 4;
    }
    return 0;
}

bool TextureManager::loadFromPath(const std::string &basePath) {
    const std::string dir = basePath + "/Tiles";
    struct Entry {
        const char *filename;
        TileType type;
    };
    const Entry entries[] = {
        {"grass.png", TileType::Grass},
        {"mud.png", TileType::Mud},
        {"tree.png", TileType::Tree},
        {"lava.png", TileType::Lava},
        {"flag.png", TileType::Flag},
    };

    loaded_ = true;
    for (const auto &e : entries) {
        const std::string path = dir + "/" + e.filename;
        const std::size_t idx = tileTypeToIndex(e.type);
        if (!textures_[idx].loadFromFile(path)) {
            Logger::get()->warn("TextureManager: failed to load {}", path);
            loaded_ = false;
        }
    }
    if (loaded_)
        Logger::get()->info("TextureManager: loaded tile set from {}", dir);
    return loaded_;
}

const sf::Texture &TextureManager::getTexture(TileType type) const {
    return textures_[tileTypeToIndex(type)];
}
