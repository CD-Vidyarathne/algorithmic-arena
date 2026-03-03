#pragma once

#include "Entity.h"
#include <SFML/Graphics/RectangleShape.hpp>

class PlayerCommander : public Entity {
  public:
    explicit PlayerCommander(sf::Vector2f position);

    void update(float dt) override;
    void render(sf::RenderWindow &window) override;

  private:
    sf::RectangleShape shape_;
    float speed_ = 200.f;
};

