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

    // Register or update a collider for an entity (uses owner's collisionLayer())
    void addCollider(entities::Entity* owner, const sf::FloatRect& bounds);

    // Remove collider for an entity
    void removeCollider(entities::Entity* owner);

    // Returns list of entities that are colliding with the given entity (excluding itself)
    std::vector<entities::Entity*> checkCollisions(entities::Entity* owner) const;

    // Return first entity that would collide with the provided bounds (exclude an optional owner)
    // If allowedLayers != 0, only colliders whose layer bit intersects allowedLayers are considered
    entities::Entity* firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Returns true if a line segment from p0 to p1 intersects any collider (optionally excluding an entity)
    // and restricted to allowedLayers. Useful to implement line-of-sight checks that should be blocked by walls.
    bool segmentIntersectsAny(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

private:
    std::vector<CollisionBox> colliders_;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H
