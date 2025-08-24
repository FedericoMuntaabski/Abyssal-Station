#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONEVENTS_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONEVENTS_H

#include <functional>
#include <vector>
#include <unordered_map>

namespace entities { class Entity; }

namespace collisions {

// Custom hasher for entity pairs
struct EntityPairHash {
    std::size_t operator()(const std::pair<entities::Entity*, entities::Entity*>& pair) const {
        std::size_t h1 = std::hash<entities::Entity*>{}(pair.first);
        std::size_t h2 = std::hash<entities::Entity*>{}(pair.second);
        return h1 ^ (h2 << 1); // Simple hash combine
    }
};

// Collision event types
enum class CollisionEventType {
    OnEnter,
    OnExit,
    OnStay
};

// Collision event data
struct CollisionEvent {
    entities::Entity* entityA;
    entities::Entity* entityB;
    CollisionEventType type;
    float deltaTime;
};

// Collision event callback
using CollisionCallback = std::function<void(const CollisionEvent&)>;

// Collision event manager
class CollisionEventManager {
public:
    CollisionEventManager() = default;
    ~CollisionEventManager() = default;

    // Register callback for specific event type
    void registerCallback(CollisionEventType type, CollisionCallback callback);

    // Clear all callbacks for a specific event type
    void clearCallbacks(CollisionEventType type);

    // Clear all callbacks
    void clearAllCallbacks();

    // Fire an event
    void fireEvent(const CollisionEvent& event);

    // Update collision states and fire appropriate events
    void updateCollisionStates(entities::Entity* entityA, entities::Entity* entityB, bool isColliding, float deltaTime);

private:
    std::unordered_map<CollisionEventType, std::vector<CollisionCallback>> callbacks_;
    
    // Track ongoing collisions for enter/exit/stay events
    std::unordered_map<std::pair<entities::Entity*, entities::Entity*>, float, EntityPairHash> ongoingCollisions_;
    
    // Helper to create entity pair key (ensures consistent ordering)
    std::pair<entities::Entity*, entities::Entity*> makeEntityPair(entities::Entity* a, entities::Entity* b);
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONEVENTS_H
