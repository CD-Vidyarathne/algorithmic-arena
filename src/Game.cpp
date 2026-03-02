#include "Game.h"
#include "Util/Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Game::Game()
    : window_(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Algorithmic Arena") {
    Logger::get()->info("Game initialized");
    window_.setFramerateLimit(60);
    initializeTileMap();
}

void Game::run() {
    while (window_.isOpen()) {
        float dt = clock_.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    while (const auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            Logger::get()->warn("Window closing event detected");
            window_.close();
        }
    }
}

void Game::update(float dt) {
    entityManager_.updateAll(dt);
    entityManager_.removeDeadEntities();
}

void Game::render() {
    window_.clear(sf::Color::Black);

    if (tileMap_) {
        tileMap_->draw(window_);
    }

    entityManager_.renderAll(window_);
    window_.display();
}

void Game::initializeTileMap() {
    tileMap_ = std::make_unique<TileMap>(40, 22, 32);

    for (unsigned int x = 0; x < tileMap_->getWidth(); ++x) {
        tileMap_->setTile(x, 0, TileType::Blocked);
        tileMap_->setTile(x, tileMap_->getHeight() - 1, TileType::Blocked);
    }

    for (unsigned int y = 0; y < tileMap_->getHeight(); ++y) {
        tileMap_->setTile(0, y, TileType::Blocked);
        tileMap_->setTile(tileMap_->getWidth() - 1, y, TileType::Blocked);
    }

    for (unsigned int x = 10; x < 15; ++x) {
        tileMap_->setTile(x, 10, TileType::Blocked);
    }
}
