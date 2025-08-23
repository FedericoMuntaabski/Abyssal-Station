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
    if (velocity_.x != 0.f || velocity_.y != 0.f) {
        float len = std::sqrt(velocity_.x*velocity_.x + velocity_.y*velocity_.y);
        if (len > 0.f) velocity_ /= len;
        position_ += velocity_ * speed_ * deltaTime;
        shape_.setPosition(position_);
    }

    // Rate-limited debug: log position and velocity every debugLogInterval_ seconds
    debugLogTimer_ += deltaTime;
    if (debugLogTimer_ >= debugLogInterval_) {
        debugLogTimer_ = 0.f;
        Logger::instance().info("[Player][DEBUG] id=" + std::to_string(id_) +
            " pos=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")" +
            " vel=(" + std::to_string(velocity_.x) + "," + std::to_string(velocity_.y) + ")");
    }
}

void Player::render(sf::RenderWindow& window) {
    window.draw(shape_);
}

} // namespace entities
