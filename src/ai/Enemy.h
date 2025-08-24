#ifndef ABYSSAL_STATION_SRC_AI_ENEMY_H
#define ABYSSAL_STATION_SRC_AI_ENEMY_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "entities/Entity.h"
#include "AIState.h"
#include "AISystem.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace entities { class Player; }
namespace collisions { class CollisionManager; }

namespace ai {

class Enemy : public entities::Entity {
public:
    using Id = entities::Entity::Id;

    Enemy(Id id = 0u,
          const sf::Vector2f& position = {0.f, 0.f},
          const sf::Vector2f& size = {32.f, 32.f},
          float speed = 100.f,
          float visionRange = 200.f,
          float attackRange = 24.f,
          const std::vector<sf::Vector2f>& patrolPoints = {},
          BehaviorProfile profile = BehaviorProfile::NEUTRAL);
    ~Enemy() override;

    // Game loop hooks
    void update(float deltaTime) override;
    void update(float deltaTime, const sf::Vector2f& playerPos);
    void render(sf::RenderWindow& window) override;

    // Enhanced AI integration
    void setAIAgent(std::unique_ptr<AIAgent> agent);
    AIAgent* getAIAgent() const { return aiAgent_.get(); }
    bool hasAIAgent() const { return aiAgent_ != nullptr; }

    // Legacy state control (for backward compatibility)
    void changeState(AIState newState);
    AIState getCurrentState() const;

    // Player target for detection/chase (optional)
    void setTargetPlayer(entities::Player* player) { targetPlayer_ = player; }

    // Configurable parameters
    void setSpeed(float s) { speed_ = s; }
    void setVisionRange(float r) { visionRange_ = r; }
    void setAttackRange(float r) { attackRange_ = r; }
    void setBehaviorProfile(BehaviorProfile profile);

    // Enhanced functionality
    void setPatrolPoints(const std::vector<sf::Vector2f>& points);
    void addPatrolPoint(const sf::Vector2f& point);
    const std::vector<sf::Vector2f>& getPatrolPoints() const { return patrolPoints_; }

    // Event handling
    void onDamageReceived(float damage, entities::Entity* source);
    void onSoundHeard(const sf::Vector2f& soundPosition, float intensity);

    // Legacy methods (for backward compatibility)
    bool detectPlayer(const sf::Vector2f& playerPos) const;
    bool detectPlayer() const;
    void moveTowards(const sf::Vector2f& dst, float dt);
    void moveTowards(const sf::Vector2f& dst, float dt, collisions::CollisionManager* collisionManager);
    
    // Intended movement API (for collision handling)
    sf::Vector2f computeIntendedMove(float deltaTime) const;
    void commitMove(const sf::Vector2f& newPosition);
    void performMovementPlanning(float deltaTime, collisions::CollisionManager* collisionManager);

    // Attack functionality
    void attack(entities::Player& player);
    void setAttackCooldown(float cd) { attackCooldown_ = cd; }

    // Debug visualization
    void setVisionColors(const sf::Color& fill, const sf::Color& outline) { visionFillColor_ = fill; visionOutlineColor_ = outline; }
    void setFacingDirection(const sf::Vector2f& dir) { facingDir_ = dir; }

private:
    // Legacy FSM for backward compatibility
    void runLegacyFSM(float deltaTime, const sf::Vector2f* playerPos);
    std::size_t findNearestPatrolIndex() const;
    static const char* legacyStateToString(AIState s);

    // Enhanced AI agent (optional)
    std::unique_ptr<AIAgent> aiAgent_;
    
    // Legacy state management
    AIState legacyState_{AIState::IDLE};
    float speed_;
    float visionRange_;
    float attackRange_;
    BehaviorProfile behaviorProfile_;

    std::vector<sf::Vector2f> patrolPoints_;
    std::size_t currentPatrolIndex_{0};

    // Raw pointer is fine here - scene owns player lifetime
    entities::Player* targetPlayer_{nullptr};
    // CollisionManager pointer (not owned) used for LOS checks and movement planning
    collisions::CollisionManager* collisionManager_{nullptr};

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
    
    // Mode selection
    bool useEnhancedAI_{false};
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_ENEMY_H
