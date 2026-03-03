#pragma once

#include "Entity.h"
#include <SFML/Graphics/RectangleShape.hpp>

class Minion : public Entity {
  public:
    explicit Minion(sf::Vector2f position);

    void update(float dt) override;
    void render(sf::RenderWindow &window) override;

  private:
    sf::RectangleShape shape_;
};
