#include "AISystem.h"
#include "entities/Entity.h"
#include "entities/EntityManager.h"
#include "entities/Player.h"
#include "collisions/CollisionManager.h"
#include "core/Logger.h"
#include <algorithm>
#include <chrono>

namespace ai {

AIAgent::AIAgent(entities::Entity* entity, const AIAgentConfig& config)
    : entity_(entity)
    , config_(config)
    , currentState_(AIState::IDLE)
    , previousState_(AIState::IDLE)
    , timeInCurrentState_(0.0f)
    , timeInPreviousState_(0.0f)
    , primaryTarget_(nullptr)
    , currentPatrolIndex_(0)
    , currentPathIndex_(0)
    , targetPosition_(0, 0)
    , lastKnownPlayerPosition_(0, 0)
    , timeSincePlayerSeen_(0.0f)
    , isAlert_(false)
    , alertTimer_(0.0f)
    , lastAlertTime_(0.0f)
    , investigationTimer_(0.0f)
    , stunnedTimer_(0.0f)
    , attackCooldown_(0.0f)
    , fleeCooldown_(0.0f)
    , updateTimeAccumulator_(0.0f)
    , updateCount_(0)
{
    perceptionSystem_ = std::make_unique<PerceptionSystem>(config_.perception);
    pathfindingSystem_ = std::make_unique<PathfindingSystem>(config_.pathfinding);
}

AIAgent::~AIAgent() = default;

void AIAgent::update(float deltaTime, entities::EntityManager* entityManager, 
                    collisions::CollisionManager* collisionManager) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!entity_ || !entity_->isActive()) {
        return;
    }
    
    // Update timers
    timeInCurrentState_ += deltaTime;
    timeSincePlayerSeen_ += deltaTime;
    attackCooldown_ = std::max(0.0f, attackCooldown_ - deltaTime);
    fleeCooldown_ = std::max(0.0f, fleeCooldown_ - deltaTime);
    investigationTimer_ = std::max(0.0f, investigationTimer_ - deltaTime);
    stunnedTimer_ = std::max(0.0f, stunnedTimer_ - deltaTime);
    alertTimer_ = std::max(0.0f, alertTimer_ - deltaTime);
    
    // Handle stunned state
    if (stunnedTimer_ > 0.0f) {
        executeStunned(deltaTime);
        return;
    }
    
    // Update perception
    performanceStats_.perceptionChecks++;
    auto perceptions = perceptionSystem_->updatePerception(
        entity_, getEntityPosition(), getFacingDirection(),
        entityManager, collisionManager, timeInCurrentState_
    );
    
    recentPerceptions_ = perceptions;
    
    // Process perception events
    for (const auto& perception : perceptions) {
        if (perception.type == PerceptionType::SIGHT && perception.source) {
            if (auto* player = dynamic_cast<entities::Player*>(perception.source)) {
                lastKnownPlayerPosition_ = perception.position;
                timeSincePlayerSeen_ = 0.0f;
                addTarget(perception.source, Priority::HIGH);
            }
        }
    }
    
    // Make behavioral decision
    auto decision = makeDecision(perceptions, entityManager, collisionManager);
    
    // Execute the decision
    if (decision.newState != currentState_) {
        changeState(decision.newState, decision.reason);
    }
    
    // Update target position if provided
    if (decision.targetPosition != sf::Vector2f(0, 0)) {
        targetPosition_ = decision.targetPosition;
    }
    
    // Execute current state behavior
    switch (currentState_) {
        case AIState::IDLE:
            executeIdle(deltaTime);
            break;
        case AIState::PATROL:
            executePatrol(deltaTime, collisionManager);
            break;
        case AIState::CHASE:
            executeChase(deltaTime, collisionManager);
            break;
        case AIState::ATTACK:
            executeAttack(deltaTime);
            break;
        case AIState::FLEE:
            executeFlee(deltaTime, collisionManager);
            break;
        case AIState::RETURN:
            executeReturn(deltaTime, collisionManager);
            break;
        case AIState::INVESTIGATE:
            executeInvestigate(deltaTime, collisionManager);
            break;
        case AIState::ALERT:
            executeAlert(deltaTime);
            break;
        case AIState::STUNNED:
            executeStunned(deltaTime);
            break;
        case AIState::DEAD:
            // Do nothing when dead
            break;
    }
    
    // Update performance stats
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    updateTimeAccumulator_ += duration.count() / 1000.0f; // Convert to milliseconds
    updateCount_++;
    
    if (updateCount_ >= 60) { // Update average every 60 frames
        performanceStats_.averageUpdateTime = updateTimeAccumulator_ / updateCount_;
        updateTimeAccumulator_ = 0.0f;
        updateCount_ = 0;
    }
}

