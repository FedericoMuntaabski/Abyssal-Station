#ifndef ABYSSAL_STATION_SRC_AI_AIMANAGER_H
#define ABYSSAL_STATION_SRC_AI_AIMANAGER_H

#include "AISystem.h"
#include "Enemy.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace entities { 
    class EntityManager; 
    class Entity;
}
namespace collisions { class CollisionManager; }

namespace ai {

// Configuration for AI coordination
struct CoordinationConfig {
    bool enableCoordination = true;
    float alertRadius = 200.0f;
    float coordinationUpdateInterval = 0.1f;  // How often to update coordination (seconds)
    int maxCoordinatedAgents = 10;           // Maximum agents that can coordinate
    bool shareTargetInformation = true;
    bool enableGroupBehaviors = true;
};

// Enhanced AI manager with coordination and performance monitoring
class AIManager {
public:
    explicit AIManager(const CoordinationConfig& config = CoordinationConfig{});
    ~AIManager();
    
    // Agent management
    void addAgent(entities::Entity* entity, const AIAgentConfig& agentConfig = AIAgentConfig{});
    void removeAgent(entities::Entity* entity);
    AIAgent* getAgent(entities::Entity* entity);
    void clearAllAgents();
    
    // Legacy Enemy support for backward compatibility
    void addEnemyPointer(Enemy* enemy);
    void removeEnemyPointer(Enemy* enemy);
    
    // Update all AI agents
    void updateAll(float deltaTime, entities::EntityManager* entityManager, 
                   collisions::CollisionManager* collisionManager);
    
    // Coordination features
    void alertAgentsInRadius(const sf::Vector2f& position, float radius, 
                           entities::Entity* source = nullptr);
    void shareTargetInformation(entities::Entity* target, const sf::Vector2f& lastKnownPosition);
    
    // Event broadcasting
    void onEntityDamaged(entities::Entity* entity, float damage, entities::Entity* source);
    void onEntityDied(entities::Entity* entity);
    void onSoundMade(const sf::Vector2f& position, float intensity, entities::Entity* source = nullptr);
    
    // Performance monitoring
    struct PerformanceMetrics {
        int totalAgents = 0;
        int activeAgents = 0;
        float averageUpdateTime = 0.0f;
        int totalPerceptionChecks = 0;
        int totalPathfindingRequests = 0;
        int totalStateChanges = 0;
        float coordinationUpdateTime = 0.0f;
    };
    PerformanceMetrics getPerformanceMetrics() const;
    void resetPerformanceMetrics();
    
    // Configuration
    void setCoordinationConfig(const CoordinationConfig& config) { coordinationConfig_ = config; }
    const CoordinationConfig& getCoordinationConfig() const { return coordinationConfig_; }
    
    // Debug information
    struct DebugInfo {
        std::vector<AIAgent::DebugInfo> agentDebugInfo;
        std::vector<sf::Vector2f> alertPositions;
        std::vector<std::pair<sf::Vector2f, sf::Vector2f>> coordinationLinks;
        PerformanceMetrics performance;
    };
    DebugInfo getDebugInfo() const;

private:
    CoordinationConfig coordinationConfig_;
    
    // Agent storage
    std::unordered_map<entities::Entity*, std::unique_ptr<AIAgent>> agents_;
    std::vector<AIAgent*> activeAgents_;  // Cache for performance
    
    // Legacy enemy support
    std::vector<Enemy*> legacyEnemies_;
    
    // Coordination data
    std::vector<sf::Vector2f> recentAlerts_;
    std::unordered_map<entities::Entity*, sf::Vector2f> sharedTargetPositions_;
    float coordinationUpdateTimer_;
    
    // Performance tracking
    mutable PerformanceMetrics performanceMetrics_;
    float performanceUpdateTimer_;
    
    // Internal methods
    void updateCoordination(float deltaTime);
    void updateActiveAgentsList();
    std::vector<AIAgent*> getAgentsInRadius(const sf::Vector2f& position, float radius);
    void broadcastAlert(const sf::Vector2f& position, entities::Entity* source);
    void updatePerformanceMetrics();
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_AIMANAGER_H
