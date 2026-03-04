#pragma once

#include "Entity.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <optional>

class Minion : public Entity {
  public:
    explicit Minion(sf::Vector2f position);

    void update(float dt) override;
    void render(sf::RenderWindow &window) override;

  private:
    void updateRotationFromVelocity();

    sf::Texture texture_;
    std::optional<sf::Sprite> sprite_;
    float lastAngleDeg_ = 0.f;
};
