#include "EntityManager.h"
#include <algorithm>
#include <chrono>
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

std::size_t EntityManager::removeEntities(const std::vector<Entity::Id>& ids) {
    std::size_t removedCount = 0;
    for (Entity::Id id : ids) {
        if (removeEntity(id)) {
            ++removedCount;
        }
    }
    return removedCount;
}

void EntityManager::updateAll(float deltaTime) {
    auto start = std::chrono::high_resolution_clock::now();
    std::size_t updated = 0;
    
    for (const auto& e : entities_) {
        if (e) {
            e->update(deltaTime);
            ++updated;
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
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    performanceStats_.totalEntities = entities_.size();
    performanceStats_.entitiesUpdated = updated;
    performanceStats_.lastUpdateTime = duration.count() / 1000.0f; // Convert to milliseconds
}

void EntityManager::updateActiveEntities(float deltaTime) {
    // For now, same as updateAll - can be optimized later with entity states
    updateAll(deltaTime);
}

void EntityManager::renderAll(sf::RenderWindow& window) const {
    auto start = std::chrono::high_resolution_clock::now();
    std::size_t rendered = 0;
    
    for (const auto& e : entities_) {
        if (e) {
            e->render(window);
            ++rendered;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    const_cast<EntityManager*>(this)->performanceStats_.entitiesRendered = rendered;
    const_cast<EntityManager*>(this)->performanceStats_.lastRenderTime = duration.count() / 1000.0f;
}

void EntityManager::renderVisibleEntities(sf::RenderWindow& window, const sf::FloatRect& viewBounds) const {
    // Simple frustum culling - only render entities within view bounds
    auto start = std::chrono::high_resolution_clock::now();
    std::size_t rendered = 0;
    
    for (const auto& e : entities_) {
        if (e) {
            sf::FloatRect entityBounds;
            entityBounds.position = e->position();
            entityBounds.size = e->size();
            
            if (viewBounds.findIntersection(entityBounds)) {
                e->render(window);
                ++rendered;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    const_cast<EntityManager*>(this)->performanceStats_.entitiesRendered = rendered;
    const_cast<EntityManager*>(this)->performanceStats_.lastRenderTime = duration.count() / 1000.0f;
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

std::size_t EntityManager::activeCount() const noexcept {
    // For now, count all entities - can be enhanced with entity state tracking
    return entities_.size();
}

std::vector<Entity*> EntityManager::getEntitiesByLayer(Entity::Layer layer) const {
    std::vector<Entity*> result;
    std::uint32_t layerMask = static_cast<std::uint32_t>(layer);
    
    for (const auto& entity : entities_) {
        if (entity && entity->collisionLayer() == layerMask) {
            result.push_back(entity.get());
        }
    }
    
    return result;
}

void EntityManager::markEntityForRemoval(Entity::Id id) {
    markedForRemoval_.push_back(id);
    core::Logger::instance().info("[EntityManager] Marked entity id=" + std::to_string(id) + " for removal");
}

void EntityManager::cleanupMarkedEntities() {
    if (markedForRemoval_.empty()) return;
    
    std::size_t removed = removeEntities(markedForRemoval_);
    core::Logger::instance().info("[EntityManager] Cleaned up " + std::to_string(removed) + " marked entities");
    markedForRemoval_.clear();
}

void EntityManager::clearAllEntities() {
    std::size_t count = entities_.size();
    entities_.clear();
    markedForRemoval_.clear();
    core::Logger::instance().info("[EntityManager] Cleared all " + std::to_string(count) + " entities");
}

void EntityManager::setCollisionManager(CollisionManager* manager) {
    collisionManager_ = manager;
}

std::vector<Entity*> EntityManager::allEntities() const
{
    std::vector<Entity*> out;
    out.reserve(entities_.size());
    for (const auto& up : entities_) if (up) out.push_back(up.get());
    return out;
}

auto EntityManager::findEntityById(Entity::Id id) const {
    return std::find_if(entities_.begin(), entities_.end(), [id](const std::unique_ptr<Entity>& e) {
        return e && e->id() == id;
    });
}

} // namespace entities