void AIAgent::changeState(AIState newState, const std::string& reason) {
    if (newState == currentState_) return;
    
    previousState_ = currentState_;
    timeInPreviousState_ = timeInCurrentState_;
    currentState_ = newState;
    timeInCurrentState_ = 0.0f;
    
    performanceStats_.stateChanges++;
    
    std::string logMessage = "[AI] Entity " + std::to_string(entity_->id()) + 
                           " -> " + stateToString(newState);
    if (!reason.empty()) {
        logMessage += " (" + reason + ")";
    }
    
    core::Logger::instance().info(logMessage);
}

BehaviorDecision AIAgent::makeDecision(const std::vector<PerceptionEvent>& perceptions,
                                      entities::EntityManager* entityManager,
                                      collisions::CollisionManager* collisionManager) {
    BehaviorDecision decision(currentState_, Priority::LOW);
    
    // Check for critical situations first
    if (shouldFlee()) {
        decision.newState = AIState::FLEE;
        decision.priority = Priority::CRITICAL;
        decision.reason = "Low health - fleeing";
        
        // Find flee direction (away from threats)
        sf::Vector2f fleeDirection(0, 0);
        for (const auto& perception : perceptions) {
            if (perception.source && perception.type == PerceptionType::SIGHT) {
                sf::Vector2f awayFromThreat = getEntityPosition() - perception.position;
                float length = std::sqrt(awayFromThreat.x * awayFromThreat.x + awayFromThreat.y * awayFromThreat.y);
                if (length > 0.0f) {
                    fleeDirection += (awayFromThreat / length);
                }
            }
        }
        
        if (fleeDirection != sf::Vector2f(0, 0)) {
            float length = std::sqrt(fleeDirection.x * fleeDirection.x + fleeDirection.y * fleeDirection.y);
            if (length > 0.0f) {
                fleeDirection /= length;
                decision.targetPosition = getEntityPosition() + fleeDirection * config_.fleeDistance;
            }
        }
        
        return decision;
    }
    
    // Handle behavior based on profile and perceptions
    entities::Entity* bestTarget = nullptr;
    Priority bestPriority = Priority::LOW;
    
    // Evaluate current targets and perceptions
    for (const auto& perception : perceptions) {
        if (!perception.source) continue;
        
        Priority targetPriority = calculateTargetPriority(perception.source);
        
        if (targetPriority > bestPriority) {
            bestTarget = perception.source;
            bestPriority = targetPriority;
        }
    }
    
    // Decision making based on behavior profile
    if (bestTarget) {
        float distance = distanceTo(bestTarget);
        
        switch (config_.profile) {
            case BehaviorProfile::AGGRESSIVE:
                if (distance <= config_.attackRange && shouldAttack(bestTarget)) {
                    decision.newState = AIState::ATTACK;
                    decision.priority = Priority::HIGH;
                    decision.targetEntity = bestTarget;
                    decision.reason = "Aggressive - attacking target";
                } else if (bestPriority >= Priority::MEDIUM) {
                    decision.newState = AIState::CHASE;
                    decision.priority = Priority::HIGH;
                    decision.targetEntity = bestTarget;
                    decision.targetPosition = bestTarget->position();
                    decision.reason = "Aggressive - chasing target";
                }
                break;
                
            case BehaviorProfile::DEFENSIVE:
                if (bestPriority >= Priority::HIGH) {
                    if (distance <= config_.attackRange * 2.0f) {
                        decision.newState = AIState::FLEE;
                        decision.priority = Priority::HIGH;
                        decision.reason = "Defensive - fleeing from threat";
                    } else {
                        decision.newState = AIState::ALERT;
                        decision.priority = Priority::MEDIUM;
                        decision.reason = "Defensive - staying alert";
                    }
                }
                break;
                
            case BehaviorProfile::NEUTRAL:
                if (distance <= config_.attackRange && shouldAttack(bestTarget)) {
                    decision.newState = AIState::ATTACK;
                    decision.priority = Priority::MEDIUM;
                    decision.targetEntity = bestTarget;
                    decision.reason = "Neutral - defending self";
                } else if (bestPriority >= Priority::HIGH) {
                    decision.newState = AIState::CHASE;
                    decision.priority = Priority::MEDIUM;
                    decision.targetEntity = bestTarget;
                    decision.targetPosition = bestTarget->position();
                    decision.reason = "Neutral - investigating threat";
                }
                break;
                
            case BehaviorProfile::PASSIVE:
                if (bestPriority >= Priority::HIGH) {
                    decision.newState = AIState::FLEE;
                    decision.priority = Priority::HIGH;
                    decision.reason = "Passive - avoiding conflict";
                }
                break;
                
            case BehaviorProfile::GUARD:
                if (distance <= config_.attackRange && shouldAttack(bestTarget)) {
                    decision.newState = AIState::ATTACK;
                    decision.priority = Priority::HIGH;
                    decision.targetEntity = bestTarget;
                    decision.reason = "Guard - defending position";
                } else if (bestPriority >= Priority::MEDIUM) {
                    decision.newState = AIState::ALERT;
                    decision.priority = Priority::MEDIUM;
                    decision.reason = "Guard - heightened awareness";
                }
                break;
                
            case BehaviorProfile::SCOUT:
                if (bestPriority >= Priority::MEDIUM) {
                    decision.newState = AIState::INVESTIGATE;
                    decision.priority = Priority::MEDIUM;
                    decision.targetPosition = bestTarget->position();
                    decision.reason = "Scout - investigating target";
                    
                    // Alert others if configured
                    if (config_.canAlertOthers) {
                        alertNearbyAgents(bestTarget->position());
                    }
                }
                break;
        }
    }
    
    // Default behaviors when no targets
    if (decision.newState == currentState_ && decision.priority == Priority::LOW) {
        switch (currentState_) {
            case AIState::IDLE:
                if (!patrolPoints_.empty()) {
                    decision.newState = AIState::PATROL;
                    decision.reason = "Starting patrol";
                }
                break;
                
            case AIState::CHASE:
            case AIState::INVESTIGATE:
                if (timeSincePlayerSeen_ > config_.investigationTime) {
                    decision.newState = AIState::RETURN;
                    decision.reason = "Lost target - returning";
                }
                break;
                
            case AIState::ALERT:
                if (alertTimer_ <= 0.0f) {
                    decision.newState = !patrolPoints_.empty() ? AIState::PATROL : AIState::IDLE;
                    decision.reason = "Alert timeout - resuming normal behavior";
                }
                break;
                
            case AIState::RETURN:
                if (!patrolPoints_.empty()) {
                    float distanceToPatrol = distanceTo(patrolPoints_[currentPatrolIndex_]);
                    if (distanceToPatrol < 32.0f) {
                        decision.newState = AIState::PATROL;
                        decision.reason = "Reached patrol point";
                    }
                } else {
                    decision.newState = AIState::IDLE;
                    decision.reason = "No patrol points - going idle";
                }
                break;
                
            default:
                break;
        }
    }
    
    return decision;
}

