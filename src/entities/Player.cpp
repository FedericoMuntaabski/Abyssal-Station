#include "Player.h"
#include "Entity.h"
#include "MovementHelper.h"
#include <core/Logger.h>
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../collisions/CollisionManager.h"
#include <string>
#include <cmath>
#include <chrono>

namespace entities {

using core::Logger;

static const char* stateToString(Player::State s) {
    switch (s) {
        case Player::State::Idle: return "Idle";
        case Player::State::Walking: return "Walking";
        case Player::State::Running: return "Running";
        case Player::State::Dead: return "Dead";
        default: return "Unknown";
    }
}

Player::Player(Id id, const sf::Vector2f& position, const sf::Vector2f& size, float speed, int health)
    : Entity(id, position, size), speed_(speed), health_(health), state_(State::Idle), velocity_(0.f,0.f) {
    // ensure player sits on Player collision layer
    setCollisionLayer(Layer::Player);
    shape_.setSize(size_);
    shape_.setFillColor(sf::Color::Blue);
    shape_.setPosition(position_);
    
    // Initialize survival system
    baseSpeed_ = speed_;
    battery_ = 100.0f;  // Inicia en 100%
    fatigue_ = 100.0f;  // Inicia en 100% (máxima energía)
    weight_ = 0.0f;     // Inicia sin peso
    stealthState_ = StealthState::Standing;
    flashlightOn_ = true;
    isRunning_ = false;
    currentDetectability_ = 1.0f;
    hasInjury_ = false;
    availableSlots_ = 4;
    
    Logger::instance().info("Player: created id=" + std::to_string(id_) + " with survival systems");
    
    creationTime_ = std::chrono::steady_clock::now();
}

Player::~Player() {
    Logger::instance().info("Player: destroyed id=" + std::to_string(id_));
}

void Player::handleInput(input::InputManager& inputManager) {
    velocity_ = {0.f, 0.f};
    using input::Action;
    
    // Handle survival actions first
    if (inputManager.isActionPressed(Action::ToggleFlashlight)) {
        toggleFlashlight();
    }
    
    // Handle crouch action
    bool crouchPressed = inputManager.isActionPressed(Action::Crouch);
    setCrouching(crouchPressed);
    
    // Handle hotbar selection
    if (inputManager.isActionPressed(Action::HotbarSlot1)) selectHotbarSlot(0);
    if (inputManager.isActionPressed(Action::HotbarSlot2)) selectHotbarSlot(1);
    if (inputManager.isActionPressed(Action::HotbarSlot3)) selectHotbarSlot(2);
    if (inputManager.isActionPressed(Action::HotbarSlot4)) selectHotbarSlot(3);
    
    // Handle attack action
    if (inputManager.isActionPressed(Action::Attack)) {
        useCurrentItem(); // This will handle attacking if current item is a weapon
    }
    
    // Movement input
    sf::Vector2f moveDirection{0.f, 0.f};
    if (inputManager.isActionPressed(Action::MoveLeft))  moveDirection.x -= 1.f;
    if (inputManager.isActionPressed(Action::MoveRight)) moveDirection.x += 1.f;
    if (inputManager.isActionPressed(Action::MoveUp))    moveDirection.y -= 1.f;
    if (inputManager.isActionPressed(Action::MoveDown))  moveDirection.y += 1.f;
    
    velocity_ = moveDirection;
    
    // Update facing direction based on movement
    if (moveDirection.x != 0.f || moveDirection.y != 0.f) {
        rotation_ = std::atan2(moveDirection.y, moveDirection.x) * 180.0f / 3.14159265f;
    }
    
    // Check if running with Shift key
    bool runPressed = inputManager.isActionPressed(Action::Run);
    isRunning_ = (velocity_.x != 0.f || velocity_.y != 0.f) && runPressed && !isCrouching() && canRun();
    
    // Set movement slow if crouching or choose slow movement
    if (isCrouching()) {
        setMovingSlow(true);
    } else {
        setMovingSlow(false);
    }

    // Determine new state and log if it changed
    State prev = state_;
    if (velocity_.x != 0.f || velocity_.y != 0.f) {
        if (isRunning_) {
            state_ = State::Running;
        } else {
            state_ = State::Walking;
        }
    } else {
        state_ = State::Idle;
    }
    if (prev != state_) {
        Logger::instance().info("[Player] id=" + std::to_string(id_) + " state=" + stateToString(state_));
    }
}

void Player::update(float deltaTime) {
    // Update vital signs and survival mechanics
    updateVitalSigns(deltaTime);
    updateStealthState(deltaTime);
    checkForCriticalInjury();
    
    if (hasInjury_) {
        startSlowRegeneration(deltaTime);
    }

    // Movement is now controlled externally via computeIntendedMove() and commitMove()
    // Keep position/shape syncing if other systems changed position
    shape_.setPosition(position_);

    // Rate-limited debug: log position and velocity every debugLogInterval_ seconds
    debugLogTimer_ += deltaTime;
    if (debugLogTimer_ >= debugLogInterval_) {
        debugLogTimer_ = 0.f;
        Logger::instance().info("[Player][DEBUG] id=" + std::to_string(id_) +
            " pos=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")" +
            " vel=(" + std::to_string(velocity_.x) + "," + std::to_string(velocity_.y) + ")" +
            " health=" + std::to_string(health_) + 
            " battery=" + std::to_string(battery_) + 
            " fatigue=" + std::to_string(fatigue_) +
            " weight=" + std::to_string(weight_));
    }
}

