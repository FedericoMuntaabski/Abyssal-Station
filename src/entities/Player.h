#ifndef ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H
#define ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H

#include "Entity.h"
#include "MovementHelper.h"
#include "../gameplay/InventorySystem.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <algorithm>

namespace input { class InputManager; }
namespace collisions { class CollisionManager; }

namespace entities {

class Player : public Entity {
public:
    enum class State { Idle, Walking, Running, Dead };
    
    // Stealth states for survival mechanics
    enum class StealthState { Standing, Crouching, Slow };

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
    
    // NEW: Vital signs accessors
    float getBattery() const noexcept { return battery_; }
    float getFatigue() const noexcept { return fatigue_; }
    float getWeight() const noexcept { return weight_; }
    float getMaxWeight() const noexcept { return maxWeight_; }
    
    // NEW: Vision and direction
    float getRotation() const noexcept { return rotation_; }
    void setRotation(float angle) { rotation_ = angle; }
    sf::Vector2f getFacingDirection() const;
    bool isFlashlightOn() const noexcept { return flashlightOn_; }
    StealthState getStealthState() const noexcept { return stealthState_; }
    bool hasInjury() const noexcept { return hasInjury_; }
    int getAvailableSlots() const noexcept { return availableSlots_; }
    float getDetectability() const noexcept { return currentDetectability_; }

    // Setters for precise restoration from GameState
    void setHealth(int health);
    void setPosition(const sf::Vector2f& position) noexcept override; // Override Entity's setPosition to sync shape
    void setSpeed(float speed) noexcept { speed_ = speed; }
    void setState(State state) noexcept { state_ = state; }
    
    // NEW: Vital signs setters
    void setBattery(float battery) noexcept { battery_ = std::clamp(battery, 0.0f, 100.0f); }
    void setFatigue(float fatigue) noexcept { fatigue_ = std::clamp(fatigue, 0.0f, 100.0f); }
    void setWeight(float weight) noexcept { weight_ = std::max(0.0f, weight); }

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
    
    // NEW: Survival mechanics queries
    bool isRunning() const noexcept { return isRunning_; }
    bool isCrouching() const noexcept { return stealthState_ == StealthState::Crouching; }
    bool isMovingSlow() const noexcept { return stealthState_ == StealthState::Slow; }
    bool isOverloaded() const noexcept { return weight_ > maxWeight_ * 0.8f; }
    bool canRun() const noexcept { return fatigue_ > 10.0f && !isOverloaded(); } // Puede correr si tiene más de 10% de fatiga
    
    // NEW: Vital signs management
    void drainFatigue(float amount);
    void drainBattery(float amount);
    void recoverFatigue(float deltaTime);
    void rechargeBattery(float deltaTime);
    void calculateWeight();
    void updateVitalSigns(float deltaTime);
    
    // NEW: Stealth mechanics
    void setCrouching(bool crouching);
    void setMovingSlow(bool slow);
    void toggleFlashlight();
    void updateStealthState(float deltaTime);
    
    // NEW: Injury system
    void checkForCriticalInjury();
    void applyInjuryEffects();
    void startSlowRegeneration(float deltaTime);
    
    // NEW: Inventory management
    gameplay::InventorySystem* getInventory() { return &inventory_; }
    const gameplay::InventorySystem* getInventory() const { return &inventory_; }
    bool addItem(std::unique_ptr<gameplay::InventoryItem> item);
    bool removeItem(size_t slot);
    bool useCurrentItem();
    gameplay::InventoryItem* getCurrentWeapon() const;
    void selectHotbarSlot(size_t slot);
    
    // NEW: Combat system
    bool canAttack() const;
    int calculateAttackDamage() const;
    void performAttack();
    bool isAttackOnCooldown() const;
    
    // Debug support
    const sf::Vector2f& getVelocity() const { return velocity_; }
    float getDebugLogInterval() const { return debugLogInterval_; }
    void setDebugLogInterval(float interval) { debugLogInterval_ = interval; }

private:
    // Basic attributes
    float speed_;
    int health_;
    State state_;
    sf::Vector2f velocity_;
    sf::RectangleShape shape_;
    
    // NEW: Vision and direction
    float rotation_{0.0f};            // Facing angle in degrees (0 = right, 90 = down)
    
    // NEW: Vital signs system
    float battery_{100.0f};           // 0-100, recarga lenta automática
    float fatigue_{0.0f};             // 0-100, aumenta con acciones físicas
    float weight_{0.0f};              // Calculado dinámicamente según inventario
    float maxWeight_{20.0f};          // Capacidad máxima de peso
    
    // NEW: Stealth and movement states
    StealthState stealthState_{StealthState::Standing};
    bool flashlightOn_{true};         // Linterna encendida/apagada
    bool isRunning_{false};           // Si está corriendo actualmente
    float currentDetectability_{1.0f}; // Factor de detectabilidad (0.0-1.0)
    
    // NEW: Injury system
    bool hasInjury_{false};           // Si tiene heridas críticas
    float injuryThreshold_{35.0f};    // Umbral de vida para heridas críticas
    int availableSlots_{4};           // Slots de inventario disponibles
    float baseSpeed_;                 // Velocidad base antes de modificaciones
    
    // NEW: Timers for vital signs
    float batteryRegenTimer_{0.0f};
    float fatigueRecoveryTimer_{0.0f};
    float healthRegenTimer_{0.0f};
    float attackCooldownTimer_{0.0f};
    
    // NEW: Inventory system
    gameplay::InventorySystem inventory_;
    
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
