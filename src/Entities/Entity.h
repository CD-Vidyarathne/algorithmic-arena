#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

class Entity {
  public:
    explicit Entity(sf::Vector2f position, sf::Vector2f size, sf::Color color)
        : position_(position), velocity_({0.f, 0.f}), size_(size), color_(color) {
    }

    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow &window) = 0;

    virtual sf::FloatRect getBounds() const {
        return sf::FloatRect(position_, size_);
    }

    sf::Vector2f getPosition() const {
        return position_;
    }
    sf::Vector2f getVelocity() const {
        return velocity_;
    }
    sf::Vector2f getSize() const {
        return size_;
    }

    void setPosition(sf::Vector2f position) {
        position_ = position;
    }
    void setVelocity(sf::Vector2f velocity) {
        velocity_ = velocity;
    }

  private:
    sf::Vector2f position_;
    sf::Vector2f velocity_;
    sf::Vector2f size_;
    sf::Color color_;
};