// Implement behavior execution functions
void AIAgent::executeIdle(float deltaTime) {
    // Minimal processing in idle state
}

void AIAgent::executePatrol(float deltaTime, collisions::CollisionManager* cm) {
    if (patrolPoints_.empty()) {
        changeState(AIState::IDLE, "No patrol points");
        return;
    }
    
    sf::Vector2f destination = patrolPoints_[currentPatrolIndex_];
    float distanceToDestination = distanceTo(destination);
    
    if (distanceToDestination < 32.0f) {
        // Reached patrol point, move to next
        currentPatrolIndex_ = (currentPatrolIndex_ + 1) % patrolPoints_.size();
        destination = patrolPoints_[currentPatrolIndex_];
    }
    
    updatePath(destination, cm);
    followPath(deltaTime, cm);
}

void AIAgent::executeChase(float deltaTime, collisions::CollisionManager* cm) {
    entities::Entity* target = getPrimaryTarget();
    if (!target || !isTargetValid(target)) {
        changeState(AIState::RETURN, "Lost chase target");
        return;
    }
    
    updatePath(target->position(), cm);
    followPath(deltaTime, cm);
}

void AIAgent::executeAttack(float deltaTime) {
    entities::Entity* target = getPrimaryTarget();
    if (!target || !isTargetValid(target)) {
        changeState(AIState::RETURN, "Lost attack target");
        return;
    }
    
    float distance = distanceTo(target);
    if (distance > config_.attackRange) {
        changeState(AIState::CHASE, "Target out of attack range");
        return;
    }
    
    if (attackCooldown_ <= 0.0f) {
        // Perform attack
        if (auto* player = dynamic_cast<entities::Player*>(target)) {
            player->applyDamage(10); // Basic damage
            attackCooldown_ = 1.0f; // 1 second cooldown
            
            core::Logger::instance().info("[AI] Entity " + std::to_string(entity_->id()) + 
                                        " attacked player for 10 damage");
        }
    }
}

