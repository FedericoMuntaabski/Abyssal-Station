#include "AIManager.h"
#include "../entities/Entity.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"
#include "../collisions/CollisionManager.h"
#include "../core/Logger.h"
#include <algorithm>
#include <chrono>

namespace ai {

AIManager::AIManager(const CoordinationConfig& config)
    : coordinationConfig_(config)
    , coordinationUpdateTimer_(0.0f)
    , performanceUpdateTimer_(0.0f)
{
}

AIManager::~AIManager() = default;

void AIManager::addAgent(entities::Entity* entity, const AIAgentConfig& agentConfig) {
    if (!entity) return;
    
    // Remove existing agent if present
    removeAgent(entity);
    
    // Create new agent
    auto agent = std::make_unique<AIAgent>(entity, agentConfig);
    agents_[entity] = std::move(agent);
    
    // Update active agents list
    updateActiveAgentsList();
    
    core::Logger::instance().info("[AI] Added AI agent for entity " + std::to_string(entity->id()));
}

void AIManager::removeAgent(entities::Entity* entity) {
    auto it = agents_.find(entity);
    if (it != agents_.end()) {
        agents_.erase(it);
        updateActiveAgentsList();
        
        core::Logger::instance().info("[AI] Removed AI agent for entity " + std::to_string(entity->id()));
    }
}

AIAgent* AIManager::getAgent(entities::Entity* entity) {
    auto it = agents_.find(entity);
    return it != agents_.end() ? it->second.get() : nullptr;
}

void AIManager::clearAllAgents() {
    agents_.clear();
    activeAgents_.clear();
    legacyEnemies_.clear();
    
    core::Logger::instance().info("[AI] Cleared all AI agents");
}

void AIManager::addEnemyPointer(Enemy* enemy) {
    if (enemy && std::find(legacyEnemies_.begin(), legacyEnemies_.end(), enemy) == legacyEnemies_.end()) {
        legacyEnemies_.push_back(enemy);
    }
}

void AIManager::removeEnemyPointer(Enemy* enemy) {
    auto it = std::find(legacyEnemies_.begin(), legacyEnemies_.end(), enemy);
    if (it != legacyEnemies_.end()) {
        legacyEnemies_.erase(it);
    }
}

void AIManager::updateAll(float deltaTime, entities::EntityManager* entityManager, 
                         collisions::CollisionManager* collisionManager) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Update coordination system
    if (coordinationConfig_.enableCoordination) {
        updateCoordination(deltaTime);
    }
    
    // Update all AI agents
    for (auto* agent : activeAgents_) {
        if (agent) {
            agent->update(deltaTime, entityManager, collisionManager);
        }
    }
    
            // Update legacy enemies for backward compatibility
    for (auto* enemy : legacyEnemies_) {
        if (enemy && enemy->isActive()) {
            // Assuming enemy has an update method that takes player position
            // This maintains backward compatibility
            if (entityManager) {
                auto players = entityManager->getEntitiesOfType<entities::Player>();
                if (!players.empty()) {
                    enemy->update(deltaTime, players[0]->position());
                } else {
                    enemy->update(deltaTime);
                }
            } else {
                enemy->update(deltaTime);
            }
        }
    }
    
