#include "CollisionSystem.h"
#include "CollisionManager.h"
#include "CollisionEvents.h"
#include "../entities/Entity.h"
#include "../entities/Player.h"
#include "../core/Logger.h"

#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <algorithm>
#include <cmath>

namespace collisions {

using core::Logger;

CollisionSystem::CollisionSystem(CollisionManager& manager, const Config& config)
    : manager_(manager), config_(config), logTimer_(0.f), logInterval_(0.25f) {}

CollisionResolution CollisionSystem::resolve(entities::Entity* entity, float deltaTime) {
    CollisionResolution resolution;
    if (!entity) return resolution;

    // accumulate timer for rate-limited logs
    logTimer_ += deltaTime;

    // Get detailed collision information
    auto collisions = manager_.checkCollisionsDetailed(entity);
    
    if (collisions.empty()) {
        return resolution; // No collisions to resolve
    }

    // Update collision events if enabled
    if (config_.enableEvents) {
        updateCollisionEvents(collisions, deltaTime);
    }

    // For now, focus on solid collisions (non-triggers)
    auto solidCollisions = collisions;
    solidCollisions.erase(
        std::remove_if(solidCollisions.begin(), solidCollisions.end(),
            [](const CollisionResult& collision) {
                return collision.isTrigger;
            }),
        solidCollisions.end());

    if (solidCollisions.empty()) {
        return resolution; // Only trigger collisions
    }

    // Calculate resolution for the most significant collision
    auto& primaryCollision = solidCollisions[0];
    resolution = calculateResolution(entity, primaryCollision);

    // Apply resolution if valid
    if (resolution.wasResolved && shouldResolveCollision(entity, primaryCollision.entityB)) {
        sf::Vector2f newPos = entity->position() + resolution.correction;
        
        // Clamp correction distance
        float correctionDistance = std::sqrt(
            resolution.correction.x * resolution.correction.x + 
            resolution.correction.y * resolution.correction.y);
        
        if (correctionDistance <= config_.maxCorrectionDistance) {
            entity->setPosition(newPos);
            
            stats_.totalResolutions++;
            stats_.totalCorrectionDistance += correctionDistance;

            // Classify collision for logging
            const char* collisionDesc = "Entity vs Entity";
            if (dynamic_cast<entities::Player*>(entity)) {
                collisionDesc = "Player vs Entity";
            }

            // Rate-limit collision logs
            if (config_.logResolutions && logTimer_ >= logInterval_) {
                logTimer_ = 0.f;
                Logger::instance().info(std::string("[CollisionSystem] ") + collisionDesc +
                    " idA=" + std::to_string(entity->id()) + " idB=" + std::to_string(primaryCollision.entityB->id()));

                Logger::instance().info(std::string("[CollisionSystem] Resolved collision: entity id=") + 
                    std::to_string(entity->id()) +
                    " moved by (" + std::to_string(resolution.correction.x) + "," + 
                    std::to_string(resolution.correction.y) + ") to (" +
                    std::to_string(newPos.x) + "," + std::to_string(newPos.y) + ")");
            }
        } else {
            Logger::instance().warning("[CollisionSystem] Correction distance too large: " + 
                std::to_string(correctionDistance) + " > " + std::to_string(config_.maxCorrectionDistance));
        }
    }

    return resolution;
}

void CollisionSystem::resolveMultiple(const std::vector<entities::Entity*>& entities, float deltaTime) {
    // Simple approach: resolve each entity individually
    // More sophisticated approaches could consider inter-entity dependencies
    for (auto* entity : entities) {
        if (entity && entity->isActive()) {
            resolve(entity, deltaTime);
        }
    }
}

void CollisionSystem::resolveAll(float deltaTime) {
    // This would require access to all entities
    // For now, caller should use resolveMultiple with the relevant entities
    Logger::instance().info("[CollisionSystem] resolveAll called - use resolveMultiple instead");
}

void CollisionSystem::enableCollisionEvents(entities::Entity* entityA, entities::Entity* entityB, bool enabled) {
    auto entityPair = makeEntityPair(entityA, entityB);
    
    if (enabled) {
        enabledEventPairs_.insert(entityPair);
    } else {
        enabledEventPairs_.erase(entityPair);
    }
}

void CollisionSystem::disableCollisionEvents(entities::Entity* entityA, entities::Entity* entityB) {
    enableCollisionEvents(entityA, entityB, false);
}

void CollisionSystem::resolveWithPhysics(entities::Entity* entityA, entities::Entity* entityB, 
                                        const CollisionResult& collision, float deltaTime) {
    // Basic physics resolution - could be expanded with mass, velocity, restitution, etc.
    sf::Vector2f mtv = calculateMinimumTranslationVector(
        entityA->getBounds(), entityB->getBounds());
    
    // For now, apply half the correction to each entity
    sf::Vector2f correctionA = mtv * -0.5f;
    sf::Vector2f correctionB = mtv * 0.5f;
    
    entityA->setPosition(entityA->position() + correctionA);
    entityB->setPosition(entityB->position() + correctionB);
    
    stats_.totalResolutions += 2; // Count both entities
}

bool CollisionSystem::sweepTest(entities::Entity* entity, const sf::Vector2f& velocity, float deltaTime, 
                               CollisionResult& firstHit) {
    if (!entity || !config_.enableContinuousDetection) return false;
    
    stats_.continuousDetectionTests++;
    
    auto sweepResults = manager_.sweepTest(entity->getBounds(), velocity, deltaTime, entity);
    
    if (!sweepResults.empty()) {
        firstHit = sweepResults[0]; // Return first collision
        firstHit.entityA = entity;
        return true;
    }
    
    return false;
}

void CollisionSystem::resetStats() {
    stats_ = Stats{};
}

CollisionResolution CollisionSystem::calculateResolution(entities::Entity* entity, const CollisionResult& collision) {
    CollisionResolution resolution;
    
    if (!entity || !collision.entityB) {
        return resolution;
    }
    
    // Calculate minimum translation vector
    sf::Vector2f mtv = calculateMinimumTranslationVector(
        entity->getBounds(), collision.entityB->getBounds());
    
    resolution.correction = mtv;
    resolution.normalizedNormal = collision.normal;
    resolution.penetrationDepth = std::sqrt(mtv.x * mtv.x + mtv.y * mtv.y);
    resolution.wasResolved = true;
    
    return resolution;
}

sf::Vector2f CollisionSystem::calculateMinimumTranslationVector(const sf::FloatRect& a, const sf::FloatRect& b) {
    // Calculate overlap in both axes
    float overlapX = std::min(a.position.x + a.size.x, b.position.x + b.size.x) - 
                     std::max(a.position.x, b.position.x);
    float overlapY = std::min(a.position.y + a.size.y, b.position.y + b.size.y) - 
                     std::max(a.position.y, b.position.y);
    
    // Choose the axis with minimum overlap for separation
    if (overlapX < overlapY) {
        // Separate along X axis
        if (a.position.x < b.position.x) {
            return sf::Vector2f(-overlapX, 0.f); // Move left
        } else {
            return sf::Vector2f(overlapX, 0.f); // Move right
        }
    } else {
        // Separate along Y axis
        if (a.position.y < b.position.y) {
            return sf::Vector2f(0.f, -overlapY); // Move up
        } else {
            return sf::Vector2f(0.f, overlapY); // Move down
        }
    }
}

bool CollisionSystem::shouldResolveCollision(entities::Entity* entity, entities::Entity* other) {
    // Don't resolve if either entity is inactive
    if (!entity->isActive() || !other->isActive()) {
        return false;
    }
    
    // Add more sophisticated logic here based on collision layers, entity types, etc.
    // For example: items might not need position resolution, only trigger events
    
    return true;
}

void CollisionSystem::updateCollisionEvents(const std::vector<CollisionResult>& collisions, float deltaTime) {
    std::unordered_set<std::pair<entities::Entity*, entities::Entity*>, EntityPairHash> currentCollisions;
    
    // Process current collisions
    for (const auto& collision : collisions) {
        if (!collision.entityA || !collision.entityB) continue;
        
        auto entityPair = makeEntityPair(collision.entityA, collision.entityB);
        currentCollisions.insert(entityPair);
        
        if (areEventsEnabled(collision.entityA, collision.entityB)) {
            bool wasColliding = previousCollisions_.find(entityPair) != previousCollisions_.end();
            manager_.getEventManager().updateCollisionStates(
                collision.entityA, collision.entityB, true, deltaTime);
                
            if (!wasColliding) {
                stats_.eventsTriggered++;
            }
        }
    }
    
    // Check for collisions that ended (OnExit events)
    for (const auto& previousPair : previousCollisions_) {
        if (currentCollisions.find(previousPair) == currentCollisions.end()) {
            // Collision ended
            if (areEventsEnabled(previousPair.first, previousPair.second)) {
                manager_.getEventManager().updateCollisionStates(
                    previousPair.first, previousPair.second, false, deltaTime);
                stats_.eventsTriggered++;
            }
        }
    }
    
    // Update previous collisions for next frame
    previousCollisions_ = std::move(currentCollisions);
}

std::pair<entities::Entity*, entities::Entity*> CollisionSystem::makeEntityPair(entities::Entity* a, entities::Entity* b) {
    // Ensure consistent ordering to avoid duplicate pairs (a,b) and (b,a)
    if (a->id() < b->id()) {
        return {a, b};
    } else {
        return {b, a};
    }
}

bool CollisionSystem::areEventsEnabled(entities::Entity* a, entities::Entity* b) const {
    auto entityPair = const_cast<CollisionSystem*>(this)->makeEntityPair(a, b);
    return enabledEventPairs_.find(entityPair) != enabledEventPairs_.end();
}

} // namespace collisions
