#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H

#include "CollisionBox.h"
#include <vector>

namespace entities { class Entity; }

namespace collisions {

class CollisionManager {
public:
    CollisionManager() = default;
    ~CollisionManager() = default;

    // Register or update a collider for an entity
    void addCollider(entities::Entity* owner, const sf::FloatRect& bounds);

    // Remove collider for an entity
    void removeCollider(entities::Entity* owner);

    // Returns list of entities that are colliding with the given entity (excluding itself)
    std::vector<entities::Entity*> checkCollisions(entities::Entity* owner) const;

    // Return first entity that would collide with the provided bounds (exclude an optional owner)
    entities::Entity* firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude = nullptr) const;

private:
    std::vector<CollisionBox> colliders_;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H
