#include "BehaviorStrategy.h"
#include "AISystem.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"
#include <cmath>
#include <algorithm>

namespace ai {

// AggressiveBehavior implementation
void AggressiveBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                                const std::vector<entities::Entity*>& entities, 
                                float deltaTime) {
    // Check if agent has a target
    if (agent.hasTarget()) {
        sf::Vector2f targetPos = agent.getTargetPosition();
        sf::Vector2f currentPos = entity.getPosition();
        
        // Calculate distance to target
        float dx = targetPos.x - currentPos.x;
        float dy = targetPos.y - currentPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // If close enough, attack; otherwise chase
        if (distance < attackRange_) {
            // Attack behavior
            Logger::log(LogLevel::DEBUG, "AI", "Aggressive behavior: Attacking target");
            agent.setState(AIState::ATTACK);
        } else {
            // Chase behavior - move toward target
            if (distance > 0) {
                sf::Vector2f direction(dx / distance, dy / distance);
                sf::Vector2f newPos = currentPos + direction * chaseSpeed_ * deltaTime;
                entity.setPosition(newPos);
                agent.setState(AIState::CHASE);
            }
        }
    } else {
        // No target - search for enemies aggressively
        for (entities::Entity* other : entities) {
            if (other && other->getId() != entity.getId()) {
                sf::Vector2f otherPos = other->getPosition();
                sf::Vector2f currentPos = entity.getPosition();
                
                float dx = otherPos.x - currentPos.x;
                float dy = otherPos.y - currentPos.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance < detectionRange_) {
                    agent.setTarget(other->getId(), otherPos);
                    Logger::log(LogLevel::INFO, "AI", "Aggressive behavior: Target acquired");
                    break;
                }
            }
        }
        
        // If no target found, patrol aggressively
        agent.setState(AIState::PATROL);
    }
}

// DefensiveBehavior implementation  
void DefensiveBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                               const std::vector<entities::Entity*>& entities, 
                               float deltaTime) {
    if (agent.hasTarget()) {
        sf::Vector2f targetPos = agent.getTargetPosition();
        sf::Vector2f currentPos = entity.getPosition();
        
        float dx = targetPos.x - currentPos.x;
        float dy = targetPos.y - currentPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Defensive strategy: maintain distance, only attack when threatened
        if (distance < retreatThreshold_) {
            // Retreat - move away from threat
            if (distance > 0) {
                sf::Vector2f direction(-dx / distance, -dy / distance);
                sf::Vector2f newPos = currentPos + direction * retreatSpeed_ * deltaTime;
                entity.setPosition(newPos);
                agent.setState(AIState::FLEE);
            }
        } else if (distance < attackRange_ && distance > retreatThreshold_) {
            // In optimal range - attack
            Logger::log(LogLevel::DEBUG, "AI", "Defensive behavior: Attacking from safe distance");
            agent.setState(AIState::ATTACK);
        } else {
            // Maintain distance and guard position
            agent.setState(AIState::GUARD);
        }
    } else {
        // No immediate threat - patrol defensively
        agent.setState(AIState::PATROL);
    }
}

// NeutralBehavior implementation
void NeutralBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                             const std::vector<entities::Entity*>& entities, 
                             float deltaTime) {
    // Neutral behavior: only react when directly threatened
    if (agent.hasTarget()) {
        sf::Vector2f targetPos = agent.getTargetPosition();
        sf::Vector2f currentPos = entity.getPosition();
        
        float dx = targetPos.x - currentPos.x;
        float dy = targetPos.y - currentPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Only engage if very close (threatened)
        if (distance < threatThreshold_) {
            // React defensively
            if (distance < attackRange_) {
                agent.setState(AIState::ATTACK);
            } else {
                agent.setState(AIState::ALERT);
            }
        } else {
            // Ignore distant threats
            agent.clearTarget();
            agent.setState(AIState::IDLE);
        }
    } else {
        // Continue normal activities
        agent.setState(AIState::IDLE);
    }
}

