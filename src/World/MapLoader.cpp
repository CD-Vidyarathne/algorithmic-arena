#include "MapLoader.h"
#include "../Util/Logger.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

bool startsWith(const std::string &s, const std::string &prefix) {
    return s.rfind(prefix, 0) == 0;
}

std::string trim(const std::string &s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

} // namespace

MapData MapLoader::load(const std::string &path) {
    MapData data{};

    std::ifstream in(path);
    if (!in) {
        Logger::get()->error("Failed to open map file: {}", path);
        throw std::runtime_error("MapLoader: failed to open file");
    }

    Logger::get()->info("Loading map: {}", path);

    std::string line;
    std::vector<std::string> gridLines;
    gridLines.reserve(512);

    unsigned int headerWidth = 0;
    unsigned int headerHeight = 0;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            const std::string content = trim(line.substr(1));
            if (startsWith(content, "name=")) {
                data.name = content.substr(std::string("name=").size());
            } else if (startsWith(content, "width=")) {
                headerWidth = static_cast<unsigned int>(std::stoul(content.substr(6)));
            } else if (startsWith(content, "height=")) {
                headerHeight = static_cast<unsigned int>(std::stoul(content.substr(7)));
            } else if (startsWith(content, "tile_size=")) {
                data.tileSize = static_cast<unsigned int>(std::stoul(content.substr(10)));
            } else if (startsWith(content, "time_limit=")) {
                data.timeLimitSeconds = std::stoi(content.substr(11));
            } else if (startsWith(content, "minion_cap=")) {
                data.minionCap = std::stoi(content.substr(11));
            }
            continue;
        }

        gridLines.push_back(line);
    }

    if (gridLines.empty()) {
        Logger::get()->error("MapLoader: no grid data in file {}", path);
        throw std::runtime_error("MapLoader: empty grid");
    }

    const unsigned int actualHeight = static_cast<unsigned int>(gridLines.size());
    const unsigned int actualWidth = static_cast<unsigned int>(gridLines.front().size());

    for (const auto &row : gridLines) {
        if (row.size() != actualWidth) {
            Logger::get()->error("MapLoader: inconsistent row width in {}", path);
            throw std::runtime_error("MapLoader: inconsistent row width");
        }
    }

    if (headerWidth == 0 || headerHeight == 0) {
        headerWidth = actualWidth;
        headerHeight = actualHeight;
    } else if (headerWidth != actualWidth || headerHeight != actualHeight) {
        Logger::get()->warn(
            "MapLoader: header width/height ({}x{}) does not match grid ({}x{}) in {}", headerWidth,
            headerHeight, actualWidth, actualHeight, path);
    }

    data.width = actualWidth;
    data.height = actualHeight;

    const std::size_t tileCount = static_cast<std::size_t>(data.width) * data.height;
    data.tiles.resize(tileCount);
    data.flags.resize(tileCount, 0u);
    data.flagTiles.clear();
    data.deployZone.clear();
    data.commanderStart = sf::Vector2i(-1, -1);

    int commanderCount = 0;

    for (unsigned int y = 0; y < data.height; ++y) {
        const std::string &row = gridLines[y];
        for (unsigned int x = 0; x < data.width; ++x) {
            const std::size_t index = static_cast<std::size_t>(y) * data.width + x;
            const char ch = row[x];

            TileType tile = TileType::Grass;
            uint8_t flags = 0u;

            switch (ch) {
                case 'G':
                    tile = TileType::Grass;
                    break;
                case 'M':
                    tile = TileType::Mud;
                    break;
                case 'T':
                    tile = TileType::Tree;
                    break;
                case 'L':
                    tile = TileType::Lava;
                    break;
                case 'F':
                    tile = TileType::Flag;
                    data.flagTiles.emplace_back(static_cast<int>(x), static_cast<int>(y));
                    break;
                case 'D':
                    tile = TileType::Deploy;
                    data.deployZone.emplace_back(static_cast<int>(x), static_cast<int>(y));
                    break;
                case 'E':
                    tile = TileType::Grass;
                    flags |= FLAG_ENTRANCE;
                    break;
                case 'C':
                    tile = TileType::Grass;
                    flags |= FLAG_COMMANDER_START;
                    data.commanderStart = sf::Vector2i(static_cast<int>(x), static_cast<int>(y));
                    ++commanderCount;
                    break;
                default:
                    tile = TileType::Grass;
                    Logger::get()->warn("MapLoader: unknown symbol '{}' at ({}, {}) in {}", ch, x,
                                        y, path);
                    break;
            }

            data.tiles[index] = tile;
            data.flags[index] = flags;
        }
    }

    if (commanderCount != 1) {
        Logger::get()->error("MapLoader: expected exactly one commander start (C), found {} in {}",
                             commanderCount, path);
    }
    if (data.flagTiles.empty()) {
        Logger::get()->error("MapLoader: no Flag tiles (F) found in {}", path);
    }

    Logger::get()->info("Map loaded: {} ({}x{}, tileSize={}, flags={}, F tiles={})", data.name,
                        data.width, data.height, data.tileSize, tileCount, data.flagTiles.size());

    return data;
}
