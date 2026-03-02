#pragma once

#include "Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>
#include <memory>
#include <vector>

class EntityManager {
  public:
    void addEntity(std::unique_ptr<Entity> entity) {
        entities_.push_back(std::move(entity));
    }

    void removeDeadEntities() {
        entities_.erase(std::remove_if(entities_.begin(), entities_.end(),
                                       [](const auto &e) { return !e->isAlive(); }),
                        entities_.end());
    }

    void updateAll(float dt) {
        for (auto &entity : entities_) {
            entity->update(dt);
        }
    }

    void renderAll(sf::RenderWindow &window) {
        for (auto &entity : entities_) {
            entity->render(window);
        }
    }

    const std::vector<std::unique_ptr<Entity>> &getEntities() const {
        return entities_;
    }

  private:
    std::vector<std::unique_ptr<Entity>> entities_;
};