// PassiveBehavior implementation
void PassiveBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                             const std::vector<entities::Entity*>& entities, 
                             float deltaTime) {
    // Passive behavior: flee from any threats
    if (agent.hasTarget()) {
        sf::Vector2f targetPos = agent.getTargetPosition();
        sf::Vector2f currentPos = entity.getPosition();
        
        // Always flee from any detected threat
        float dx = currentPos.x - targetPos.x;
        float dy = currentPos.y - targetPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > 0) {
            sf::Vector2f direction(dx / distance, dy / distance);
            sf::Vector2f newPos = currentPos + direction * fleeSpeed_ * deltaTime;
            entity.setPosition(newPos);
            agent.setState(AIState::FLEE);
            
            Logger::log(LogLevel::DEBUG, "AI", "Passive behavior: Fleeing from threat");
        }
    } else {
        // When safe, remain idle or hide
        agent.setState(AIState::IDLE);
    }
}

// GuardBehavior implementation
void GuardBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                           const std::vector<entities::Entity*>& entities, 
                           float deltaTime) {
    sf::Vector2f currentPos = entity.getPosition();
    
    // Check if we're at guard position
    float dx = guardPosition_.x - currentPos.x;
    float dy = guardPosition_.y - currentPos.y;
    float distanceToGuardPos = std::sqrt(dx * dx + dy * dy);
    
    if (agent.hasTarget()) {
        sf::Vector2f targetPos = agent.getTargetPosition();
        float tdx = targetPos.x - currentPos.x;
        float tdy = targetPos.y - currentPos.y;
        float distanceToTarget = std::sqrt(tdx * tdx + tdy * tdy);
        
        // Only pursue if target is within guard area
        if (distanceToTarget > guardRadius_) {
            // Target too far - return to guard position
            agent.clearTarget();
            returnToGuardPosition(agent, entity, deltaTime);
        } else {
            // Engage target within guard area
            if (distanceToTarget < attackRange_) {
                agent.setState(AIState::ATTACK);
            } else {
                // Move toward target but don't leave guard area
                if (distanceToTarget > 0) {
                    sf::Vector2f direction(tdx / distanceToTarget, tdy / distanceToTarget);
                    sf::Vector2f newPos = currentPos + direction * guardSpeed_ * deltaTime;
                    
                    // Ensure we don't leave guard radius
                    float gdx = guardPosition_.x - newPos.x;
                    float gdy = guardPosition_.y - newPos.y;
                    float newDistanceToGuard = std::sqrt(gdx * gdx + gdy * gdy);
                    
                    if (newDistanceToGuard <= guardRadius_) {
                        entity.setPosition(newPos);
                        agent.setState(AIState::CHASE);
                    }
                }
            }
        }
    } else {
        // No target - scan for threats or return to position
        scanForThreats(agent, entity, entities);
        
        if (distanceToGuardPos > positionTolerance_) {
            returnToGuardPosition(agent, entity, deltaTime);
        } else {
            agent.setState(AIState::GUARD);
        }
    }
}

void GuardBehavior::setGuardPosition(const sf::Vector2f& position) {
    guardPosition_ = position;
}

void GuardBehavior::setGuardRadius(float radius) {
    guardRadius_ = radius;
}

void GuardBehavior::returnToGuardPosition(AIAgent& agent, entities::Entity& entity, float deltaTime) {
    sf::Vector2f currentPos = entity.getPosition();
    float dx = guardPosition_.x - currentPos.x;
    float dy = guardPosition_.y - currentPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0) {
        sf::Vector2f direction(dx / distance, dy / distance);
        sf::Vector2f newPos = currentPos + direction * guardSpeed_ * deltaTime;
        entity.setPosition(newPos);
        agent.setState(AIState::RETURN_TO_POST);
    }
}

