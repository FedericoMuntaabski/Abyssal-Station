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
    Logger::instance().info("Player: created id=" + std::to_string(id_));
    
    creationTime_ = std::chrono::steady_clock::now();
}

Player::~Player() {
    Logger::instance().info("Player: destroyed id=" + std::to_string(id_));
}

void Player::handleInput(input::InputManager& inputManager) {
    velocity_ = {0.f, 0.f};
    using input::Action;
    if (inputManager.isActionPressed(Action::MoveLeft))  velocity_.x -= 1.f;
    if (inputManager.isActionPressed(Action::MoveRight)) velocity_.x += 1.f;
    if (inputManager.isActionPressed(Action::MoveUp))    velocity_.y -= 1.f;
    if (inputManager.isActionPressed(Action::MoveDown))  velocity_.y += 1.f;

    // Determine new state and log if it changed
    State prev = state_;
    if (velocity_.x != 0.f || velocity_.y != 0.f) {
        state_ = State::Walking;
    } else {
        state_ = State::Idle;
    }
    if (prev != state_) {
        Logger::instance().info("[Player] id=" + std::to_string(id_) + " state=" + stateToString(state_));
    }
}

void Player::update(float deltaTime) {

    // Movement is now controlled externally via computeIntendedMove() and commitMove()
    // Keep position/shape syncing if other systems changed position
    shape_.setPosition(position_);

    // Rate-limited debug: log position and velocity every debugLogInterval_ seconds
    debugLogTimer_ += deltaTime;
    if (debugLogTimer_ >= debugLogInterval_) {
        debugLogTimer_ = 0.f;
        Logger::instance().info("[Player][DEBUG] id=" + std::to_string(id_) +
            " pos=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")" +
            " vel=(" + std::to_string(velocity_.x) + "," + std::to_string(velocity_.y) + ")");
    }
}

sf::Vector2f Player::computeIntendedMove(float deltaTime) const {
    if (velocity_.x == 0.f && velocity_.y == 0.f) return position_;
    sf::Vector2f v = velocity_;
    float len = std::sqrt(v.x*v.x + v.y*v.y);
    if (len > 0.f) v /= len;
    return position_ + v * speed_ * deltaTime;
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

} // namespace entities
