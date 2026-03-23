#include "Minion.h"

#include "../Algorithms/Pathfinding/IPathfindingSystem.h"
#include "../World/TileMap.h"
#include "../Util/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>

namespace {
const float MINION_DISPLAY_SIZE = 24.f;
const float ARRIVAL_THRESHOLD = 4.f;
}

Minion::Minion(sf::Vector2f position, IPathfindingSystem *pathfindingSystem, const TileMap *tileMap)
    : Entity(position, sf::Vector2f(20.f, 20.f), sf::Color::Cyan),
      pathfindingSystem_(pathfindingSystem),
      tileMap_(tileMap) {
    if (!texture_.loadFromFile("../assets/Characters/Minion/minion_up.png")) {
        Logger::get()->warn("Minion: could not load minion_up.png");
    } else {
        sprite_.emplace(texture_);
        const sf::Vector2u texSize = texture_.getSize();
        sprite_->setOrigin(sf::Vector2f(static_cast<float>(texSize.x) * 0.5f,
                                        static_cast<float>(texSize.y) * 0.5f));
        const float scaleX = MINION_DISPLAY_SIZE / static_cast<float>(texSize.x);
        const float scaleY = MINION_DISPLAY_SIZE / static_cast<float>(texSize.y);
        sprite_->setScale(sf::Vector2f(scaleX, scaleY));
    }
}

void Minion::update(float dt) {
    if (!path_.empty() && tileMap_) {
        if (pathIndex_ >= path_.size())
            pathIndex_ = path_.size() - 1;

        // Skip waypoints already reached (first path node is often current tile center).
        while (!path_.empty()) {
            const sf::Vector2f target = path_[pathIndex_];
            sf::Vector2f toTarget = target - getPosition();
            const float lenSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            const float len = std::sqrt(lenSq);

            if (len <= ARRIVAL_THRESHOLD) {
                if (pathIndex_ + 1 < path_.size()) {
                    ++pathIndex_;
                    continue;
                }
                path_.clear();
                setVelocity(sf::Vector2f(0.f, 0.f));
                break;
            }

            sf::Vector2f dir = toTarget / len;
            setVelocity(dir * speed_);
            setPosition(getPosition() + getVelocity() * dt);
            break;
        }
    }

    updateRotationFromVelocity();
}

void Minion::setTarget(const sf::Vector2i &targetTile) {
    if (!pathfindingSystem_ || !tileMap_) {
        Logger::get()->warn("Minion::setTarget called without pathfinding system or tileMap");
        return;
    }

    const sf::Vector2i startTile = tileMap_->worldToTile(getPosition());
    auto tiles = pathfindingSystem_->findPath(startTile, targetTile, *tileMap_);
    path_.clear();
    pathIndex_ = 0;
    for (const auto &t : tiles) {
        path_.push_back(tileMap_->tileCentre(t));
    }
}

void Minion::updateRotationFromVelocity() {
    const sf::Vector2f v = getVelocity();
    if (v.x == 0.f && v.y == 0.f)
        return;
    const float rad = std::atan2(v.x, -v.y);
    lastAngleDeg_ = rad * 180.f / 3.14159265f;
    if (sprite_.has_value())
        sprite_->setRotation(sf::degrees(lastAngleDeg_));
}

void Minion::render(sf::RenderWindow &window) {
    if (!sprite_.has_value())
        return;
    sprite_->setPosition(getPosition());
    window.draw(*sprite_);
}
