#pragma once

#include "Entity.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <optional>
#include <vector>

class IPathfindingSystem;
class TileMap;

class Minion : public Entity {
  public:
    explicit Minion(sf::Vector2f position,
                    IPathfindingSystem *pathfindingSystem = nullptr,
                    const TileMap *tileMap = nullptr);

    void update(float dt) override;
    void render(sf::RenderWindow &window) override;

    void setTarget(const sf::Vector2i &targetTile);

    const std::optional<sf::Vector2i> &getGoalTile() const {
        return goalTile_;
    }

  private:
    void updateRotationFromVelocity();

    sf::Texture texture_;
    std::optional<sf::Sprite> sprite_;
    float lastAngleDeg_ = 0.f;

    IPathfindingSystem *pathfindingSystem_ = nullptr; // non-owning
    const TileMap *tileMap_ = nullptr;                // non-owning
    std::vector<sf::Vector2f> path_;
    std::size_t pathIndex_ = 0;
    std::optional<sf::Vector2i> goalTile_;
    float speed_ = 80.f;
};
