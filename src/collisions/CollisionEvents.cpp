#include "CollisionEvents.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"
#include <string>

namespace collisions {

void CollisionEventManager::registerCallback(CollisionEventType type, CollisionCallback callback) {
    callbacks_[type].push_back(std::move(callback));
}

void CollisionEventManager::clearCallbacks(CollisionEventType type) {
    callbacks_[type].clear();
}

void CollisionEventManager::clearAllCallbacks() {
    callbacks_.clear();
}

void CollisionEventManager::fireEvent(const CollisionEvent& event) {
    auto it = callbacks_.find(event.type);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

void CollisionEventManager::updateCollisionStates(entities::Entity* entityA, entities::Entity* entityB, bool isColliding, float deltaTime) {
    if (!entityA || !entityB) return;

    auto entityPair = makeEntityPair(entityA, entityB);
    auto it = ongoingCollisions_.find(entityPair);
    bool wasColliding = (it != ongoingCollisions_.end());

    if (isColliding) {
        if (!wasColliding) {
            // Collision started - fire OnEnter event
            CollisionEvent enterEvent{entityA, entityB, CollisionEventType::OnEnter, deltaTime};
            fireEvent(enterEvent);
            
            // Mark as ongoing collision
            ongoingCollisions_[entityPair] = 0.0f;
            
            core::Logger::instance().info("[CollisionEventManager] OnEnter: " + 
                std::to_string(entityA->id()) + " <-> " + std::to_string(entityB->id()));
        } else {
            // Collision continuing - fire OnStay event and update timer
            it->second += deltaTime;
            CollisionEvent stayEvent{entityA, entityB, CollisionEventType::OnStay, deltaTime};
            fireEvent(stayEvent);
        }
    } else {
        if (wasColliding) {
            // Collision ended - fire OnExit event
            CollisionEvent exitEvent{entityA, entityB, CollisionEventType::OnExit, deltaTime};
            fireEvent(exitEvent);
            
            // Remove from ongoing collisions
            ongoingCollisions_.erase(it);
            
            core::Logger::instance().info("[CollisionEventManager] OnExit: " + 
                std::to_string(entityA->id()) + " <-> " + std::to_string(entityB->id()));
        }
    }
}

std::pair<entities::Entity*, entities::Entity*> CollisionEventManager::makeEntityPair(entities::Entity* a, entities::Entity* b) {
    // Ensure consistent ordering to avoid duplicate pairs (a,b) and (b,a)
    if (a->id() < b->id()) {
        return {a, b};
    } else {
        return {b, a};
    }
}

} // namespace collisions
