#include "EntityManager.h"
#include <algorithm>
#include "../core/Logger.h"
#include "../collisions/CollisionManager.h"

using collisions::CollisionManager;

namespace entities {

void EntityManager::addEntity(std::unique_ptr<Entity> entity) {
    if (!entity) return;
    core::Logger::instance().info("[EntityManager] Adding entity id=" + std::to_string(entity->id()));
    // Keep pointer to newly added entity so we can register collider after moving into container
    Entity* raw = entity.get();
    entities_.push_back(std::move(entity));

    // Register collider if collision manager is set
    if (collisionManager_ && raw) {
        sf::FloatRect bounds;
        bounds.position.x = raw->position().x;
        bounds.position.y = raw->position().y;
        bounds.size.x = raw->size().x;
        bounds.size.y = raw->size().y;
        collisionManager_->addCollider(raw, bounds);
    }
}

bool EntityManager::removeEntity(Entity::Id id) {
    auto it = std::find_if(entities_.begin(), entities_.end(), [id](const std::unique_ptr<Entity>& e) {
        return e && e->id() == id;
    });

    if (it == entities_.end()) return false;
    core::Logger::instance().info("[EntityManager] Removing entity id=" + std::to_string((*it)->id()));
    // If collision manager is set, remove collider for this entity
    if (collisionManager_) collisionManager_->removeCollider(it->get());
    entities_.erase(it);
    return true;
}

void EntityManager::updateAll(float deltaTime) {
    for (const auto& e : entities_) {
        if (e) {
            e->update(deltaTime);
            // If collision manager present, update the collider bounds to match entity position/size
            if (collisionManager_) {
                sf::FloatRect bounds;
                bounds.position.x = e->position().x;
                bounds.position.y = e->position().y;
                bounds.size.x = e->size().x;
                bounds.size.y = e->size().y;
                collisionManager_->addCollider(e.get(), bounds);
            }
        }
    }
}

void EntityManager::renderAll(sf::RenderWindow& window) const {
    for (const auto& e : entities_) {
        if (e) e->render(window);
    }
}

Entity* EntityManager::getEntity(Entity::Id id) const {
    auto it = std::find_if(entities_.begin(), entities_.end(), [id](const std::unique_ptr<Entity>& e) {
        return e && e->id() == id;
    });
    return (it != entities_.end()) ? it->get() : nullptr;
}

std::size_t EntityManager::count() const noexcept {
    return entities_.size();
}

void EntityManager::setCollisionManager(CollisionManager* manager) {
    collisionManager_ = manager;
}

} // namespace entities
