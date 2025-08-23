#ifndef ABYSSAL_STATION_SRC_ENTITIES_MOVEMENTHELPER_H
#define ABYSSAL_STATION_SRC_ENTITIES_MOVEMENTHELPER_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace collisions { class CollisionManager; }

namespace entities {

class Entity;

/**
 * Advanced movement helper for robust collision detection and sliding.
 * Provides swept-AABB collision detection to prevent tunneling and
 * implements sliding along surfaces instead of complete blocking.
 */
class MovementHelper {
public:
    struct MovementResult {
        sf::Vector2f finalPosition;
        bool collisionOccurred{false};
        bool wasBlocked{false};      // True if movement was completely blocked
        bool didSlide{false};        // True if sliding movement was applied
        sf::Vector2f collisionNormal{0.f, 0.f}; // Normal of the collision surface
    };

    enum class CollisionMode {
        Block,    // Complete blocking (current behavior)
        Slide,    // Slide along collision surfaces
        Bounce    // Bounce off collision surfaces (for future use)
    };

    /**
     * Perform advanced movement with collision detection.
     * @param entity The entity to move
     * @param intendedMove The desired destination position
     * @param collisionManager The collision manager to check against
     * @param mode How to handle collisions (block, slide, bounce)
     * @param maxSteps Maximum subdivisions for swept collision (default 4)
     * @return MovementResult with final position and collision info
     */
    static MovementResult computeMovement(
        Entity* entity,
        const sf::Vector2f& intendedMove,
        collisions::CollisionManager* collisionManager,
        CollisionMode mode = CollisionMode::Block,
        int maxSteps = 4
    );

private:
    /**
     * Perform swept AABB collision detection to prevent tunneling.
     * Subdivides movement into smaller steps if collision is detected.
     */
    static MovementResult sweptAABB(
        Entity* entity,
        const sf::Vector2f& from,
        const sf::Vector2f& to,
        collisions::CollisionManager* collisionManager,
        int steps
    );

    /**
     * Compute sliding movement along collision surface.
     */
    static sf::Vector2f computeSlideMovement(
        const sf::Vector2f& intendedMove,
        const sf::Vector2f& collisionNormal
    );

    /**
     * Extract collision normal from collision bounds.
     * Simple implementation that estimates normal based on overlap.
     */
    static sf::Vector2f extractCollisionNormal(
        const sf::FloatRect& entityBounds,
        const sf::FloatRect& colliderBounds
    );
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_MOVEMENTHELPER_H
