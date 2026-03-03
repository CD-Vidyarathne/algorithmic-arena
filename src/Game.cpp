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
    auto data = MapLoader::load("../maps/benchmark_maze_512.map");

    tileMap_ = std::make_unique<TileMap>(data.width, data.height, data.tileSize);

    for (unsigned int y = 0; y < data.height; ++y) {
        for (unsigned int x = 0; x < data.width; ++x) {
            std::size_t i = y * data.width + x;
            tileMap_->setTile(x, y, data.tiles[i]);
            if (data.flags[i] & FLAG_DEPLOY)
                tileMap_->setFlag(x, y, FLAG_DEPLOY);
            if (data.flags[i] & FLAG_ENTRANCE)
                tileMap_->setFlag(x, y, FLAG_ENTRANCE);
            if (data.flags[i] & FLAG_COMMANDER_START)
                tileMap_->setFlag(x, y, FLAG_COMMANDER_START);
        }
    }
}
