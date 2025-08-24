#ifndef ABYSSAL_STATION_SRC_AI_PERCEPTION_H
#define ABYSSAL_STATION_SRC_AI_PERCEPTION_H

#include "AIState.h"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <memory>
#include <map>

namespace entities { 
    class Entity; 
    class EntityManager; 
}
namespace collisions { class CollisionManager; }

namespace ai {

// Data structure for perception events
struct PerceptionEvent {
    PerceptionType type;
    entities::Entity* source;       // What was detected
    sf::Vector2f position;          // Where it was detected
    float intensity;                // How strong the detection (0.0-1.0)
    float timestamp;                // When it was detected
    float duration;                 // How long it lasts (for memory)
    
    PerceptionEvent(PerceptionType t, entities::Entity* src, const sf::Vector2f& pos, 
                   float intense = 1.0f, float time = 0.0f, float dur = 5.0f)
        : type(t), source(src), position(pos), intensity(intense), timestamp(time), duration(dur) {}
};

// Configuration for different perception types
struct PerceptionConfig {
    float sightRange = 200.0f;
    float sightAngle = 65.0f;          // Cone angle in degrees
    float hearingRange = 150.0f;
    float proximityRange = 32.0f;
    float memoryDuration = 10.0f;      // How long to remember last seen position
    bool requiresLOS = true;           // Whether sight needs line of sight
    std::uint32_t sightLayerMask = 0xFFFFFFFF;  // What layers block sight
};

// Enhanced perception system supporting multiple detection methods
class PerceptionSystem {
public:
    explicit PerceptionSystem(const PerceptionConfig& config = PerceptionConfig{});
    
    // Update perception for an entity, returns list of perception events
    std::vector<PerceptionEvent> updatePerception(
        entities::Entity* observer,
        const sf::Vector2f& observerPosition,
        const sf::Vector2f& facingDirection,
        entities::EntityManager* entityManager,
        collisions::CollisionManager* collisionManager,
        float deltaTime
    );
    
    // Check specific perception types
    bool canSee(const sf::Vector2f& observerPos, const sf::Vector2f& observerFacing,
                const sf::Vector2f& targetPos, collisions::CollisionManager* cm,
                entities::Entity* excludeEntity = nullptr) const;
    
    bool canHear(const sf::Vector2f& observerPos, const sf::Vector2f& soundPos) const;
    
    bool isInProximity(const sf::Vector2f& observerPos, const sf::Vector2f& targetPos) const;
    
    // Memory management
    void addMemory(entities::Entity* observer, const sf::Vector2f& lastKnownPos, float currentTime);
    sf::Vector2f getLastKnownPosition(entities::Entity* observer, float currentTime) const;
    bool hasValidMemory(entities::Entity* observer, float currentTime) const;
    void clearMemory(entities::Entity* observer);
    
    // Configuration
    void setConfig(const PerceptionConfig& config) { config_ = config; }
    const PerceptionConfig& getConfig() const { return config_; }
    
    // Debug information
    struct DebugInfo {
        std::vector<sf::Vector2f> sightRays;
        std::vector<sf::Vector2f> hearingCircles;
        std::vector<sf::Vector2f> memoryPositions;
    };
    DebugInfo getDebugInfo(entities::Entity* observer) const;

private:
    PerceptionConfig config_;
    
    // Memory storage: observer entity -> {position, timestamp}
    std::map<entities::Entity*, std::pair<sf::Vector2f, float>> memory_;
    
    // Helper functions
    bool isInSightCone(const sf::Vector2f& observerPos, const sf::Vector2f& facingDir,
                       const sf::Vector2f& targetPos) const;
    float calculateAngleBetween(const sf::Vector2f& a, const sf::Vector2f& b) const;
    std::vector<entities::Entity*> getNearbyEntities(
        const sf::Vector2f& position, float radius,
        entities::EntityManager* entityManager,
        entities::Entity* exclude = nullptr
    ) const;
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_PERCEPTION_H
