#ifndef ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H
#define ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H

#include "Entity.h"
#include "MovementHelper.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

namespace input { class InputManager; }
namespace collisions { class CollisionManager; }

namespace entities {

class Player : public Entity {
public:
    enum class State { Idle, Walking, Dead };

    Player(Id id = 0u, const sf::Vector2f& position = {0.f,0.f}, const sf::Vector2f& size = {32.f,32.f}, float speed = 200.f, int health = 100);
    ~Player() override;

    // Process input and update internal velocity
    void handleInput(input::InputManager& inputManager);

    // Entity overrides
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    // Intended-movement API: compute target position without applying, and apply if allowed
    sf::Vector2f computeIntendedMove(float deltaTime) const;
    void commitMove(const sf::Vector2f& newPosition);

    // Advanced movement with collision handling
    MovementHelper::MovementResult computeAdvancedMove(float deltaTime, collisions::CollisionManager* collisionManager, MovementHelper::CollisionMode mode = MovementHelper::CollisionMode::Block);
    void commitAdvancedMove(const MovementHelper::MovementResult& moveResult);

    // Accessors
    float speed() const noexcept { return speed_; }
    int health() const noexcept { return health_; }
    State state() const noexcept { return state_; }
    int inventoryCount() const noexcept { return inventoryCount_; }

    // Setters for precise restoration from GameState
    void setHealth(int health);
    void setPosition(const sf::Vector2f& position) noexcept override; // Override Entity's setPosition to sync shape
    void setSpeed(float speed) noexcept { speed_ = speed; }
    void setState(State state) noexcept { state_ = state; }

    // Apply damage to the player
    void applyDamage(int amount);

    // Called when player collects an item
    void onItemCollected(Id itemId);

    // Gameplay integration
    void onPuzzleSolved(Id puzzleId);
    void onEnemyEncounter(Id enemyId);
    
    // Movement configuration
    void setTrackMovement(bool track) { trackMovement_ = track; }
    bool isTrackingMovement() const { return trackMovement_; }
    
    // Advanced state queries
    bool isDead() const { return state_ == State::Dead; }
    bool isMoving() const { return state_ == State::Walking; }
    bool canMove() const { return !isDead(); }
    bool canInteract() const { return !isDead(); }
    
    // Debug support
    const sf::Vector2f& getVelocity() const { return velocity_; }
    float getDebugLogInterval() const { return debugLogInterval_; }
    void setDebugLogInterval(float interval) { debugLogInterval_ = interval; }

private:
    float speed_;
    int health_;
    State state_;
    sf::Vector2f velocity_;
    sf::RectangleShape shape_;
    // Debug logging: interval (seconds) and accumulator
    float debugLogInterval_{0.5f};
    float debugLogTimer_{0.f};
    int inventoryCount_{0};
    // Telemetry tracking
    std::chrono::steady_clock::time_point creationTime_;
    sf::Vector2f lastPosition_;
    bool trackMovement_{true};
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H
