#include "Game.h"
#include "Algorithms/Collision/BruteForceCollisionSystem.h"
#include "Algorithms/Collision/QuadtreeCollisionSystem.h"
#include "Algorithms/Pathfinding/AStarPathfindingSystem.h"
#include "Algorithms/Pathfinding/DijkstrasPathfindingSystem.h"
#include "Entities/Entity.h"
#include "Entities/Minion.h"
#include "Entities/PlayerCommander.h"
#include "Util/Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>

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
    pathfindingSystem_ = std::make_unique<AStarPathfindingSystem>();
    Logger::get()->info("Pathfinding algorithm: A* (USE_ASTAR_PATHFINDING)");
#else
    pathfindingSystem_ = std::make_unique<DijkstrasPathfindingSystem>();
    Logger::get()->info("Pathfinding algorithm: Dijkstra");
#endif

    initializeTileMap();
    initializeHud();
    gameState_ = GameState::Ready;

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
            if (mouse->button == sf::Mouse::Button::Right && gameState_ == GameState::Playing) {
                const sf::Vector2i pixel(sf::Mouse::getPosition(window_));
                const sf::Vector2f world = window_.mapPixelToCoords(pixel, gameView_);
                issueOrderToTile(tileMap_->worldToTile(world));
            }
        }
        if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Enter && gameState_ == GameState::Ready) {
                startMatch();
            }
            if (key->code == sf::Keyboard::Key::R &&
                (gameState_ == GameState::Won || gameState_ == GameState::Lost)) {
                gameState_ = GameState::Ready;
                startMatch();
            }
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
            if (key->code == sf::Keyboard::Key::O && gameState_ == GameState::Playing) {
                orderMinionsToNearestUncapturedFlag();
            }
            if (key->code == sf::Keyboard::Key::M) {
                minimapVisible_ = !minimapVisible_;
            }
            if (key->code == sf::Keyboard::Key::F1) {
                debugCollision_ = !debugCollision_;
            }
            if (key->code == sf::Keyboard::Key::F2) {
                debugPathfinding_ = !debugPathfinding_;
            }
        }
    }
}

void Game::update(float dt) {
    if (gameState_ == GameState::Ready) {
        updateCamera();
        return;
    }

    if (gameState_ != GameState::Playing) {
        updateCamera();
        return;
    }

    entityManager_.updateAll(dt);
    if (collisionSystem_ && tileMap_) {
        collisionSystem_->update(entityManager_, *tileMap_);
    }
    entityManager_.removeDeadEntities();

    refreshMinionList();

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

    updateGameplay(dt);
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
    if (debugPathfinding_ && pathfindingSystem_ && tileMap_) {
        pathfindingSystem_->drawDebug(window_, *tileMap_);
    }

    window_.setView(window_.getDefaultView());
    renderHud();
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
    timeLimitSeconds_ = data.timeLimitSeconds > 0 ? static_cast<float>(data.timeLimitSeconds) : 180.f;
    deployZone_ = data.deployZone;
    flagTilePositions_ = data.flagTiles;
    totalFlags_ = static_cast<int>(flagTilePositions_.size());
    flagCaptured_.assign(flagTilePositions_.size(), 0u);

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

    minions_.clear();
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
    auto minion = std::make_unique<Minion>(spawnPos, pathfindingSystem_.get(), tileMap_.get());
    Minion *ptr = minion.get();
    minions_.push_back(ptr);
    hasSpawnedMinion_ = true;
    entityManager_.addEntity(std::move(minion));
}

void Game::initializeHud() {
    static const std::vector<std::string> candidates = {
        "../assets/Fonts/DejaVuSans.ttf",
        "../assets/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"};

    for (const auto &path : candidates) {
        if (hudFont_.openFromFile(path)) {
            hudFontLoaded_ = true;
            Logger::get()->info("HUD font loaded from {}", path);
            return;
        }
    }
    Logger::get()->warn("HUD font could not be loaded; HUD text disabled");
}

void Game::startMatch() {
    entityManager_.clear();
    minions_.clear();
    commander_ = nullptr;
    cameraTarget_ = nullptr;
    hasSpawnedMinion_ = false;
    gameTimer_ = 0.f;
    score_ = 0.f;
    capturedFlags_ = 0;
    scoreTickAccumulator_ = 0.f;
    spawnCooldown_ = 0.f;
    initializeTileMap();
    gameState_ = GameState::Playing;
}

void Game::issueOrderToTile(const sf::Vector2i &targetTile) {
    if (!tileMap_ || minions_.empty())
        return;
    if (targetTile.x < 0 || targetTile.y < 0 ||
        static_cast<unsigned int>(targetTile.x) >= tileMap_->getWidth() ||
        static_cast<unsigned int>(targetTile.y) >= tileMap_->getHeight()) {
        return;
    }
    if (!tileMap_->isPassable(static_cast<unsigned int>(targetTile.x),
                              static_cast<unsigned int>(targetTile.y))) {
        return;
    }
    for (Minion *m : minions_) {
        if (m && m->isAlive())
            m->setTarget(targetTile);
    }
}