void AIAgent::executeFlee(float deltaTime, collisions::CollisionManager* cm) {
    if (fleeCooldown_ > 0.0f) {
        return; // Still in flee cooldown
    }
    
    updatePath(targetPosition_, cm);
    followPath(deltaTime, cm);
    
    // Check if we've fled far enough
    float distanceFromDanger = 0.0f;
    for (auto* target : targets_) {
        if (target) {
            distanceFromDanger = std::max(distanceFromDanger, distanceTo(target));
        }
    }
    
    if (distanceFromDanger >= config_.fleeDistance) {
        fleeCooldown_ = 5.0f; // 5 second cooldown before fleeing again
        changeState(AIState::ALERT, "Fled to safety");
    }
}

void AIAgent::executeReturn(float deltaTime, collisions::CollisionManager* cm) {
    if (patrolPoints_.empty()) {
        changeState(AIState::IDLE, "No return point");
        return;
    }
    
    sf::Vector2f returnPoint = patrolPoints_[currentPatrolIndex_];
    float distance = distanceTo(returnPoint);
    
    if (distance < 32.0f) {
        changeState(AIState::PATROL, "Returned to patrol");
        return;
    }
    
    updatePath(returnPoint, cm);
    followPath(deltaTime, cm);
}

void AIAgent::executeInvestigate(float deltaTime, collisions::CollisionManager* cm) {
    float distance = distanceTo(targetPosition_);
    
    if (distance < 32.0f || investigationTimer_ <= 0.0f) {
        changeState(AIState::ALERT, "Investigation complete");
        alertTimer_ = config_.alertDuration;
        return;
    }
    
    updatePath(targetPosition_, cm);
    followPath(deltaTime, cm);
}

void AIAgent::executeAlert(float deltaTime) {
    // Stay in alert state, scan for threats
    if (alertTimer_ <= 0.0f) {
        changeState(!patrolPoints_.empty() ? AIState::PATROL : AIState::IDLE, "Alert timeout");
    }
}

void AIAgent::executeStunned(float deltaTime) {
    // Do nothing while stunned
    if (stunnedTimer_ <= 0.0f) {
        changeState(previousState_, "Recovered from stun");
    }
}

// Utility function implementations
bool AIAgent::shouldFlee() const {
    float healthPercentage = getHealthPercentage();
    return healthPercentage < config_.healthThreshold && config_.cautionLevel > 0.5f;
}

