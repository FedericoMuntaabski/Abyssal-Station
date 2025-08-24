#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H

#include "CollisionManager.h"
#include "CollisionEvents.h"
#include <vector>
#include <unordered_set>

namespace entities { class Entity; class Player; }

namespace collisions {

struct CollisionResolution {
    sf::Vector2f correction{0.f, 0.f}; // Position correction vector
    sf::Vector2f normalizedNormal{0.f, 0.f}; // Collision normal
    float penetrationDepth{0.f}; // How deep the penetration is
    bool wasResolved{false}; // Whether resolution was applied
};

class CollisionSystem {
public:
    struct Config {
        float maxCorrectionDistance = 50.f; // Maximum distance to correct in one frame
        bool enableContinuousDetection = true; // Use sweep tests for fast-moving objects
        bool enableEvents = true; // Fire collision events
        bool logResolutions = true; // Log collision resolutions
    };

    explicit CollisionSystem(CollisionManager& manager, const Config& config = Config{});

    // Configuration
    void setConfig(const Config& config) { config_ = config; }
    const Config& getConfig() const { return config_; }

    // Resolve collisions for a single entity (applies position corrections if needed)
    CollisionResolution resolve(entities::Entity* entity, float deltaTime);

    // Resolve collisions for multiple entities simultaneously
    void resolveMultiple(const std::vector<entities::Entity*>& entities, float deltaTime);

    // Resolve collisions for all registered colliders
    void resolveAll(float deltaTime);

    // Enable/disable collision events for specific entity pairs
    void enableCollisionEvents(entities::Entity* entityA, entities::Entity* entityB, bool enabled = true);
    void disableCollisionEvents(entities::Entity* entityA, entities::Entity* entityB);

    // Physics-like resolution with momentum conservation (basic implementation)
    void resolveWithPhysics(entities::Entity* entityA, entities::Entity* entityB, 
                           const CollisionResult& collision, float deltaTime);

    // Sweep test for continuous collision detection
    bool sweepTest(entities::Entity* entity, const sf::Vector2f& velocity, float deltaTime, 
                  CollisionResult& firstHit);

    // Statistics and debugging
    struct Stats {
        int totalResolutions = 0;
        int eventsTriggered = 0;
        float totalCorrectionDistance = 0.f;
        int continuousDetectionTests = 0;
    };
    
    const Stats& getStats() const { return stats_; }
    void resetStats();

private:
    CollisionManager& manager_;
    Config config_;
    
    // Rate-limiter for collision logs
    float logTimer_{0.f};
    float logInterval_{0.25f};
    
    // Track enabled event pairs
    std::unordered_set<std::pair<entities::Entity*, entities::Entity*>, 
                      EntityPairHash> enabledEventPairs_;
    
    // Statistics
    Stats stats_;
    
    // Previous frame collision states for event generation
    std::unordered_set<std::pair<entities::Entity*, entities::Entity*>, 
                      EntityPairHash> previousCollisions_;
    
    // Helper methods
    CollisionResolution calculateResolution(entities::Entity* entity, const CollisionResult& collision);
    sf::Vector2f calculateMinimumTranslationVector(const sf::FloatRect& a, const sf::FloatRect& b);
    bool shouldResolveCollision(entities::Entity* entity, entities::Entity* other);
    void updateCollisionEvents(const std::vector<CollisionResult>& collisions, float deltaTime);
    
    // Entity pair utilities
    std::pair<entities::Entity*, entities::Entity*> makeEntityPair(entities::Entity* a, entities::Entity* b);
    bool areEventsEnabled(entities::Entity* a, entities::Entity* b) const;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONSYSTEM_H
