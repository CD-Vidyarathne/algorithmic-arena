#include "PlayerCommander.h"
#include "../Util/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

namespace {
const float COMMANDER_DISPLAY_SIZE = 64.f;
}

PlayerCommander::PlayerCommander(sf::Vector2f position)
    : Entity(position, sf::Vector2f(28.f, 28.f), sf::Color::Blue) {
    if (!texture_.loadFromFile("../assets/Characters/Commander/com_up.png")) {
        Logger::get()->warn("PlayerCommander: could not load com_up.png, will not draw");
    } else {
        sprite_.emplace(texture_);
        const sf::Vector2u texSize = texture_.getSize();
        sprite_->setOrigin(sf::Vector2f(static_cast<float>(texSize.x) * 0.5f,
                                        static_cast<float>(texSize.y) * 0.5f));
        const float scaleX = COMMANDER_DISPLAY_SIZE / static_cast<float>(texSize.x);
        const float scaleY = COMMANDER_DISPLAY_SIZE / static_cast<float>(texSize.y);
        sprite_->setScale(sf::Vector2f(scaleX, scaleY));
    }
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
    updateRotationFromVelocity();
    const sf::Vector2f pos = getPosition();
    sf::Vector2f newPos(pos.x + velocity.x * dt, pos.y + velocity.y * dt);
    setPosition(newPos);
}

void PlayerCommander::updateRotationFromVelocity() {
    const sf::Vector2f v = getVelocity();
    if (v.x == 0.f && v.y == 0.f)
        return;
    const float rad = std::atan2(v.x, -v.y);
    lastAngleDeg_ = rad * 180.f / 3.14159265f;
    if (sprite_.has_value())
        sprite_->setRotation(sf::degrees(lastAngleDeg_));
}

void PlayerCommander::render(sf::RenderWindow &window) {
    if (!sprite_.has_value())
        return;
    sprite_->setPosition(getPosition());
    window.draw(*sprite_);
}