bool AIAgent::shouldAttack(entities::Entity* target) const {
    if (!target) return false;
    
    // Check if it's a valid target type (e.g., player)
    if (dynamic_cast<entities::Player*>(target)) {
        return config_.aggressionLevel > 0.3f;
    }
    
    return false;
}

bool AIAgent::isTargetValid(entities::Entity* target) const {
    return target && target->isActive();
}

Priority AIAgent::calculateTargetPriority(entities::Entity* target) const {
    if (!target) return Priority::LOW;
    
    // Players have high priority if configured
    if (dynamic_cast<entities::Player*>(target) && config_.prioritizePlayerTargets) {
        return Priority::HIGH;
    }
    
    // Distance-based priority
    float distance = distanceTo(target);
    if (distance < config_.attackRange) {
        return Priority::HIGH;
    } else if (distance < config_.perception.sightRange * 0.5f) {
        return Priority::MEDIUM;
    }
    
    return Priority::LOW;
}

void AIAgent::updatePath(const sf::Vector2f& destination, collisions::CollisionManager* cm) {
    if (!pathfindingSystem_ || !cm) return;
    
    sf::Vector2f currentPos = getEntityPosition();
    float distance = std::sqrt(std::pow(destination.x - currentPos.x, 2) + 
                              std::pow(destination.y - currentPos.y, 2));
    
    // Only update path if destination changed significantly or no current path
    if (currentPath_.empty() || distance > 64.0f) {
        performanceStats_.pathfindingRequests++;
        
        auto result = pathfindingSystem_->findPath(currentPos, destination, cm, entity_);
        if (result.success) {
            currentPath_ = result.path;
            currentPathIndex_ = 0;
        }
    }
}

void AIAgent::followPath(float deltaTime, collisions::CollisionManager* cm) {
    if (currentPath_.empty() || currentPathIndex_ >= currentPath_.size()) {
        return;
    }
    
    sf::Vector2f currentPos = getEntityPosition();
    sf::Vector2f targetWaypoint = currentPath_[currentPathIndex_];
    
    // Check if we reached the current waypoint
    float distance = std::sqrt(std::pow(targetWaypoint.x - currentPos.x, 2) + 
                              std::pow(targetWaypoint.y - currentPos.y, 2));
    
    if (distance < 16.0f) {
        currentPathIndex_++;
        if (currentPathIndex_ >= currentPath_.size()) {
            currentPath_.clear();
            return;
        }
        targetWaypoint = currentPath_[currentPathIndex_];
    }
    
    // Move towards waypoint
    sf::Vector2f direction = targetWaypoint - currentPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length > 0.0f) {
        direction /= length;
        sf::Vector2f newPosition = currentPos + direction * config_.speed * deltaTime;
        
        // Simple collision avoidance (this would be handled by the entity's movement system)
        entity_->setPosition(newPosition);
    }
}

void AIAgent::alertNearbyAgents(const sf::Vector2f& alertPosition) {
    // This would require access to other AI agents, which could be provided through EntityManager
    // For now, just log the alert
    core::Logger::instance().info("[AI] Entity " + std::to_string(entity_->id()) + 
                                " alerting others about position (" + 
                                std::to_string(alertPosition.x) + ", " + 
                                std::to_string(alertPosition.y) + ")");
    lastAlertTime_ = timeInCurrentState_;
}

float AIAgent::getHealthPercentage() const {
    // This would need to be implemented based on the entity's health system
    // For now, assume entities don't have health unless they're players
    if (auto* player = dynamic_cast<entities::Player*>(entity_)) {
        return static_cast<float>(player->health()) / 100.0f; // Assuming max health is 100
    }
    return 1.0f; // Non-player entities assumed to be at full health
}

// Add target management functions
void AIAgent::addTarget(entities::Entity* target, Priority priority) {
    if (!target || std::find(targets_.begin(), targets_.end(), target) != targets_.end()) {
        return;
    }
    
    targets_.push_back(target);
    targetPriorities_[target] = priority;
    
    // Update primary target if this one has higher priority
    if (!primaryTarget_ || priority > targetPriorities_[primaryTarget_]) {
        primaryTarget_ = target;
    }
}

