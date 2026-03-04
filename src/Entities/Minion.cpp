#include "Minion.h"
#include "../Util/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>

namespace {
const float MINION_DISPLAY_SIZE = 24.f;
}

Minion::Minion(sf::Vector2f position)
    : Entity(position, sf::Vector2f(20.f, 20.f), sf::Color::Cyan) {
    if (!texture_.loadFromFile("assets/Characters/Minion/minion_up.png") &&
        !texture_.loadFromFile("../assets/Characters/Minion/minion_up.png")) {
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
    (void)dt;
    // No movement until pathfinding is wired
    updateRotationFromVelocity();
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
