#include "PlayerCommander.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

PlayerCommander::PlayerCommander(sf::Vector2f position)
    : Entity(position, sf::Vector2f(28.f, 28.f), sf::Color::Blue) {
    shape_.setSize(getSize());
    shape_.setFillColor(getColor());
}

void PlayerCommander::update(float dt) {
    sf::Vector2f direction(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        direction.y -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        direction.y += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        direction.x -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        direction.x += 1.f;
    }

    sf::Vector2f velocity(0.f, 0.f);
    if (direction.x != 0.f || direction.y != 0.f) {
        const float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0.f) {
            direction.x /= len;
            direction.y /= len;
        }
        velocity.x = direction.x * speed_;
        velocity.y = direction.y * speed_;
    }

    setVelocity(velocity);
    const sf::Vector2f pos = getPosition();
    sf::Vector2f newPos(pos.x + velocity.x * dt, pos.y + velocity.y * dt);
    setPosition(newPos);
}

void PlayerCommander::render(sf::RenderWindow &window) {
    shape_.setPosition(getPosition());
    window.draw(shape_);
}