    // Update performance metrics
    performanceUpdateTimer_ += deltaTime;
    if (performanceUpdateTimer_ >= 1.0f) { // Update every second
        updatePerformanceMetrics();
        performanceUpdateTimer_ = 0.0f;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    performanceMetrics_.coordinationUpdateTime = duration.count() / 1000.0f; // Convert to milliseconds
}

void AIManager::updateCoordination(float deltaTime) {
    coordinationUpdateTimer_ += deltaTime;
    
    if (coordinationUpdateTimer_ >= coordinationConfig_.coordinationUpdateInterval) {
        coordinationUpdateTimer_ = 0.0f;
        
        // Update shared target information
        if (coordinationConfig_.shareTargetInformation) {
            for (auto* agent : activeAgents_) {
                if (!agent) continue;
                
                auto* primaryTarget = agent->getPrimaryTarget();
                if (primaryTarget) {
                    shareTargetInformation(primaryTarget, primaryTarget->position());
                }
            }
        }
        
        // Clean up old alerts
        if (recentAlerts_.size() > 10) {
            recentAlerts_.erase(recentAlerts_.begin(), recentAlerts_.begin() + 5);
        }
    }
}

void AIManager::updateActiveAgentsList() {
    activeAgents_.clear();
    
    for (auto& pair : agents_) {
        if (pair.second && pair.first && pair.first->isActive()) {
            activeAgents_.push_back(pair.second.get());
        }
    }
}

void AIManager::alertAgentsInRadius(const sf::Vector2f& position, float radius, entities::Entity* source) {
    if (!coordinationConfig_.enableCoordination) return;
    
    recentAlerts_.push_back(position);
    
    auto agentsInRange = getAgentsInRadius(position, radius);
    
    for (auto* agent : agentsInRange) {
        if (agent) {
            agent->onAlertReceived(position, source);
        }
    }
    
    core::Logger::instance().info("[AI] Alerted " + std::to_string(agentsInRange.size()) + 
                                " agents at position (" + std::to_string(position.x) + 
                                ", " + std::to_string(position.y) + ")");
}

void AIManager::shareTargetInformation(entities::Entity* target, const sf::Vector2f& lastKnownPosition) {
    if (!coordinationConfig_.shareTargetInformation || !target) return;
    
    sharedTargetPositions_[target] = lastKnownPosition;
    
    // Notify nearby agents about the target
    for (auto* agent : activeAgents_) {
        if (agent) {
            agent->addTarget(target, Priority::MEDIUM);
        }
    }
}

void AIManager::onEntityDamaged(entities::Entity* entity, float damage, entities::Entity* source) {
    if (!entity) return;
    
    // Notify the damaged entity's agent
    auto* agent = getAgent(entity);
    if (agent) {
        agent->onDamageReceived(damage, source);
    }
    
    // Alert nearby agents if coordination is enabled
    if (coordinationConfig_.enableCoordination && source) {
        alertAgentsInRadius(entity->position(), coordinationConfig_.alertRadius, source);
    }
}

void AIManager::onEntityDied(entities::Entity* entity) {
    if (!entity) return;
    
    // Remove from shared targets
    sharedTargetPositions_.erase(entity);
    
    // Notify all agents that this entity died
    for (auto* agent : activeAgents_) {
        if (agent) {
            agent->onEntityDied(entity);
        }
    }
    
    // Remove the agent if it was the dead entity
    removeAgent(entity);
    
    core::Logger::instance().info("[AI] Entity " + std::to_string(entity->id()) + " died, notified all agents");
}

void AIManager::onSoundMade(const sf::Vector2f& position, float intensity, entities::Entity* source) {
    if (!coordinationConfig_.enableCoordination) return;
    
    float hearingRadius = intensity * 150.0f; // Scale intensity to radius
    auto agentsInRange = getAgentsInRadius(position, hearingRadius);
    
    for (auto* agent : agentsInRange) {
        if (agent) {
            agent->onSoundHeard(position, intensity);
        }
    }
    
    core::Logger::instance().info("[AI] Sound at (" + std::to_string(position.x) + 
                                 ", " + std::to_string(position.y) + 
                                 ") intensity " + std::to_string(intensity) + 
                                 " heard by " + std::to_string(agentsInRange.size()) + " agents");
}

std::vector<AIAgent*> AIManager::getAgentsInRadius(const sf::Vector2f& position, float radius) {
    std::vector<AIAgent*> agentsInRange;
    
    float radiusSquared = radius * radius;
    
    for (auto* agent : activeAgents_) {
        if (!agent) continue;
        
        // Get agent's entity position
        sf::Vector2f agentPos(0, 0);
        auto debugInfo = agent->getDebugInfo();
        // Note: We need a way to get the agent's position. This would need to be added to AIAgent
        // For now, skip position check or assume all agents are in range
        agentsInRange.push_back(agent);
    }
    
    return agentsInRange;
}

void AIManager::broadcastAlert(const sf::Vector2f& position, entities::Entity* source) {
    alertAgentsInRadius(position, coordinationConfig_.alertRadius, source);
}

void AIManager::updatePerformanceMetrics() {
    performanceMetrics_.totalAgents = static_cast<int>(agents_.size());
    performanceMetrics_.activeAgents = static_cast<int>(activeAgents_.size());
    
    // Aggregate performance stats from all agents
    performanceMetrics_.totalPerceptionChecks = 0;
    performanceMetrics_.totalPathfindingRequests = 0;
    performanceMetrics_.totalStateChanges = 0;
    float totalUpdateTime = 0.0f;
    
    for (auto* agent : activeAgents_) {
        if (agent) {
            auto stats = agent->getPerformanceStats();
            performanceMetrics_.totalPerceptionChecks += stats.perceptionChecks;
            performanceMetrics_.totalPathfindingRequests += stats.pathfindingRequests;
            performanceMetrics_.totalStateChanges += stats.stateChanges;
            totalUpdateTime += stats.averageUpdateTime;
        }
    }
    
    if (!activeAgents_.empty()) {
        performanceMetrics_.averageUpdateTime = totalUpdateTime / activeAgents_.size();
    }
}

AIManager::PerformanceMetrics AIManager::getPerformanceMetrics() const {
    return performanceMetrics_;
}

void AIManager::resetPerformanceMetrics() {
    performanceMetrics_ = PerformanceMetrics{};
    
    for (auto* agent : activeAgents_) {
        if (agent) {
            agent->resetPerformanceStats();
        }
    }
}

AIManager::DebugInfo AIManager::getDebugInfo() const {
    DebugInfo info;
    
    // Collect debug info from all agents
    for (auto* agent : activeAgents_) {
        if (agent) {
            info.agentDebugInfo.push_back(agent->getDebugInfo());
        }
    }
    
    // Add alert positions
    info.alertPositions = recentAlerts_;
    
    // Add performance metrics
    info.performance = performanceMetrics_;
    
    // Add coordination links (could be expanded to show agent relationships)
    for (const auto& targetPos : sharedTargetPositions_) {
        // For now, just add the shared target positions
        info.coordinationLinks.emplace_back(targetPos.second, targetPos.second);
    }
    
    return info;
}

} // namespace ai
