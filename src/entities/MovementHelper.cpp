#include "MovementHelper.h"
#include "Entity.h"
#include "../collisions/CollisionManager.h"
#include "../core/Logger.h"
#include <algorithm>
#include <cmath>

namespace entities {

using core::Logger;

MovementHelper::MovementResult MovementHelper::computeMovement(
    Entity* entity,
    const sf::Vector2f& intendedMove,
    collisions::CollisionManager* collisionManager,
    CollisionMode mode,
    int maxSteps
) {
    if (!entity || !collisionManager) {
        return {intendedMove, false, false, false, {0.f, 0.f}};
    }

    MovementResult result;
    result.finalPosition = entity->position();

    // If no movement intended, return current position
    sf::Vector2f moveVector = intendedMove - entity->position();
    if (std::abs(moveVector.x) < 0.001f && std::abs(moveVector.y) < 0.001f) {
        return result;
    }

    // Perform swept AABB collision detection
    result = sweptAABB(entity, entity->position(), intendedMove, collisionManager, maxSteps);

    // If collision occurred and mode is sliding, attempt to slide
    if (result.collisionOccurred && mode == CollisionMode::Slide && !result.wasBlocked) {
        sf::Vector2f remainingMove = intendedMove - result.finalPosition;
        if (std::abs(remainingMove.x) > 0.001f || std::abs(remainingMove.y) > 0.001f) {
            sf::Vector2f slideMove = computeSlideMovement(remainingMove, result.collisionNormal);
            
            // Test if slide movement is valid
            sf::Vector2f slideDestination = result.finalPosition + slideMove;
            sf::FloatRect slideBounds;
            slideBounds.position = slideDestination;
            slideBounds.size = entity->size();
            
            if (!collisionManager->firstColliderForBounds(slideBounds, entity)) {
                result.finalPosition = slideDestination;
                result.didSlide = true;
                Logger::instance().info("[MovementHelper] Entity id=" + std::to_string(entity->id()) + " slid along surface");
            }
        }
    }

    return result;
}

MovementHelper::MovementResult MovementHelper::sweptAABB(
    Entity* entity,
    const sf::Vector2f& from,
    const sf::Vector2f& to,
    collisions::CollisionManager* collisionManager,
    int steps
) {
    MovementResult result;
    result.finalPosition = from;

    if (steps <= 0) steps = 1;

    sf::Vector2f currentPos = from;
    sf::Vector2f totalMove = to - from;
    sf::Vector2f stepMove = totalMove / static_cast<float>(steps);

    for (int step = 0; step < steps; ++step) {
        sf::Vector2f nextPos = currentPos + stepMove;
        
        // Create bounds for next position
        sf::FloatRect bounds;
        bounds.position = nextPos;
        bounds.size = entity->size();

        // Check for collision
        auto* collider = collisionManager->firstColliderForBounds(bounds, entity);
        if (collider) {
            result.collisionOccurred = true;
            result.wasBlocked = true;
            
            // Extract collision normal for potential sliding
            sf::FloatRect colliderBounds;
            colliderBounds.position = collider->position();
            colliderBounds.size = collider->size();
            result.collisionNormal = extractCollisionNormal(bounds, colliderBounds);
            
            Logger::instance().info("[MovementHelper] Entity id=" + std::to_string(entity->id()) + 
                " collision detected at step " + std::to_string(step + 1) + "/" + std::to_string(steps));
            break;
        }

        currentPos = nextPos;
        result.finalPosition = currentPos;
    }

    return result;
}

sf::Vector2f MovementHelper::computeSlideMovement(
    const sf::Vector2f& intendedMove,
    const sf::Vector2f& collisionNormal
) {
    // Project the intended movement onto the surface (perpendicular to normal)
    // slideMove = intendedMove - (intendedMove · normal) * normal
    float dot = intendedMove.x * collisionNormal.x + intendedMove.y * collisionNormal.y;
    return intendedMove - (dot * collisionNormal);
}

sf::Vector2f MovementHelper::extractCollisionNormal(
    const sf::FloatRect& entityBounds,
    const sf::FloatRect& colliderBounds
) {
    // Simple collision normal extraction based on overlap direction
    sf::Vector2f normal(0.f, 0.f);

    // Calculate overlaps
    float leftOverlap = (entityBounds.position.x + entityBounds.size.x) - colliderBounds.position.x;
    float rightOverlap = (colliderBounds.position.x + colliderBounds.size.x) - entityBounds.position.x;
    float topOverlap = (entityBounds.position.y + entityBounds.size.y) - colliderBounds.position.y;
    float bottomOverlap = (colliderBounds.position.y + colliderBounds.size.y) - entityBounds.position.y;

    // Find the smallest overlap to determine collision direction
    float minOverlap = std::min({leftOverlap, rightOverlap, topOverlap, bottomOverlap});

    if (minOverlap == leftOverlap) {
        normal = {-1.f, 0.f}; // Collision from left
    } else if (minOverlap == rightOverlap) {
        normal = {1.f, 0.f};  // Collision from right
    } else if (minOverlap == topOverlap) {
        normal = {0.f, -1.f}; // Collision from top
    } else {
        normal = {0.f, 1.f};  // Collision from bottom
    }

    return normal;
}

} // namespace entities
