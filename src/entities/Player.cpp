#include "Player.h"
#include "Entity.h"
#include <core/Logger.h>
#include "../input/InputManager.h"
#include "../input/Action.h"
#include <string>
#include <cmath>

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

void Player::applyDamage(int amount) {
    health_ -= amount;
    Logger::instance().info("[Player] id=" + std::to_string(id_) + " took " + std::to_string(amount) + " damage, health=" + std::to_string(health_));
    if (health_ <= 0) {
        health_ = 0;
        state_ = State::Dead;
        Logger::instance().info("[Player] id=" + std::to_string(id_) + " is dead");
    }
}

void Player::render(sf::RenderWindow& window) {
    window.draw(shape_);
}

void Player::onItemCollected(Id itemId) {
    inventoryCount_++;
    Logger::instance().info("[Player] Collected item id=" + std::to_string(itemId) + ", total=" + std::to_string(inventoryCount_));
}

} // namespace entities