sf::Vector2f Player::computeIntendedMove(float deltaTime) const {
    if (velocity_.x == 0.f && velocity_.y == 0.f) return position_;
    
    sf::Vector2f v = velocity_;
    float len = std::sqrt(v.x*v.x + v.y*v.y);
    if (len > 0.f) v /= len;
    
    // Calculate effective speed based on state
    float effectiveSpeed = speed_;
    
    // Apply stealth speed modifications
    if (isCrouching()) {
        effectiveSpeed *= 0.5f; // 50% speed when crouching
    } else if (isMovingSlow()) {
        effectiveSpeed *= 0.7f; // 70% speed when moving slow
    }
    
    // Apply weight penalty
    if (isOverloaded()) {
        effectiveSpeed *= 0.8f; // 20% speed reduction when overloaded
    }
    
    // Apply running bonus (if not fatigued)
    if (isRunning_ && canRun()) {
        effectiveSpeed *= 1.2f; // 20% speed bonus when running
    }
    
    return position_ + v * effectiveSpeed * deltaTime;
}

void Player::commitMove(const sf::Vector2f& newPosition) {
    position_ = newPosition;
    shape_.setPosition(position_);
}

MovementHelper::MovementResult Player::computeAdvancedMove(float deltaTime, collisions::CollisionManager* collisionManager, MovementHelper::CollisionMode mode) {
    sf::Vector2f intendedPosition = computeIntendedMove(deltaTime);
    return MovementHelper::computeMovement(this, intendedPosition, collisionManager, mode);
}

