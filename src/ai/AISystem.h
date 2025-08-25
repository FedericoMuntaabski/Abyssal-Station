#ifndef ABYSSAL_STATION_SRC_AI_AISYSTEM_H
#define ABYSSAL_STATION_SRC_AI_AISYSTEM_H

#include "AIState.h"
#include "Perception.h"
#include "Pathfinding.h"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace entities { 
    class Entity; 
    class EntityManager;
    class Player;
    class Player;
}
namespace collisions { class CollisionManager; }

namespace ai {

// Behavior decision data
struct BehaviorDecision {
    AIState newState;
    Priority priority;
    sf::Vector2f targetPosition;
    entities::Entity* targetEntity;
    std::string reason;
    
    BehaviorDecision(AIState state = AIState::IDLE, Priority prio = Priority::LOW)
        : newState(state), priority(prio), targetPosition(0, 0), targetEntity(nullptr), reason("") {}
};

// AI agent configuration
struct AIAgentConfig {
    BehaviorProfile profile = BehaviorProfile::NEUTRAL;
    PerceptionConfig perception;
    PathfindingConfig pathfinding;
    
    // Behavior parameters
    float healthThreshold = 0.2f;       // When to flee (20% health)
    float aggressionLevel = 0.5f;       // How likely to attack (0.0-1.0)
    float cautionLevel = 0.5f;          // How likely to flee (0.0-1.0)
    float investigationTime = 5.0f;     // How long to investigate sounds
    float alertDuration = 10.0f;        // How long to stay alert
    float stunnedDuration = 3.0f;       // How long stunning lasts
    
    // Movement parameters
    float speed = 100.0f;
    float attackRange = 32.0f;
    float fleeDistance = 150.0f;        // How far to flee
    
    // Coordination
    bool canAlertOthers = true;         // Can alert other AIs
    float alertRadius = 200.0f;         // How far alerts reach
    
    // Multi-target support
    int maxTargets = 3;                 // Maximum simultaneous targets
    bool prioritizePlayerTargets = true; // Players have higher priority
};

// Enhanced AI agent with advanced behaviors
class AIAgent {
public:
    explicit AIAgent(entities::Entity* entity, const AIAgentConfig& config = AIAgentConfig{});
    ~AIAgent();
    
    // Main update loop
    void update(float deltaTime, entities::EntityManager* entityManager, 
                collisions::CollisionManager* collisionManager);
    
    // State management
    void changeState(AIState newState, const std::string& reason = "");
    AIState getCurrentState() const { return currentState_; }
    BehaviorProfile getProfile() const { return config_.profile; }
    
    // Target management
    void addTarget(entities::Entity* target, Priority priority = Priority::MEDIUM);
    void removeTarget(entities::Entity* target);
    void clearTargets();
    bool hasTarget() const;
    void setTarget(uint32_t entityId, const sf::Vector2f& position);
    void clearTarget();
    sf::Vector2f getTargetPosition() const;
    entities::Entity* getPrimaryTarget() const;
    const std::vector<entities::Entity*>& getAllTargets() const { return targets_; }
    
    // State management
    void setState(AIState state);
    AIState getState() const { return currentState_; }
    
    // External events
    void onDamageReceived(float damage, entities::Entity* source);
    void onSoundHeard(const sf::Vector2f& soundPosition, float intensity);
    void onAlertReceived(const sf::Vector2f& alertPosition, entities::Entity* source);
    void onEntityDied(entities::Entity* entity);
    
    // Patrol management
    void setPatrolPoints(const std::vector<sf::Vector2f>& points);
    void addPatrolPoint(const sf::Vector2f& point);
    const std::vector<sf::Vector2f>& getPatrolPoints() const { return patrolPoints_; }
    
    // Configuration
    void setConfig(const AIAgentConfig& config) { config_ = config; }
    const AIAgentConfig& getConfig() const { return config_; }
    
