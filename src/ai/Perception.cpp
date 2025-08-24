#include "Perception.h"
#include "entities/Entity.h"
#include "entities/EntityManager.h"
#include "collisions/CollisionManager.h"
#include "core/Logger.h"
#include <cmath>
#include <algorithm>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ai {

PerceptionSystem::PerceptionSystem(const PerceptionConfig& config) : config_(config) {}

std::vector<PerceptionEvent> PerceptionSystem::updatePerception(
    entities::Entity* observer,
    const sf::Vector2f& observerPosition,
    const sf::Vector2f& facingDirection,
    entities::EntityManager* entityManager,
    collisions::CollisionManager* collisionManager,
    float deltaTime
) {
    std::vector<PerceptionEvent> events;
    
    if (!observer || !entityManager) {
        return events;
    }
    
    // Get nearby entities for performance
    auto nearbyEntities = getNearbyEntities(
        observerPosition, 
        std::max({config_.sightRange, config_.hearingRange, config_.proximityRange}),
        entityManager,
        observer
    );
    
    for (auto* entity : nearbyEntities) {
        if (!entity || entity == observer) continue;
        
        sf::Vector2f targetPos = entity->position();
        float distance = std::sqrt(std::pow(targetPos.x - observerPosition.x, 2) + 
                                 std::pow(targetPos.y - observerPosition.y, 2));
        
        // Check sight perception
        if (distance <= config_.sightRange && canSee(observerPosition, facingDirection, targetPos, collisionManager, observer)) {
            float intensity = 1.0f - (distance / config_.sightRange);
            events.emplace_back(PerceptionType::SIGHT, entity, targetPos, intensity);
            
            // Update memory when we see something
            addMemory(observer, targetPos, deltaTime);
        }
        
        // Check hearing perception
        if (distance <= config_.hearingRange && canHear(observerPosition, targetPos)) {
            float intensity = 1.0f - (distance / config_.hearingRange);
            events.emplace_back(PerceptionType::HEARING, entity, targetPos, intensity);
        }
        
        // Check proximity perception
        if (distance <= config_.proximityRange && isInProximity(observerPosition, targetPos)) {
            float intensity = 1.0f - (distance / config_.proximityRange);
            events.emplace_back(PerceptionType::PROXIMITY, entity, targetPos, intensity);
        }
    }
    
    // Check memory-based perception
    if (hasValidMemory(observer, deltaTime)) {
        sf::Vector2f memoryPos = getLastKnownPosition(observer, deltaTime);
        events.emplace_back(PerceptionType::MEMORY, nullptr, memoryPos, 0.5f, deltaTime, config_.memoryDuration);
    }
    
    return events;
}

bool PerceptionSystem::canSee(const sf::Vector2f& observerPos, const sf::Vector2f& observerFacing,
                             const sf::Vector2f& targetPos, collisions::CollisionManager* cm,
                             entities::Entity* excludeEntity) const {
    // Check distance first for performance and correctness
    float distance = std::sqrt(std::pow(targetPos.x - observerPos.x, 2) + 
                              std::pow(targetPos.y - observerPos.y, 2));
    if (distance > config_.sightRange) {
        return false;
    }
    
    // Check if target is in sight cone
    if (!isInSightCone(observerPos, observerFacing, targetPos)) {
        return false;
    }
    
    // Check line of sight if collision manager is available
    if (cm && config_.requiresLOS) {
        return !cm->segmentIntersectsAny(observerPos, targetPos, excludeEntity, config_.sightLayerMask);
    }
    
    return true;
}

bool PerceptionSystem::canHear(const sf::Vector2f& observerPos, const sf::Vector2f& soundPos) const {
    float distance = std::sqrt(std::pow(soundPos.x - observerPos.x, 2) + 
                              std::pow(soundPos.y - observerPos.y, 2));
    return distance <= config_.hearingRange;
}

bool PerceptionSystem::isInProximity(const sf::Vector2f& observerPos, const sf::Vector2f& targetPos) const {
    float distance = std::sqrt(std::pow(targetPos.x - observerPos.x, 2) + 
                              std::pow(targetPos.y - observerPos.y, 2));
    return distance <= config_.proximityRange;
}

void PerceptionSystem::addMemory(entities::Entity* observer, const sf::Vector2f& lastKnownPos, float currentTime) {
    memory_[observer] = {lastKnownPos, currentTime};
}

sf::Vector2f PerceptionSystem::getLastKnownPosition(entities::Entity* observer, float currentTime) const {
    auto it = memory_.find(observer);
    if (it != memory_.end()) {
        return it->second.first;
    }
    return {0.0f, 0.0f};
}

bool PerceptionSystem::hasValidMemory(entities::Entity* observer, float currentTime) const {
    auto it = memory_.find(observer);
    if (it != memory_.end()) {
        float elapsed = currentTime - it->second.second;
        return elapsed <= config_.memoryDuration;
    }
    return false;
}

void PerceptionSystem::clearMemory(entities::Entity* observer) {
    memory_.erase(observer);
}

bool PerceptionSystem::isInSightCone(const sf::Vector2f& observerPos, const sf::Vector2f& facingDir,
                                    const sf::Vector2f& targetPos) const {
    sf::Vector2f toTarget = targetPos - observerPos;
    float angle = calculateAngleBetween(facingDir, toTarget);
    return std::abs(angle) <= (config_.sightAngle * 0.5f * M_PI / 180.0f);
}

float PerceptionSystem::calculateAngleBetween(const sf::Vector2f& a, const sf::Vector2f& b) const {
    float dot = a.x * b.x + a.y * b.y;
    float lenA = std::sqrt(a.x * a.x + a.y * a.y);
    float lenB = std::sqrt(b.x * b.x + b.y * b.y);
    
    if (lenA < 0.0001f || lenB < 0.0001f) return 0.0f;
    
    float cosAngle = dot / (lenA * lenB);
    cosAngle = std::max(-1.0f, std::min(1.0f, cosAngle)); // Clamp to prevent NaN
    return std::acos(cosAngle);
}

std::vector<entities::Entity*> PerceptionSystem::getNearbyEntities(
    const sf::Vector2f& position, float radius,
    entities::EntityManager* entityManager,
    entities::Entity* exclude
) const {
    std::vector<entities::Entity*> nearby;
    
    if (!entityManager) return nearby;
    
    // Get all entities and filter by distance
    // Note: This is a simple implementation. For better performance,
    // you might want to use spatial partitioning in EntityManager
    auto allEntities = entityManager->allEntities();
    
    for (auto* entity : allEntities) {
        if (!entity || entity == exclude) continue;
        
        sf::Vector2f entityPos = entity->position();
        float distance = std::sqrt(std::pow(entityPos.x - position.x, 2) + 
                                 std::pow(entityPos.y - position.y, 2));
        
        if (distance <= radius) {
            nearby.push_back(entity);
        }
    }
    
    return nearby;
}

PerceptionSystem::DebugInfo PerceptionSystem::getDebugInfo(entities::Entity* observer) const {
    DebugInfo info;
    
    // Add memory position if available
    auto it = memory_.find(observer);
    if (it != memory_.end()) {
        info.memoryPositions.push_back(it->second.first);
    }
    
    // Additional debug info could be added here for sight rays, hearing circles, etc.
    
    return info;
}

} // namespace ai