void AIAgent::removeTarget(entities::Entity* target) {
    auto it = std::find(targets_.begin(), targets_.end(), target);
    if (it != targets_.end()) {
        targets_.erase(it);
        targetPriorities_.erase(target);
        
        if (primaryTarget_ == target) {
            // Find new primary target
            primaryTarget_ = nullptr;
            Priority bestPriority = Priority::LOW;
            for (auto* t : targets_) {
                Priority p = targetPriorities_[t];
                if (p > bestPriority) {
                    primaryTarget_ = t;
                    bestPriority = p;
                }
            }
        }
    }
}

void AIAgent::clearTargets() {
    targets_.clear();
    targetPriorities_.clear();
    primaryTarget_ = nullptr;
}

entities::Entity* AIAgent::getPrimaryTarget() const {
    return primaryTarget_;
}

// Event handlers
void AIAgent::onDamageReceived(float damage, entities::Entity* source) {
    if (source) {
        addTarget(source, Priority::CRITICAL);
    }
    
    // Become alert
    alertTimer_ = config_.alertDuration;
    if (currentState_ == AIState::IDLE || currentState_ == AIState::PATROL) {
        changeState(AIState::ALERT, "Received damage");
    }
}

void AIAgent::onSoundHeard(const sf::Vector2f& soundPosition, float intensity) {
    if (intensity > 0.5f && 
        (currentState_ == AIState::IDLE || currentState_ == AIState::PATROL)) {
        targetPosition_ = soundPosition;
        investigationTimer_ = config_.investigationTime;
        changeState(AIState::INVESTIGATE, "Heard sound");
    }
}

void AIAgent::onAlertReceived(const sf::Vector2f& alertPosition, entities::Entity* source) {
    if (currentState_ == AIState::IDLE || currentState_ == AIState::PATROL) {
        targetPosition_ = alertPosition;
        alertTimer_ = config_.alertDuration;
        changeState(AIState::ALERT, "Received alert");
    }
}

void AIAgent::onEntityDied(entities::Entity* entity) {
    removeTarget(entity);
}

// Patrol management
void AIAgent::setPatrolPoints(const std::vector<sf::Vector2f>& points) {
    patrolPoints_ = points;
    currentPatrolIndex_ = 0;
}

void AIAgent::addPatrolPoint(const sf::Vector2f& point) {
    patrolPoints_.push_back(point);
}

// Helper functions
float AIAgent::distanceTo(const sf::Vector2f& position) const {
    sf::Vector2f entityPos = getEntityPosition();
    return std::sqrt(std::pow(position.x - entityPos.x, 2) + 
                    std::pow(position.y - entityPos.y, 2));
}

float AIAgent::distanceTo(entities::Entity* entity) const {
    if (!entity) return std::numeric_limits<float>::max();
    return distanceTo(entity->position());
}

sf::Vector2f AIAgent::getEntityPosition() const {
    return entity_ ? entity_->position() : sf::Vector2f(0, 0);
}

sf::Vector2f AIAgent::getFacingDirection() const {
    // This would need to be implemented based on the entity's facing direction
    // For now, return a default direction
    return sf::Vector2f(1, 0);
}

// Debug and performance functions
AIAgent::DebugInfo AIAgent::getDebugInfo() const {
    DebugInfo info;
    info.currentState = currentState_;
    info.profile = config_.profile;
    info.currentPath = currentPath_;
    info.patrolPoints = patrolPoints_;
    info.currentTargets = targets_;
    info.lastPerceptionEvents = recentPerceptions_;
    info.targetPosition = targetPosition_;
    info.timeInCurrentState = timeInCurrentState_;
    
    return info;
}

void AIAgent::resetPerformanceStats() {
    performanceStats_ = PerformanceStats{};
    updateTimeAccumulator_ = 0.0f;
    updateCount_ = 0;
}

} // namespace ai
