#include "QuadtreeCollisionSystem.h"

#include "../../Entities/Entity.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>

namespace {

float rectWidth(const sf::FloatRect& r) {
    return r.size.x;
}

float rectHeight(const sf::FloatRect& r) {
    return r.size.y;
}

struct Quadtree {
    sf::FloatRect bounds;
    int capacity;
    std::vector<Entity*> entities;
    std::unique_ptr<Quadtree> nw;
    std::unique_ptr<Quadtree> ne;
    std::unique_ptr<Quadtree> sw;
    std::unique_ptr<Quadtree> se;

    explicit Quadtree(const sf::FloatRect& b, int cap = 4) : bounds(b), capacity(cap) {}

    bool isLeaf() const { return !nw && !ne && !sw && !se; }

    bool contains(const sf::FloatRect& r) const {
        const float left = bounds.position.x;
        const float top = bounds.position.y;
        const float right = left + rectWidth(bounds);
        const float bottom = top + rectHeight(bounds);

        const float rLeft = r.position.x;
        const float rTop = r.position.y;
        const float rRight = rLeft + rectWidth(r);
        const float rBottom = rTop + rectHeight(r);

        return rLeft >= left && rTop >= top && rRight <= right && rBottom <= bottom;
    }

    void subdivide() {
        const float x = bounds.position.x;
        const float y = bounds.position.y;
        const float w = rectWidth(bounds) * 0.5f;
        const float h = rectHeight(bounds) * 0.5f;

        nw = std::make_unique<Quadtree>(sf::FloatRect({x, y}, {w, h}), capacity);
        ne = std::make_unique<Quadtree>(sf::FloatRect({x + w, y}, {w, h}), capacity);
        sw = std::make_unique<Quadtree>(sf::FloatRect({x, y + h}, {w, h}), capacity);
        se = std::make_unique<Quadtree>(sf::FloatRect({x + w, y + h}, {w, h}), capacity);
    }

    void insert(Entity* e) {
        const sf::FloatRect b = e->getBounds();
        if (!contains(b))
            return;

        if (isLeaf() && static_cast<int>(entities.size()) < capacity) {
            entities.push_back(e);
            return;
        }

        if (isLeaf()) {
            subdivide();
            for (Entity* existing : entities) {
                insertIntoChildren(existing);
            }
            entities.clear();
        }

        insertIntoChildren(e);
    }

    void insertIntoChildren(Entity* e) {
        const sf::FloatRect b = e->getBounds();
        if (nw->contains(b)) {
            nw->insert(e);
        } else if (ne->contains(b)) {
            ne->insert(e);
        } else if (sw->contains(b)) {
            sw->insert(e);
        } else if (se->contains(b)) {
            se->insert(e);
        } else {
            entities.push_back(e);
        }
    }

    void gatherPairs(std::vector<std::pair<Entity*, Entity*>>& out) const {
        if (!isLeaf()) {
            nw->gatherPairs(out);
            ne->gatherPairs(out);
            sw->gatherPairs(out);
            se->gatherPairs(out);
        }

        const std::size_t n = entities.size();
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                out.emplace_back(entities[i], entities[j]);
            }
        }
    }

    void draw(sf::RenderWindow& window) const {
        sf::RectangleShape rect;
        rect.setPosition(bounds.position);
        rect.setSize(bounds.size);
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(1.f);
        rect.setOutlineColor(sf::Color::Yellow);
        window.draw(rect);

        if (!isLeaf()) {
            nw->draw(window);
            ne->draw(window);
            sw->draw(window);
            se->draw(window);
        }
    }
};

std::unique_ptr<Quadtree> buildQuadtree(const TileMap& map, EntityManager& entities) {
    const float mapW = static_cast<float>(map.getWidth() * map.getTileSize());
    const float mapH = static_cast<float>(map.getHeight() * map.getTileSize());
    auto root = std::make_unique<Quadtree>(
        sf::FloatRect({0.f, 0.f}, sf::Vector2f(mapW, mapH)), 4);

    for (auto& uptr : entities.getEntities()) {
        Entity* e = uptr.get();
        if (!e || !e->isAlive())
            continue;
        root->insert(e);
    }
    return root;
}

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

sf::FloatRect intersectRects(const sf::FloatRect& a, const sf::FloatRect& b) {
    const float left = std::max(a.position.x, b.position.x);
    const float top = std::max(a.position.y, b.position.y);
    const float right =
        std::min(a.position.x + rectWidth(a), b.position.x + rectWidth(b));
    const float bottom =
        std::min(a.position.y + rectHeight(a), b.position.y + rectHeight(b));

    if (right <= left || bottom <= top)
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});

    return sf::FloatRect({left, top}, {right - left, bottom - top});
}

void resolveEntityOverlap(Entity& a, Entity& b) {
    sf::FloatRect boundsA = a.getBounds();
    sf::FloatRect boundsB = b.getBounds();
    const sf::FloatRect inter = intersectRects(boundsA, boundsB);
    if (rectWidth(inter) <= 0.f || rectHeight(inter) <= 0.f)
        return;

    const float pushX = rectWidth(inter);
    const float pushY = rectHeight(inter);

    auto posA = a.getPosition();
    auto posB = b.getPosition();

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
                                     {static_cast<float>(tileSize),
                                      static_cast<float>(tileSize)});

        const sf::FloatRect inter = intersectRects(bounds, tileRect);
        if (rectWidth(inter) <= 0.f || rectHeight(inter) <= 0.f)
            return;

        auto pos = entity.getPosition();
        const float pushX = rectWidth(inter);
        const float pushY = rectHeight(inter);

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

std::unique_ptr<Quadtree> g_lastTree;

}  // namespace

void QuadtreeCollisionSystem::update(EntityManager& entities, const TileMap& map) {
    auto tree = buildQuadtree(map, entities);

    std::vector<std::pair<Entity*, Entity*>> pairs;
    pairs.reserve(128);
    tree->gatherPairs(pairs);

    for (auto [a, b] : pairs) {
        if (!a || !b || !a->isAlive() || !b->isAlive())
            continue;
        resolveEntityOverlap(*a, *b);
    }

    for (auto& uptr : entities.getEntities()) {
        Entity* e = uptr.get();
        if (!e || !e->isAlive())
            continue;
        resolveEntityTileCollision(*e, map);
    }

    g_lastTree = std::move(tree);
}

void QuadtreeCollisionSystem::drawDebug(sf::RenderWindow& window) {
    if (!g_lastTree)
        return;
    g_lastTree->draw(window);
}