void Player::commitAdvancedMove(const MovementHelper::MovementResult& moveResult) {
    sf::Vector2f oldPosition = position_;
    position_ = moveResult.finalPosition;
    shape_.setPosition(position_);
    
    // Log collision details
    if (moveResult.collisionOccurred) {
        if (moveResult.didSlide) {
            Logger::instance().info("[Player] id=" + std::to_string(id_) + " slid to position (" + 
                std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
        } else if (moveResult.wasBlocked) {
            Logger::instance().info("[Player] id=" + std::to_string(id_) + " movement blocked at (" + 
                std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
        }
    }
}

void Player::applyDamage(int amount) {
    health_ -= amount;
    Logger::instance().info("[Player] id=" + std::to_string(id_) + " took " + std::to_string(amount) + " damage, health=" + std::to_string(health_));
    if (health_ <= 0) {
        health_ = 0;
        state_ = State::Dead;
        Logger::instance().info("[Player] id=" + std::to_string(id_) + " is dead");
    }
}

void Player::setHealth(int health) {
    int oldHealth = health_;
    health_ = std::max(0, health); // Ensure health doesn't go negative
    if (oldHealth != health_) {
        Logger::instance().info("[Player] id=" + std::to_string(id_) + " health changed from " + std::to_string(oldHealth) + " to " + std::to_string(health_));
        
        // Update state based on new health
        if (health_ <= 0) {
            state_ = State::Dead;
            Logger::instance().info("[Player] id=" + std::to_string(id_) + " died due to health restoration");
        } else if (state_ == State::Dead && health_ > 0) {
            state_ = State::Idle;
            Logger::instance().info("[Player] id=" + std::to_string(id_) + " revived from dead state");
        }
    }
}

void Player::setPosition(const sf::Vector2f& position) noexcept {
    Entity::setPosition(position);
    shape_.setPosition(position_);
    Logger::instance().info("[Player] id=" + std::to_string(id_) + " position set to (" + std::to_string(position.x) + "," + std::to_string(position.y) + ")");
}

void Player::render(sf::RenderWindow& window) {
    window.draw(shape_);
}

void Player::onItemCollected(Id itemId) {
    inventoryCount_++;
    Logger::instance().info("[Player] Collected item id=" + std::to_string(itemId) + ", total=" + std::to_string(inventoryCount_));
}

void Player::onPuzzleSolved(Id puzzleId) {
    Logger::instance().info("[Player] Solved puzzle id=" + std::to_string(puzzleId));
}

void Player::onEnemyEncounter(Id enemyId) {
    Logger::instance().info("[Player] Encountered enemy id=" + std::to_string(enemyId));
}

// NEW: Vital signs management methods
void Player::updateVitalSigns(float deltaTime) {
    // Update timers
    batteryRegenTimer_ += deltaTime;
    fatigueRecoveryTimer_ += deltaTime;
    healthRegenTimer_ += deltaTime;
    attackCooldownTimer_ += deltaTime;
    
    // Battery stays at 100% by default
    battery_ = 100.0f;
    
    // Drain battery if flashlight is on (2 points per minute)
    if (flashlightOn_) {
        drainBattery(2.0f / 60.0f * deltaTime);
    }
    
    // Fatigue regeneration - always recovering gradually (2 points per second) when not at max
    if (fatigueRecoveryTimer_ >= 0.5f) { // Check every 0.5 seconds
        fatigueRecoveryTimer_ = 0.0f;
        if (fatigue_ < 100.0f) {
            recoverFatigue(deltaTime);
        }
    }
    
    // Drain fatigue ONLY when running (not walking)
    if (isRunning_) {
        drainFatigue(5.0f * deltaTime); // 5 points per second when running
        
        // Extra fatigue from weight
        float weightFactor = weight_ / 10.0f; // +1 point per 10kg
        drainFatigue(weightFactor * deltaTime);
    }
    
    // Calculate current weight
    calculateWeight();
}

void Player::drainFatigue(float amount) {
    fatigue_ = std::max(0.0f, fatigue_ - amount); // Fatiga BAJA cuando se drena
}

void Player::drainBattery(float amount) {
    battery_ = std::max(0.0f, battery_ - amount);
    if (battery_ <= 0.0f && flashlightOn_) {
        flashlightOn_ = false; // Auto-turn off when battery depleted
        Logger::instance().info("[Player] Flashlight turned off - battery depleted");
    }
}

void Player::recoverFatigue(float deltaTime) {
    if (fatigue_ < 100.0f) {
        fatigue_ = std::min(100.0f, fatigue_ + 2.0f * deltaTime); // Fatiga SUBE cuando se recupera
    }
}

void Player::rechargeBattery(float deltaTime) {
    battery_ = std::min(100.0f, battery_ + 1.0f * deltaTime);
}

void Player::calculateWeight() {
    // Use the actual inventory system weight calculation
    weight_ = inventory_.getCurrentWeight();
}

// NEW: Stealth mechanics methods
void Player::setCrouching(bool crouching) {
    StealthState newState = crouching ? StealthState::Crouching : StealthState::Standing;
    if (stealthState_ != newState) {
        stealthState_ = newState;
        Logger::instance().info("[Player] Stealth state changed to " + 
                              std::string(crouching ? "Crouching" : "Standing"));
    }
}

void Player::setMovingSlow(bool slow) {
    if (slow && stealthState_ != StealthState::Crouching) {
        stealthState_ = StealthState::Slow;
    } else if (!slow && stealthState_ == StealthState::Slow) {
        stealthState_ = StealthState::Standing;
    }
}

void Player::toggleFlashlight() {
    if (battery_ > 0.0f) {
        flashlightOn_ = !flashlightOn_;
        Logger::instance().info("[Player] Flashlight " + std::string(flashlightOn_ ? "ON" : "OFF"));
    } else {
        Logger::instance().info("[Player] Cannot turn on flashlight - no battery");
    }
}

void Player::updateStealthState(float deltaTime) {
    // Calculate detectability based on current state
    float detectability = 1.0f;
    
    if (isCrouching()) {
        detectability *= 0.3f; // 70% less visible when crouching
    }
    
    if (isMovingSlow()) {
        detectability *= 0.5f; // 50% less detectable when moving slow
    }
    
    if (!flashlightOn_) {
        detectability *= 0.2f; // 80% less visible in darkness
    }
    
    currentDetectability_ = detectability;
}

// NEW: Injury system methods
void Player::checkForCriticalInjury() {
    if (health_ <= injuryThreshold_ && !hasInjury_) {
        hasInjury_ = true;
        applyInjuryEffects();
        Logger::instance().info("[Player] Critical injury sustained - applying permanent effects");
    }
}

void Player::applyInjuryEffects() {
    if (hasInjury_) {
        // Reduce speed by 25% (cojera)
        speed_ = baseSpeed_ * 0.75f;
        
        // Reduce available inventory slots to 2
        availableSlots_ = 2;
        
        Logger::instance().info("[Player] Injury effects applied: speed reduced, slots limited to 2");
    }
}

void Player::startSlowRegeneration(float deltaTime) {
    if (hasInjury_ && health_ < 50) {
        healthRegenTimer_ += deltaTime;
        if (healthRegenTimer_ >= 30.0f) { // 1 point every 30 seconds
            healthRegenTimer_ = 0.0f;
            health_ = std::min(50, health_ + 1); // Only regenerate up to 50%
        }
    }
}

// Only implement getFacingDirection - rotation accessors are inline in header
sf::Vector2f Player::getFacingDirection() const {
    float radians = rotation_ * 3.14159265f / 180.0f;
    return sf::Vector2f(std::cos(radians), std::sin(radians));
}

// NEW: Inventory management methods
bool Player::addItem(std::unique_ptr<gameplay::InventoryItem> item) {
    if (inventory_.addItem(std::move(item))) {
        calculateWeight(); // Recalculate weight after adding item
        Logger::instance().info("[Player] Item added to inventory");
        return true;
    }
    return false;
}

bool Player::removeItem(size_t slot) {
    auto item = inventory_.removeItem(slot);
    if (item) {
        calculateWeight(); // Recalculate weight after removing item
        Logger::instance().info("[Player] Item removed from inventory slot " + std::to_string(slot));
        return true;
    }
    return false;
}

bool Player::useCurrentItem() {
    size_t currentSlot = inventory_.getCurrentHotbarSlot();
    auto item = inventory_.getHotbarItem(currentSlot);
    
    if (!item) return false;
    
    // Handle different item types
    switch (item->type) {
        case gameplay::InventoryItem::Type::Weapon:
            if (canAttack()) {
                performAttack();
                return true;
            }
            break;
        case gameplay::InventoryItem::Type::Consumable:
            // Future: Handle consumables (healing items, etc.)
            inventory_.useItem(currentSlot);
            calculateWeight();
            return true;
        default:
            // Default use behavior
            inventory_.useItem(currentSlot);
            calculateWeight();
            return true;
    }
    
    return false;
}

gameplay::InventoryItem* Player::getCurrentWeapon() const {
    return inventory_.getCurrentWeapon();
}

void Player::selectHotbarSlot(size_t slot) {
    inventory_.selectHotbarSlot(slot);
    Logger::instance().info("[Player] Selected hotbar slot " + std::to_string(slot));
}

// NEW: Combat system methods
bool Player::canAttack() const {
    return !isDead() && 
           !isAttackOnCooldown() && 
           fatigue_ >= 15.0f && // Requires at least 15% fatigue
           getCurrentWeapon() != nullptr;
}

int Player::calculateAttackDamage() const {
    auto weapon = getCurrentWeapon();
    if (!weapon) return 0;
    
    // Use weapon manager to calculate damage
    float criticalChance = 0.1f; // 10% base critical chance
    
    // Fatigue affects critical chance
    if (fatigue_ > 80.0f) {
        criticalChance += 0.1f; // +10% when well rested
    }
    
    return gameplay::WeaponManager::calculateDamage(*weapon, criticalChance);
}

void Player::performAttack() {
    if (!canAttack()) return;
    
    auto weapon = getCurrentWeapon();
    if (!weapon) return;
    
    // Drain fatigue for attack
    drainFatigue(15.0f);
    
    // Damage the weapon
    auto stats = gameplay::WeaponManager::getWeaponStats(*weapon);
    size_t currentSlot = inventory_.getCurrentHotbarSlot();
    inventory_.damageItem(currentSlot, stats.durabilityLoss);
    
    // Set attack cooldown
    attackCooldownTimer_ = 0.0f;
    
    // Calculate and log damage
    int damage = calculateAttackDamage();
    Logger::instance().info("[Player] Performed attack with " + weapon->name + " for " + std::to_string(damage) + " damage");
}

bool Player::isAttackOnCooldown() const {
    auto weapon = getCurrentWeapon();
    if (!weapon) return true;
    
    auto stats = gameplay::WeaponManager::getWeaponStats(*weapon);
    return attackCooldownTimer_ < stats.attackSpeed;
}

} // namespace entities
