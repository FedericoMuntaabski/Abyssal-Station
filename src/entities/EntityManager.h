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

    // Update and render all managed entities
    void updateAll(float deltaTime);
    void renderAll(sf::RenderWindow& window) const;

    // Lookup helpers
    Entity* getEntity(Entity::Id id) const;
    // Return raw pointers to all entities (non-owning)
    std::vector<Entity*> allEntities() const;
    std::size_t count() const noexcept;

    // Collision manager wiring: set the collision manager used to register colliders
    void setCollisionManager(collisions::CollisionManager* manager);

private:
    std::vector<std::unique_ptr<Entity>> entities_;
    collisions::CollisionManager* collisionManager_{nullptr};
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITYMANAGER_H
