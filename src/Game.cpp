#include "Game.h"
#include "Algorithms/Collision/BruteForceCollisionSystem.h"
#include "Algorithms/Collision/QuadtreeCollisionSystem.h"
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

#ifdef USE_QUADTREE_COLLISION
    collisionSystem_ = std::make_unique<QuadtreeCollisionSystem>();
    Logger::get()->info("Collision system: QuadtreeCollisionSystem (USE_QUADTREE_COLLISION)");
#else
    collisionSystem_ = std::make_unique<BruteForceCollisionSystem>();
    Logger::get()->info("Collision system: BruteForceCollisionSystem");
#endif

#ifdef USE_ASTAR_PATHFINDING
    Logger::get()->info("Pathfinding algorithm: A* (USE_ASTAR_PATHFINDING)");
#else
    Logger::get()->info("Pathfinding algorithm: Dijkstra");
#endif

    initializeTileMap();

    gameView_.setSize(
        sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
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
        if (const auto *mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (minimapVisible_ && mouse->button == sf::Mouse::Button::Left) {
                const sf::Vector2i clickPos = sf::Mouse::getPosition(window_);
                if (isMouseInMinimap(clickPos)) {
                    moveCameraToMinimapPosition(clickPos);
                }
            }
        }
        if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::T && commander_) {
                const sf::Vector2i pixel(sf::Mouse::getPosition(window_));
                const sf::Vector2f world = window_.mapPixelToCoords(pixel, gameView_);
                commander_->setPosition(world);
                setCameraTarget(commander_);
            }
            if (key->code == sf::Keyboard::Key::C && commander_) {
                setCameraTarget(commander_);
            }
            if (key->code == sf::Keyboard::Key::F) {
                moveCameraToNextFlag();
            }
            if (key->code == sf::Keyboard::Key::M) {
                minimapVisible_ = !minimapVisible_;
            }
            if (key->code == sf::Keyboard::Key::F1) {
                debugCollision_ = !debugCollision_;
            }
        }
    }
}

void Game::update(float dt) {
    entityManager_.updateAll(dt);
    if (collisionSystem_ && tileMap_) {
        collisionSystem_->update(entityManager_, *tileMap_);
    }
    entityManager_.removeDeadEntities();

    minions_.erase(
        std::remove_if(minions_.begin(), minions_.end(), [](Minion *m) { return !m->isAlive(); }),
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
        const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
        const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
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
        centre = gameView_.getCenter();
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

    entityManager_.renderAllExcept(window_, commander_);
    if (commander_)
        commander_->render(window_);

    if (debugCollision_ && collisionSystem_) {
        collisionSystem_->drawDebug(window_);
    }

    window_.setView(window_.getDefaultView());
    if (minimapVisible_)
        renderMinimap();
    window_.display();
}

void Game::renderMinimap() {
    if (!tileMap_)
        return;
    const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
    const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
    const sf::Vector2u winSize = window_.getSize();

    minimapPixelBounds_.position.x = minimapViewport_.position.x * static_cast<float>(winSize.x);
    minimapPixelBounds_.position.y = minimapViewport_.position.y * static_cast<float>(winSize.y);
    minimapPixelBounds_.size.x = minimapViewport_.size.x * static_cast<float>(winSize.x);
    minimapPixelBounds_.size.y = minimapViewport_.size.y * static_cast<float>(winSize.y);

    sf::View minimapView;
    minimapView.setViewport(minimapViewport_);
    minimapView.setSize(sf::Vector2f(mapW, mapH));
    minimapView.setCenter(sf::Vector2f(mapW * 0.5f, mapH * 0.5f));

    window_.setView(minimapView);
    tileMap_->draw(window_);

    const float borderThickness = 40.f;
    sf::RectangleShape viewRect(gameView_.getSize());
    viewRect.setPosition(gameView_.getCenter() - gameView_.getSize() * 0.5f);
    viewRect.setFillColor(sf::Color(255, 255, 255, 35));
    viewRect.setOutlineColor(sf::Color(255, 255, 255));
    viewRect.setOutlineThickness(borderThickness);
    window_.draw(viewRect);

    window_.setView(window_.getDefaultView());
}

bool Game::isMouseInMinimap(sf::Vector2i pixel) const {
    return minimapPixelBounds_.contains(
        sf::Vector2f(static_cast<float>(pixel.x), static_cast<float>(pixel.y)));
}

void Game::moveCameraToMinimapPosition(sf::Vector2i pixel) {
    if (!tileMap_ || minimapPixelBounds_.size.x <= 0.f || minimapPixelBounds_.size.y <= 0.f)
        return;
    const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
    const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());

    const float nx =
        (static_cast<float>(pixel.x) - minimapPixelBounds_.position.x) / minimapPixelBounds_.size.x;
    const float ny =
        (static_cast<float>(pixel.y) - minimapPixelBounds_.position.y) / minimapPixelBounds_.size.y;
    const float worldX = nx * mapW;
    const float worldY = ny * mapH;

    const float halfW = gameView_.getSize().x * 0.5f;
    const float halfH = gameView_.getSize().y * 0.5f;
    const float minX = halfW;
    const float maxX = std::max(minX, mapW - halfW);
    const float minY = halfH;
    const float maxY = std::max(minY, mapH - halfH);
    gameView_.setCenter(
        sf::Vector2f(std::clamp(worldX, minX, maxX), std::clamp(worldY, minY, maxY)));
    setCameraTarget(nullptr);
}

void Game::moveCameraToNextFlag() {
    if (!tileMap_ || flagTilePositions_.empty())
        return;
    const sf::Vector2i tile = flagTilePositions_[flagCycleIndex_];
    flagCycleIndex_ = (flagCycleIndex_ + 1) % flagTilePositions_.size();
    const sf::Vector2f worldCentre = tileMap_->tileCentre(tile);

    const float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
    const float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
    const float halfW = gameView_.getSize().x * 0.5f;
    const float halfH = gameView_.getSize().y * 0.5f;
    const float minX = halfW;
    const float maxX = std::max(minX, mapW - halfW);
    const float minY = halfH;
    const float maxY = std::max(minY, mapH - halfH);
    gameView_.setCenter(
        sf::Vector2f(std::clamp(worldCentre.x, minX, maxX), std::clamp(worldCentre.y, minY, maxY)));
    setCameraTarget(nullptr);
}

void Game::initializeTileMap() {
    auto data = MapLoader::load("../maps/nexus_siege_512.map");

    maxMinions_ = data.minionCap > 0 ? data.minionCap : 100;
    deployZone_ = data.deployZone;
    flagTilePositions_ = data.flagTiles;

    textureManager_ = std::make_unique<TextureManager>();
    if (!textureManager_->loadFromPath("../assets")) {
        Logger::get()->warn("TextureManager: no tile textures loaded; using colored tiles");
        textureManager_.reset();
    }
    tileMap_ =
        std::make_unique<TileMap>(data.width, data.height, data.tileSize, textureManager_.get());

    for (unsigned int y = 0; y < data.height; ++y) {
        for (unsigned int x = 0; x < data.width; ++x) {
            std::size_t i = y * data.width + x;
            tileMap_->setTile(x, y, data.tiles[i]);
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
    if (tileMap_->getTile(static_cast<unsigned int>(cmdTile.x),
                          static_cast<unsigned int>(cmdTile.y)) != TileType::Deploy)
        return;

    const sf::Vector2f spawnPos = tileMap_->tileCentre(cmdTile);
    auto minion = std::make_unique<Minion>(spawnPos);
    Minion *ptr = minion.get();
    minions_.push_back(ptr);
    entityManager_.addEntity(std::move(minion));
}
