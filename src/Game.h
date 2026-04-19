#pragma once

#include "Entities/EntityManager.h"
#include "World/MapLoader.h"
#include "World/TextureManager.h"
#include "World/TileMap.h"
#include "Algorithms/Collision/ICollisionSystem.h"
#include "Algorithms/Pathfinding/IPathfindingSystem.h"
#include "Util/CsvLogger.h"
#include "Util/PathfindBudget.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Entity;
class Minion;
class PlayerCommander;

enum class GameState { Ready, Playing, Won, Lost };

struct GameOptions {
    std::optional<std::string> benchmarkCsvPath;
    bool uncappedFps = false;
    std::string mapPath = "../maps/nexus_siege_128.map";
};

class Game {
  public:
    explicit Game(GameOptions options = {});
    void run();

    void setCameraTarget(Entity* entity) { cameraTarget_ = entity; }


    const sf::View& getGameView() const { return gameView_; }

  private:
    void processEvents();
    void update(float dt);
    void render();
    void updateCamera(float dt);
    void renderMinimap();
    void renderFlagCaptureOverlays();
    bool isMouseInMinimap(sf::Vector2i pixel) const;
    void moveCameraToMinimapPosition(sf::Vector2i pixel);
    void moveCameraToNextFlag();
    void renderHud();
    void initializeHud();
    void startMatch();
    void updateGameplay(float dt);
    void retargetMinionsFromCapturedFlagGoals();
    void issueOrderToTile(const sf::Vector2i& targetTile);
    void issueOrderToSelectionOrAll(const sf::Vector2i& targetTile);
    void orderMinionsToNearestUncapturedFlag();
    sf::Vector2i nearestUncapturedFlagForTile(const sf::Vector2i& from) const;
    void refreshMinionList();
    void handleWorldLeftClick(sf::Vector2i pixel);
    void beginSelectionDrag(sf::Vector2i pixel);
    void updateSelectionDrag(sf::Vector2i pixel);
    void finalizeSelectionDrag(bool addMode);
    void clearSelection();
    void selectAllMinions();
    void renderSelectionMarkers();
    void renderSelectionBox();
    void renderPathDebugOverlay();

    void initializeTileMap();
    void spawnMinion();
    void spawnMinionAtWorld(const sf::Vector2f &worldPos);
    void spawnMinionsInDeployZone(int count);

    static constexpr std::array<int, 12> kBulkSpawnPresets{10,   25,   50,   100,  250,  500,
                                                             1000, 2500, 5000, 10000, 25000, 50000};
    std::size_t bulkSpawnPresetIndex_ = 6;

    int bulkSpawnAmount() const {
        return kBulkSpawnPresets[bulkSpawnPresetIndex_ % kBulkSpawnPresets.size()];
    }

    static constexpr int kPathfindsPerFrameBudget = 64;

    sf::RenderWindow window_;
    std::unique_ptr<TextureManager> textureManager_;
    std::unique_ptr<TileMap> tileMap_;
    sf::Clock clock_;
    EntityManager entityManager_;

    sf::View gameView_;
    sf::FloatRect minimapViewport_{sf::Vector2f(0.75f, 0.75f), sf::Vector2f(0.25f, 0.25f)};  // normalized 0-1
    sf::FloatRect minimapPixelBounds_;  // updated each frame in renderMinimap for exact hit test
    bool minimapVisible_ = false;  // default off, toggle with M
    std::vector<sf::Vector2i> flagTilePositions_;
    std::unordered_map<std::uint64_t, std::size_t> flagTileToIndex_;
    size_t flagCycleIndex_ = 0;

    Entity* cameraTarget_ = nullptr;
    PlayerCommander* commander_ = nullptr;

    int maxMinions_ = 100;
    std::vector<sf::Vector2i> deployZone_;
    std::vector<Minion*> minions_;
    std::vector<Minion*> selectedMinions_;
    float spawnCooldown_ = 0.f;
    float spawnCooldownDuration_ = 0.28f;
    float minionSelectRadius_ = 26.f;
    float cameraFollowLerp_ = 10.f;
    bool edgePanEnabled_ = true;
    float edgePanSpeed_ = 1300.f;
    int edgePanMarginPx_ = 24;
    float cameraViewScale_ = 1.f;
    bool selectionDragging_ = false;
    sf::Vector2f selectionDragStart_{0.f, 0.f};
    sf::Vector2f selectionDragCurrent_{0.f, 0.f};

    GameState gameState_ = GameState::Ready;
    float gameTimer_ = 0.f;
    bool hasSpawnedMinion_ = false;
    float score_ = 0.f;
    int capturedFlags_ = 0;
    int totalFlags_ = 0;
    std::vector<uint8_t> flagCaptured_;
    float scoreTickAccumulator_ = 0.f;
    float initialSpawnGraceSeconds_ = 5.f;
    float captureRatePerMinion_ = 0.55f;

    std::unique_ptr<ICollisionSystem> collisionSystem_;
    std::unique_ptr<IPathfindingSystem> pathfindingSystem_;
    PathfindBudget pathfindBudget_;
    bool debugCollision_ = false;
    bool debugPathfinding_ = false;
    bool showPerfOverlay_ = true;
    float smoothedFps_ = 0.f;
    float lastCollisionMs_ = 0.f;

    sf::Font hudFont_;
    bool hudFontLoaded_ = false;
    bool showKeymapHud_ = true;

    std::string mapPath_;
    std::optional<CsvLogger> csvLogger_;
    float csvLogAccumulator_ = 0.f;
    double benchCollisionUsAccum_ = 0.;
    std::int64_t benchPathNanoAccum_ = 0;
    int benchPathCallsAccum_ = 0;
};
