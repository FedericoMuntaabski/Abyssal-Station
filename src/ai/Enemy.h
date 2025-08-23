#ifndef ABYSSAL_STATION_SRC_AI_ENEMY_H
#define ABYSSAL_STATION_SRC_AI_ENEMY_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "entities/Entity.h"
#include <vector>
#include <cstdint>

namespace entities { class Player; }
namespace collisions { class CollisionManager; }

namespace ai {

// Initial simple AI states for enemies
enum class AIState { IDLE, PATROL, CHASE, ATTACK, RETURN };

class Enemy : public entities::Entity {
public:
    using Id = entities::Entity::Id;

    Enemy(Id id = 0u,
          const sf::Vector2f& position = {0.f, 0.f},
          const sf::Vector2f& size = {32.f, 32.f},
          float speed = 100.f,
          float visionRange = 200.f,
          float attackRange = 24.f,
          const std::vector<sf::Vector2f>& patrolPoints = {});
    ~Enemy() override;

    // Game loop hooks
    void update(float deltaTime) override;
        // Update using an external player position (used by managers)
        void update(float deltaTime, const sf::Vector2f& playerPos);
    void render(sf::RenderWindow& window) override;

    // State control
    void changeState(AIState newState);

    // Player target for detection/chase (optional)
    void setTargetPlayer(entities::Player* player) { targetPlayer_ = player; }

    // Configurable parameters
    void setSpeed(float s) { speed_ = s; }
    void setVisionRange(float r) { visionRange_ = r; }
    void setAttackRange(float r) { attackRange_ = r; }

private:
    // Returns true if the given player position is within visionRange_
        bool detectPlayer(const sf::Vector2f& playerPos) const;
    // Convenience overload that checks the stored targetPlayer_
    bool detectPlayer() const;
    void moveTowards(const sf::Vector2f& dst, float dt);
    // Collision-aware movement: will consult CollisionManager and attempt simple sliding
    void moveTowards(const sf::Vector2f& dst, float dt, collisions::CollisionManager* collisionManager);
    static const char* stateToString(AIState s);
    // Core FSM runner used internally
    void runFSM(float deltaTime, const sf::Vector2f* playerPos);
    // Find patrol point index nearest to current position
    std::size_t findNearestPatrolIndex() const;

    AIState state_{AIState::IDLE};
    float speed_;
    float visionRange_;
    float attackRange_;

    std::vector<sf::Vector2f> patrolPoints_;
    std::size_t currentPatrolIndex_{0};

    // Raw pointer is fine here - scene owns player lifetime
    entities::Player* targetPlayer_{nullptr};

    // Simple debug shape (not required for logic)
    sf::RectangleShape shape_;
    // Facing direction used for cone vision drawing
    sf::Vector2f facingDir_{1.f, 0.f};
    // Debug vision colors (configurable)
    sf::Color visionFillColor_{255, 0, 0, 40};
    sf::Color visionOutlineColor_{255, 0, 0, 120};
    // Logging cooldown so transitions aren't spammed
    float logCooldown_{0.5f};
    float logTimer_{0.f};
    // Attack cooldown to limit attack frequency
    float attackCooldown_{1.0f};
    float attackTimer_{0.f};
    // Intended movement API (compute but don't commit until collision checks)
    sf::Vector2f intendedPosition_{0.f, 0.f};
public:
    // Compute intended move given deltaTime (used by Collision checks)
    sf::Vector2f computeIntendedMove(float deltaTime) const;
    // Commit the move after collision checks
    void commitMove(const sf::Vector2f& newPosition);
    // Basic attack: reduce player's health if within attackRange
    void attack(entities::Player& player);
    // Configure attack cooldown (seconds)
    void setAttackCooldown(float cd) { attackCooldown_ = cd; }
    // Configure debug vision colors
    void setVisionColors(const sf::Color& fill, const sf::Color& outline) { visionFillColor_ = fill; visionOutlineColor_ = outline; }
    // Expose facing direction (useful for tests/tuning)
    void setFacingDirection(const sf::Vector2f& dir) { facingDir_ = dir; }
    // Plan movement using collision manager (will set intendedPosition_ appropriately)
    void performMovementPlanning(float deltaTime, collisions::CollisionManager* collisionManager);
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_ENEMY_H
