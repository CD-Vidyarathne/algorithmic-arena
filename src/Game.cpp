#include "Game.h"
#include "Entities/Entity.h"
#include "Entities/Minion.h"
#include "Entities/PlayerCommander.h"
#include "Util/Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

Game::Game()
    : window_(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Algorithmic Arena") {
    Logger::get()->info("Game initialized");
    window_.setFramerateLimit(60);
    initializeTileMap();

    gameView_.setSize(sf::Vector2f(static_cast<float>(WINDOW_WIDTH),
                                   static_cast<float>(WINDOW_HEIGHT)));
    const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
    const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
    gameView_.setCenter(sf::Vector2f(mapW * 0.5f, mapH * 0.5f));
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

    minions_.erase(std::remove_if(minions_.begin(), minions_.end(),
                      [](Minion *m) { return !m->isAlive(); }),
                   minions_.end());

    if (commander_ && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        spawnCooldown_ -= dt;
        if (spawnCooldown_ <= 0.f) {
            spawnMinion();
            spawnCooldown_ = 0.2f;
        }
    } else {
        spawnCooldown_ = 0.f;
    }

    if (commander_ && tileMap_) {
        sf::Vector2f pos = commander_->getPosition();
        const float mapW =
            static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
        const float mapH =
            static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
        const sf::Vector2f size = commander_->getSize();

        const float maxX = std::max(0.f, mapW - size.x);
        const float maxY = std::max(0.f, mapH - size.y);

        pos.x = std::clamp(pos.x, 0.f, maxX);
        pos.y = std::clamp(pos.y, 0.f, maxY);
        commander_->setPosition(pos);
    }

    updateCamera();
}

void Game::updateCamera() {
    const float halfW = gameView_.getSize().x * 0.5f;
    const float halfH = gameView_.getSize().y * 0.5f;
    const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
    const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());

    sf::Vector2f centre;
    if (cameraTarget_) {
        centre = cameraTarget_->getPosition();
    } else {
        centre.x = mapW * 0.5f;
        centre.y = mapH * 0.5f;
    }
   
    const float minX = (mapW >= gameView_.getSize().x) ? halfW : mapW * 0.5f;
    const float maxX = (mapW >= gameView_.getSize().x) ? (mapW - halfW) : mapW * 0.5f;
    const float minY = (mapH >= gameView_.getSize().y) ? halfH : mapH * 0.5f;
    const float maxY = (mapH >= gameView_.getSize().y) ? (mapH - halfH) : mapH * 0.5f;
    centre.x = std::clamp(centre.x, minX, maxX);
    centre.y = std::clamp(centre.y, minY, maxY);
    gameView_.setCenter(centre);
}

void Game::render() {
    window_.clear(sf::Color::Black);

    window_.setView(gameView_);

    if (tileMap_) {
        tileMap_->draw(window_);
    }

    entityManager_.renderAll(window_);

    window_.setView(window_.getDefaultView());
    window_.display();
}

void Game::initializeTileMap() {
    auto data = MapLoader::load("../maps/benchmark_maze_512.map");

    maxMinions_ = data.minionCap > 0 ? data.minionCap : 100;
    deployZone_ = data.deployZone;

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

    sf::Vector2f commanderPos =
        tileMap_->tileCentre(sf::Vector2i(data.commanderStart.x, data.commanderStart.y));
    auto commander = std::make_unique<PlayerCommander>(commanderPos);
    commander_ = commander.get();
    setCameraTarget(commander_);
    entityManager_.addEntity(std::move(commander));
}

void Game::spawnMinion() {
    if (!commander_ || !tileMap_)
        return;
    if (static_cast<int>(minions_.size()) >= maxMinions_)
        return;

    const sf::Vector2i cmdTile = tileMap_->worldToTile(commander_->getPosition());
    if (cmdTile.x < 0 || cmdTile.y < 0 ||
        static_cast<unsigned int>(cmdTile.x) >= tileMap_->getWidth() ||
        static_cast<unsigned int>(cmdTile.y) >= tileMap_->getHeight())
        return;
    if (!tileMap_->hasFlag(static_cast<unsigned int>(cmdTile.x),
                          static_cast<unsigned int>(cmdTile.y), FLAG_DEPLOY))
        return;

    const sf::Vector2f spawnPos = tileMap_->tileCentre(cmdTile);
    auto minion = std::make_unique<Minion>(spawnPos);
    Minion *ptr = minion.get();
    minions_.push_back(ptr);
    entityManager_.addEntity(std::move(minion));
}
