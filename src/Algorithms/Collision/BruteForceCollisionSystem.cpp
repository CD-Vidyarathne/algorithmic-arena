#include "BruteForceCollisionSystem.h"

#include "../../Entities/Entity.h"

namespace {

float rectWidth(const sf::FloatRect& r) {
    return r.size.x;
}

float rectHeight(const sf::FloatRect& r) {
    return r.size.y;
}

void resolveOverlapAlongSmallestAxis(Entity& a, Entity& b, const sf::FloatRect& intersection) {
    const float pushX = rectWidth(intersection);
    const float pushY = rectHeight(intersection);

    auto posA = a.getPosition();
    auto posB = b.getPosition();

    const sf::FloatRect boundsA = a.getBounds();
    const sf::FloatRect boundsB = b.getBounds();

    if (pushX < pushY) {
        if (boundsA.position.x < boundsB.position.x) {
            posA.x -= pushX * 0.5f;
            posB.x += pushX * 0.5f;
        } else {
            posA.x += pushX * 0.5f;
            posB.x -= pushX * 0.5f;
        }
    } else {
        if (boundsA.position.y < boundsB.position.y) {
            posA.y -= pushY * 0.5f;
            posB.y += pushY * 0.5f;
        } else {
            posA.y += pushY * 0.5f;
            posB.y -= pushY * 0.5f;
        }
    }

    a.setPosition(posA);
    b.setPosition(posB);
}

void resolveEntityTileCollision(Entity& entity, const TileMap& map) {
    sf::FloatRect bounds = entity.getBounds();

    const float left = bounds.position.x;
    const float top = bounds.position.y;
    const float right = left + rectWidth(bounds);
    const float bottom = top + rectHeight(bounds);

    const sf::Vector2i topLeftTile = map.worldToTile({left, top});
    const sf::Vector2i topRightTile = map.worldToTile({right, top});
    const sf::Vector2i bottomLeftTile = map.worldToTile({left, bottom});
    const sf::Vector2i bottomRightTile = map.worldToTile({right, bottom});

    const unsigned int mapW = map.getWidth();
    const unsigned int mapH = map.getHeight();
    const unsigned int tileSize = map.getTileSize();

    auto handleTile = [&](const sf::Vector2i& tile) {
        if (tile.x < 0 || tile.y < 0)
            return;
        const unsigned int tx = static_cast<unsigned int>(tile.x);
        const unsigned int ty = static_cast<unsigned int>(tile.y);
        if (tx >= mapW || ty >= mapH)
            return;

        if (map.isPassable(tx, ty))
            return;

        const float tileLeft = static_cast<float>(tx * tileSize);
        const float tileTop = static_cast<float>(ty * tileSize);
        const sf::FloatRect tileRect({tileLeft, tileTop},
                                     {static_cast<float>(tileSize), static_cast<float>(tileSize)});

        const float interLeft = std::max(bounds.position.x, tileRect.position.x);
        const float interTop = std::max(bounds.position.y, tileRect.position.y);
        const float interRight =
            std::min(bounds.position.x + rectWidth(bounds),
                     tileRect.position.x + rectWidth(tileRect));
        const float interBottom =
            std::min(bounds.position.y + rectHeight(bounds),
                     tileRect.position.y + rectHeight(tileRect));

        if (interRight <= interLeft || interBottom <= interTop)
            return;

        const sf::FloatRect intersection({interLeft, interTop},
                                         {interRight - interLeft, interBottom - interTop});

        auto pos = entity.getPosition();
        const float pushX = rectWidth(intersection);
        const float pushY = rectHeight(intersection);

        if (pushX < pushY) {
            if (bounds.position.x < tileRect.position.x) {
                pos.x -= pushX;
            } else {
                pos.x += pushX;
            }
        } else {
            if (bounds.position.y < tileRect.position.y) {
                pos.y -= pushY;
            } else {
                pos.y += pushY;
            }
        }

        entity.setPosition(pos);
        bounds = entity.getBounds();
    };

    handleTile(topLeftTile);
    handleTile(topRightTile);
    handleTile(bottomLeftTile);
    handleTile(bottomRightTile);
}

}  // namespace

void BruteForceCollisionSystem::update(EntityManager& entities, const TileMap& map) {
    auto& list = entities.getEntities();

    const std::size_t count = list.size();
    for (std::size_t i = 0; i < count; ++i) {
        Entity* a = list[i].get();
        if (!a || !a->isAlive())
            continue;
        for (std::size_t j = i + 1; j < count; ++j) {
            Entity* b = list[j].get();
            if (!b || !b->isAlive())
                continue;

            sf::FloatRect boundsA = a->getBounds();
            sf::FloatRect boundsB = b->getBounds();

            const float interLeft = std::max(boundsA.position.x, boundsB.position.x);
            const float interTop = std::max(boundsA.position.y, boundsB.position.y);
            const float interRight =
                std::min(boundsA.position.x + rectWidth(boundsA),
                         boundsB.position.x + rectWidth(boundsB));
            const float interBottom =
                std::min(boundsA.position.y + rectHeight(boundsA),
                         boundsB.position.y + rectHeight(boundsB));

            if (interRight <= interLeft || interBottom <= interTop)
                continue;

            const sf::FloatRect intersection({interLeft, interTop},
                                             {interRight - interLeft, interBottom - interTop});
            resolveOverlapAlongSmallestAxis(*a, *b, intersection);
        }
    }

    for (auto& ptr : list) {
        if (!ptr || !ptr->isAlive())
            continue;
        resolveEntityTileCollision(*ptr, map);
    }
}

void BruteForceCollisionSystem::drawDebug(sf::RenderWindow& window) {
    // This simple implementation leaves debug drawing to the Game for now.
    (void)window;
}

