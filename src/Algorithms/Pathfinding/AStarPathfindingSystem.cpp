#include "AStarPathfindingSystem.h"

#include "../../Util/Logger.h"
#include "../../Util/PathfindingPerf.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <climits>
#include <unordered_map>
#include <unordered_set>

namespace {
struct Vec2iHash {
    std::size_t operator()(const sf::Vector2i &v) const noexcept {
        return (static_cast<std::size_t>(v.x) << 32) ^ static_cast<std::size_t>(v.y);
    }
};

bool inBounds(const TileMap &map, const sf::Vector2i &p) {
    return p.x >= 0 && p.y >= 0 && static_cast<unsigned int>(p.x) < map.getWidth() &&
           static_cast<unsigned int>(p.y) < map.getHeight();
}

std::vector<sf::Vector2i> reconstructPath(
    const sf::Vector2i &end,
    const std::unordered_map<sf::Vector2i, sf::Vector2i, Vec2iHash> &cameFrom) {
    std::vector<sf::Vector2i> path;
    sf::Vector2i current = end;
    auto it = cameFrom.find(current);
    if (it == cameFrom.end())
        return path;

    while (true) {
        path.push_back(current);
        auto itParent = cameFrom.find(current);
        if (itParent == cameFrom.end() || itParent->second == current)
            break;
        current = itParent->second;
    }
    std::reverse(path.begin(), path.end());
    return path;
}
} // namespace

void AStarPathfindingSystem::setRecordSearchVisualization(bool enabled) {
    recordSearchVisualization_ = enabled;
    if (!recordSearchVisualization_) {
        lastClosedSet_.clear();
        lastOpenSet_.clear();
        lastPath_.clear();
    }
}

int AStarPathfindingSystem::heuristic(const sf::Vector2i &a, const sf::Vector2i &b) const {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<sf::Vector2i>
AStarPathfindingSystem::findPath(sf::Vector2i start, sf::Vector2i end, const TileMap &map) {
    PathfindingPerf::FindPathScope perfScope;
    lastClosedSet_.clear();
    lastOpenSet_.clear();
    lastPath_.clear();

    if (!inBounds(map, start) || !inBounds(map, end)) {
        Logger::get()->warn("A*: start or end out of bounds");
        return {};
    }

    if (!map.isPassable(static_cast<unsigned int>(end.x),
                        static_cast<unsigned int>(end.y))) {
        Logger::get()->debug("A*: target tile not passable");
        return {};
    }

    std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> open;
    std::unordered_map<sf::Vector2i, int, Vec2iHash> gScore;
    std::unordered_map<sf::Vector2i, sf::Vector2i, Vec2iHash> cameFrom;
    std::unordered_set<sf::Vector2i, Vec2iHash> closed;

    PathNode startNode;
    startNode.pos = start;
    startNode.gCost = 0;
    startNode.fCost = heuristic(start, end);
    startNode.parent = start;
    open.push(startNode);
    gScore[start] = 0;
    cameFrom[start] = start;

    const sf::Vector2i neighbours[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!open.empty()) {
        PathNode current = open.top();
        open.pop();

        if (closed.contains(current.pos))
            continue;
        closed.insert(current.pos);
        if (recordSearchVisualization_) {
            lastClosedSet_.push_back(current.pos);
        }

        if (current.pos == end) {
            lastPath_ = reconstructPath(end, cameFrom);
            return lastPath_;
        }

        for (const sf::Vector2i &delta : neighbours) {
            sf::Vector2i next = current.pos + delta;
            if (!inBounds(map, next))
                continue;

            unsigned int nx = static_cast<unsigned int>(next.x);
            unsigned int ny = static_cast<unsigned int>(next.y);
            int tileCost = map.movementCost(nx, ny);
            if (tileCost == INT_MAX)
                continue;

            if (closed.contains(next))
                continue;

            int tentativeG = current.gCost + tileCost;
            auto itScore = gScore.find(next);
            if (itScore != gScore.end() && tentativeG >= itScore->second)
                continue;

            gScore[next] = tentativeG;
            cameFrom[next] = current.pos;

            PathNode neighbourNode;
            neighbourNode.pos = next;
            neighbourNode.gCost = tentativeG;
            neighbourNode.fCost = tentativeG + heuristic(next, end);
            neighbourNode.parent = current.pos;
            open.push(neighbourNode);
            if (recordSearchVisualization_) {
                lastOpenSet_.push_back(next);
            }
        }
    }

    Logger::get()->debug("A*: no path found");
    return {};
}

void AStarPathfindingSystem::drawDebug(sf::RenderWindow &window, const TileMap &map) {
    const float size = static_cast<float>(map.getTileSize());

    sf::RectangleShape cell(sf::Vector2f(size, size));
    cell.setFillColor(sf::Color(0, 0, 255, 80));

    for (const auto &p : lastClosedSet_) {
        cell.setPosition(map.tileToWorld(p));
        window.draw(cell);
    }

    cell.setFillColor(sf::Color(0, 255, 255, 80));
    for (const auto &p : lastOpenSet_) {
        cell.setPosition(map.tileToWorld(p));
        window.draw(cell);
    }

    if (!lastPath_.empty()) {
        cell.setFillColor(sf::Color(255, 255, 255, 120));
        for (const auto &p : lastPath_) {
            cell.setPosition(map.tileToWorld(p));
            window.draw(cell);
        }
    }
}

