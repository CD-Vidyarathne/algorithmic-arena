#include "Minion.h"
#include <SFML/Graphics/RenderWindow.hpp>

Minion::Minion(sf::Vector2f position)
    : Entity(position, sf::Vector2f(20.f, 20.f), sf::Color::Cyan) {
    shape_.setSize(getSize());
    shape_.setFillColor(getColor());
}

void Minion::update(float dt) {
    (void)dt;
    // No movement until pathfinding is wired
}

void Minion::render(sf::RenderWindow &window) {
    shape_.setPosition(getPosition());
    window.draw(shape_);
}