    // Debug information
    struct DebugInfo {
        AIState currentState;
        BehaviorProfile profile;
        std::vector<sf::Vector2f> currentPath;
        std::vector<sf::Vector2f> patrolPoints;
        std::vector<entities::Entity*> currentTargets;
        std::vector<PerceptionEvent> lastPerceptionEvents;
        sf::Vector2f targetPosition;
        std::string lastDecisionReason;
        float timeInCurrentState;
    };
    DebugInfo getDebugInfo() const;
    
    // Performance monitoring
    struct PerformanceStats {
        int perceptionChecks = 0;
        int pathfindingRequests = 0;
        int stateChanges = 0;
        float averageUpdateTime = 0.0f;
    };
    const PerformanceStats& getPerformanceStats() const { return performanceStats_; }
    void resetPerformanceStats();

private:
    entities::Entity* entity_;
    AIAgentConfig config_;
    
    // State management
    AIState currentState_;
    AIState previousState_;
    float timeInCurrentState_;
    float timeInPreviousState_;
    
    // Behavior systems
    std::unique_ptr<PerceptionSystem> perceptionSystem_;
    std::unique_ptr<PathfindingSystem> pathfindingSystem_;
    
    // Targets and priorities
    std::vector<entities::Entity*> targets_;
    std::unordered_map<entities::Entity*, Priority> targetPriorities_;
    entities::Entity* primaryTarget_;
    
    // Simple target tracking (for Strategy Pattern)
    bool hasSimpleTarget_;
    uint32_t simpleTargetId_;
    sf::Vector2f simpleTargetPosition_;
    
    // Patrol and movement
    std::vector<sf::Vector2f> patrolPoints_;
    size_t currentPatrolIndex_;
    std::vector<sf::Vector2f> currentPath_;
    size_t currentPathIndex_;
    sf::Vector2f targetPosition_;
    
    // Memory and awareness
    std::vector<PerceptionEvent> recentPerceptions_;
    sf::Vector2f lastKnownPlayerPosition_;
    float timeSincePlayerSeen_;
    bool isAlert_;
    float alertTimer_;
    
    // Coordination
    std::vector<AIAgent*> nearbyAgents_;
    float lastAlertTime_;
    
    // Timers and cooldowns
    float investigationTimer_;
    float stunnedTimer_;
    float attackCooldown_;
    float fleeCooldown_;
    
    // Performance tracking
    PerformanceStats performanceStats_;
    float updateTimeAccumulator_;
    int updateCount_;
    
    // Decision making
    BehaviorDecision makeDecision(const std::vector<PerceptionEvent>& perceptions,
                                 entities::EntityManager* entityManager,
                                 collisions::CollisionManager* collisionManager);
    
    // Behavior implementations
    void executeIdle(float deltaTime);
    void executePatrol(float deltaTime, collisions::CollisionManager* cm);
    void executeChase(float deltaTime, collisions::CollisionManager* cm);
    void executeAttack(float deltaTime);
    void executeFlee(float deltaTime, collisions::CollisionManager* cm);
    void executeReturn(float deltaTime, collisions::CollisionManager* cm);
    void executeInvestigate(float deltaTime, collisions::CollisionManager* cm);
    void executeAlert(float deltaTime);
    void executeStunned(float deltaTime);
    
    // Utility functions
    bool shouldFlee() const;
    bool shouldAttack(entities::Entity* target) const;
    bool isTargetValid(entities::Entity* target) const;
    Priority calculateTargetPriority(entities::Entity* target) const;
    void updatePath(const sf::Vector2f& destination, collisions::CollisionManager* cm);
    void followPath(float deltaTime, collisions::CollisionManager* cm);
    void alertNearbyAgents(const sf::Vector2f& alertPosition);
    float getHealthPercentage() const;
    
    // Helper functions
    float distanceTo(const sf::Vector2f& position) const;
    float distanceTo(entities::Entity* entity) const;
    sf::Vector2f getEntityPosition() const;
    sf::Vector2f getFacingDirection() const;
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_AISYSTEM_H