sf::Vector2i Game::nearestUncapturedFlagForTile(const sf::Vector2i &from) const {
    sf::Vector2i best(-1, -1);
    float bestDistSq = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < flagTilePositions_.size(); ++i) {
        if (flagCaptured_[i] != 0u)
            continue;
        const sf::Vector2i t = flagTilePositions_[i];
        const float dx = static_cast<float>(t.x - from.x);
        const float dy = static_cast<float>(t.y - from.y);
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestDistSq) {
            bestDistSq = d2;
            best = t;
        }
    }
    return best;
}

void Game::orderMinionsToNearestUncapturedFlag() {
    if (!tileMap_)
        return;
    for (Minion *m : minions_) {
        if (!m || !m->isAlive())
            continue;
        const sf::Vector2i from = tileMap_->worldToTile(m->getPosition());
        const sf::Vector2i target = nearestUncapturedFlagForTile(from);
        if (target.x >= 0)
            m->setTarget(target);
    }
}

void Game::refreshMinionList() {
    minions_.erase(
        std::remove_if(minions_.begin(), minions_.end(), [](Minion *m) { return !m || !m->isAlive(); }),
        minions_.end());
}

void Game::updateGameplay(float dt) {
    if (!tileMap_)
        return;

    gameTimer_ += dt;
    scoreTickAccumulator_ += dt;
    if (scoreTickAccumulator_ >= 1.f) {
        score_ += 1.f;
        scoreTickAccumulator_ = 0.f;
    }

    std::map<std::size_t, int> minionsOnFlag;
    for (Minion *m : minions_) {
        if (!m || !m->isAlive())
            continue;
        const sf::Vector2i tile = tileMap_->worldToTile(m->getPosition());
        for (std::size_t i = 0; i < flagTilePositions_.size(); ++i) {
            if (flagTilePositions_[i] == tile) {
                minionsOnFlag[i] += 1;
                break;
            }
        }
    }

    for (const auto &[idx, count] : minionsOnFlag) {
        const sf::Vector2i t = flagTilePositions_[idx];
        tileMap_->advanceCapture(static_cast<unsigned int>(t.x), static_cast<unsigned int>(t.y), dt,
                                 captureRatePerMinion_ * static_cast<float>(count));
    }

    for (std::size_t i = 0; i < flagTilePositions_.size(); ++i) {
        if (flagCaptured_[i] != 0u)
            continue;
        const sf::Vector2i t = flagTilePositions_[i];
        if (tileMap_->isCaptured(static_cast<unsigned int>(t.x), static_cast<unsigned int>(t.y))) {
            flagCaptured_[i] = 1u;
            capturedFlags_ += 1;
            score_ += 100.f;
        }
    }

    if (capturedFlags_ >= totalFlags_ && totalFlags_ > 0) {
        gameState_ = GameState::Won;
        return;
    }
    if (gameTimer_ >= timeLimitSeconds_) {
        gameState_ = GameState::Lost;
        return;
    }
    if (hasSpawnedMinion_ && gameTimer_ > initialSpawnGraceSeconds_ && minions_.empty()) {
        gameState_ = GameState::Lost;
    }
}

void Game::renderHud() {
    sf::RectangleShape bg(sf::Vector2f(460.f, 150.f));
    bg.setPosition(sf::Vector2f(10.f, 10.f));
    bg.setFillColor(sf::Color(0, 0, 0, 150));
    window_.draw(bg);

    if (!hudFontLoaded_)
        return;

    std::string stateText = "READY";
    if (gameState_ == GameState::Playing)
        stateText = "PLAYING";
    else if (gameState_ == GameState::Won)
        stateText = "YOU WIN";
    else if (gameState_ == GameState::Lost)
        stateText = "GAME OVER";

    const float remaining = std::max(0.f, timeLimitSeconds_ - gameTimer_);
    std::string lines = "State: " + stateText + "\nScore: " + std::to_string(static_cast<int>(score_)) +
                        "\nFlags: " + std::to_string(capturedFlags_) + "/" +
                        std::to_string(totalFlags_) + "\nTime Left: " +
                        std::to_string(static_cast<int>(remaining)) + "s\n";
    if (gameState_ == GameState::Ready) {
        lines += "Enter: Start  |  Space: Spawn  |  Right Click: Order";
    } else if (gameState_ == GameState::Playing) {
        lines += "Space: Spawn  |  Right Click: Order  |  O: Auto-order to flags";
    } else {
        lines += "R: Restart";
    }

    sf::Text text(hudFont_, lines, 18);
    text.setPosition(sf::Vector2f(20.f, 18.f));
    text.setFillColor(sf::Color::White);
    window_.draw(text);
}
