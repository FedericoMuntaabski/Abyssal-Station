#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H

#include "CollisionManager.h"

namespace entities { class Entity; class Player; }

namespace collisions {

class CollisionSystem {
public:
    explicit CollisionSystem(CollisionManager& manager);

    // Resolve collisions for a single entity (applies position corrections if needed)
    void resolve(entities::Entity* entity, float deltaTime);

    // Resolve collisions for all registered colliders
    void resolveAll();

private:
    CollisionManager& manager_;
    // simple rate-limiter for collision logs
    float logTimer_{0.f};
    float logInterval_{0.25f};
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H