void GuardBehavior::scanForThreats(AIAgent& agent, entities::Entity& entity, 
                                  const std::vector<entities::Entity*>& entities) {
    sf::Vector2f currentPos = entity.getPosition();
    
    for (entities::Entity* other : entities) {
        if (other && other->getId() != entity.getId()) {
            sf::Vector2f otherPos = other->getPosition();
            
            float dx = otherPos.x - currentPos.x;
            float dy = otherPos.y - currentPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= guardRadius_) {
                agent.setTarget(other->getId(), otherPos);
                Logger::log(LogLevel::INFO, "AI", "Guard behavior: Threat detected in guard area");
                break;
            }
        }
    }
}

// ScoutBehavior implementation
void ScoutBehavior::execute(AIAgent& agent, entities::Entity& entity, 
                           const std::vector<entities::Entity*>& entities, 
                           float deltaTime) {
    if (patrolPoints_.empty()) {
        agent.setState(AIState::IDLE);
        return;
    }
    
    sf::Vector2f currentPos = entity.getPosition();
    sf::Vector2f targetPoint = patrolPoints_[currentPatrolIndex_];
    
    // Check if we've reached current patrol point
    float dx = targetPoint.x - currentPos.x;
    float dy = targetPoint.y - currentPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < pointReachThreshold_) {
        // Reached point - move to next
        currentPatrolIndex_ = (currentPatrolIndex_ + 1) % patrolPoints_.size();
        Logger::log(LogLevel::DEBUG, "AI", "Scout behavior: Reached patrol point, moving to next");
    } else {
        // Move toward current patrol point
        if (distance > 0) {
            sf::Vector2f direction(dx / distance, dy / distance);
            sf::Vector2f newPos = currentPos + direction * scoutSpeed_ * deltaTime;
            entity.setPosition(newPos);
        }
    }
    
    // Scout behavior: report threats but don't engage
    if (agent.hasTarget()) {
        sf::Vector2f threatPos = agent.getTargetPosition();
        float tdx = threatPos.x - currentPos.x;
        float tdy = threatPos.y - currentPos.y;
        float threatDistance = std::sqrt(tdx * tdx + tdy * tdy);
        
        if (threatDistance < fleeThreshold_) {
            // Too close - flee while reporting
            if (threatDistance > 0) {
                sf::Vector2f fleeDirection(-tdx / threatDistance, -tdy / threatDistance);
                sf::Vector2f newPos = currentPos + fleeDirection * scoutSpeed_ * deltaTime;
                entity.setPosition(newPos);
                agent.setState(AIState::FLEE);
            }
        } else {
            // Safe distance - continue patrol while observing
            agent.setState(AIState::ALERT);
        }
        
        Logger::log(LogLevel::INFO, "AI", "Scout behavior: Threat observed and reported");
    } else {
        // Normal patrol
        agent.setState(AIState::PATROL);
        
        // Scan for new threats
        scanForThreats(agent, entity, entities);
    }
}

void ScoutBehavior::addPatrolPoint(const sf::Vector2f& point) {
    patrolPoints_.push_back(point);
}

void ScoutBehavior::clearPatrolPoints() {
    patrolPoints_.clear();
    currentPatrolIndex_ = 0;
}

void ScoutBehavior::scanForThreats(AIAgent& agent, entities::Entity& entity, 
                                  const std::vector<entities::Entity*>& entities) {
    sf::Vector2f currentPos = entity.getPosition();
    
    for (entities::Entity* other : entities) {
        if (other && other->getId() != entity.getId()) {
            sf::Vector2f otherPos = other->getPosition();
            
            float dx = otherPos.x - currentPos.x;
            float dy = otherPos.y - currentPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= detectionRange_) {
                agent.setTarget(other->getId(), otherPos);
                Logger::log(LogLevel::INFO, "AI", "Scout behavior: New threat detected");
                break;
            }
        }
    }
}

} // namespace ai
