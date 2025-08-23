#ifndef ABYSSAL_STATION_SRC_ENTITIES_ENTITYMANAGER_H
#define ABYSSAL_STATION_SRC_ENTITIES_ENTITYMANAGER_H

#include "Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>
#include <memory>
#include <cstddef>

namespace collisions { class CollisionManager; }

namespace entities {

class EntityManager {
public:
    EntityManager() = default;
    ~EntityManager() = default;

    // Non-copyable
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    // Add an entity to the manager. Takes ownership via unique_ptr.
    void addEntity(std::unique_ptr<Entity> entity);

    // Remove an entity by id. Returns true if removed.
    bool removeEntity(Entity::Id id);

    // Remove multiple entities by ids. Returns number of removed entities.
    std::size_t removeEntities(const std::vector<Entity::Id>& ids);

    // Update and render all managed entities
    void updateAll(float deltaTime);
    void renderAll(sf::RenderWindow& window) const;

    // Batch operations for performance
    void updateActiveEntities(float deltaTime); // Only updates entities that need updating
    void renderVisibleEntities(sf::RenderWindow& window, const sf::FloatRect& viewBounds) const;

    // Lookup helpers
    Entity* getEntity(Entity::Id id) const;
    // Return raw pointers to all entities (non-owning)
    std::vector<Entity*> allEntities() const;
    // Get entities by collision layer (for optimization)
    std::vector<Entity*> getEntitiesByLayer(Entity::Layer layer) const;
    // Get entities by type (using dynamic_cast)
    template<typename T>
    std::vector<T*> getEntitiesOfType() const;
    
    std::size_t count() const noexcept;
    std::size_t activeCount() const noexcept; // Count of entities that are not flagged as inactive

    // Collision manager wiring: set the collision manager used to register colliders
    void setCollisionManager(collisions::CollisionManager* manager);

    // Entity lifecycle management
    void markEntityForRemoval(Entity::Id id); // Mark for removal in next cleanup
    void cleanupMarkedEntities(); // Remove all marked entities
    void clearAllEntities(); // Remove all entities

    // Performance monitoring
    struct PerformanceStats {
        std::size_t totalEntities{0};
        std::size_t entitiesUpdated{0};
        std::size_t entitiesRendered{0};
        float lastUpdateTime{0.f};
        float lastRenderTime{0.f};
    };
    const PerformanceStats& getPerformanceStats() const { return performanceStats_; }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
    collisions::CollisionManager* collisionManager_{nullptr};
    std::vector<Entity::Id> markedForRemoval_;
    PerformanceStats performanceStats_;

    // Helper to find entity iterator by id
    auto findEntityById(Entity::Id id) const;
};

// Template implementation
template<typename T>
std::vector<T*> EntityManager::getEntitiesOfType() const {
    std::vector<T*> result;
    for (const auto& entity : entities_) {
        if (entity) {
            if (auto* typedEntity = dynamic_cast<T*>(entity.get())) {
                result.push_back(typedEntity);
            }
        }
    }
    return result;
}

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITYMANAGER_H
