#pragma once

#include "Entity.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <optional>

class PlayerCommander : public Entity {
  public:
    explicit PlayerCommander(sf::Vector2f position);

    void update(float dt) override;
    void render(sf::RenderWindow &window) override;

    bool ignoresMapCollision() const override {
        return true;
    }

    EntityKind kind() const override {
        return EntityKind::Commander;
    }

  private:
    void updateRotationFromVelocity();

    sf::Texture texture_;
    std::optional<sf::Sprite> sprite_;
    float speed_ = 200.f;
    float lastAngleDeg_ = 0.f;
};

