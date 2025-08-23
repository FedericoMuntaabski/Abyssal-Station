#include "Puzzle.h"
#include "../entities/Player.h"
#include "../core/Logger.h"

using namespace gameplay;

Puzzle::Puzzle(entities::Entity::Id id,
               const sf::Vector2f& position,
               const sf::Vector2f& size,
               const std::vector<std::string>& steps)
    : entities::Entity(id, position, size)
    , steps_(steps)
    , completedSteps_(steps.size(), false)
    , state_(steps.empty() ? PuzzleState::Completed : PuzzleState::Locked)
    , shape_(size)
{
    shape_.setPosition(position_);
    shape_.setFillColor(sf::Color(100, 100, 255, 200));
    
    // Bounds checking - warn if puzzle is placed outside reasonable bounds
    constexpr float MAX_COORDINATE = 10000.f;
    if (position_.x < -MAX_COORDINATE || position_.x > MAX_COORDINATE ||
        position_.y < -MAX_COORDINATE || position_.y > MAX_COORDINATE) {
        core::Logger::instance().warning(std::string("Puzzle created outside reasonable bounds: id=") + std::to_string(id) + 
                                        ", position=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
    }
    
    core::Logger::instance().info(std::string("Puzzle created: id=") + std::to_string(id) + 
                                 ", steps=" + std::to_string(steps.size()) + 
                                 ", position=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
}

Puzzle::~Puzzle() = default;

void Puzzle::update(float deltaTime)
{
    // If locked but players are interacting, transition to Active
    if (state_ == PuzzleState::Locked && !interactingPlayers_.empty()) {
        state_ = PuzzleState::Active;
        core::Logger::instance().info(std::string("Puzzle activated: id=") + std::to_string(id_));
    }

    // Simple tick: if active, periodically check completion
    if (state_ == PuzzleState::Active) {
        if (checkCompletion()) {
            state_ = PuzzleState::Completed;
            core::Logger::instance().info(std::string("Puzzle completed: id=") + std::to_string(id_));
        }
    }
}

void Puzzle::render(sf::RenderWindow& window)
{
    // Visual hint: greyed out when locked, bright when active, green when completed
    if (state_ == PuzzleState::Locked) {
        shape_.setFillColor(sf::Color(80, 80, 120, 180));
    } else if (state_ == PuzzleState::Active) {
        shape_.setFillColor(sf::Color(120, 120, 255, 220));
    } else {
        shape_.setFillColor(sf::Color(100, 220, 100, 240));
    }
    shape_.setPosition(position_);
    window.draw(shape_);
}

bool Puzzle::markStepCompleted(std::size_t stepIndex)
{
    if (stepIndex >= completedSteps_.size()) {
        core::Logger::instance().warning(std::string("Invalid step index for puzzle: id=") + std::to_string(id_) + 
                                         ", stepIndex=" + std::to_string(stepIndex) + 
                                         ", totalSteps=" + std::to_string(completedSteps_.size()));
        return false;
    }
    if (completedSteps_[stepIndex]) {
        core::Logger::instance().info(std::string("Step already completed for puzzle: id=") + std::to_string(id_) + 
                                     ", stepIndex=" + std::to_string(stepIndex));
        return false; // already done
    }
    completedSteps_[stepIndex] = true;
    
    std::string stepName = stepIndex < steps_.size() ? steps_[stepIndex] : "Unknown";
    core::Logger::instance().info(std::string("Puzzle step completed: puzzle=") + std::to_string(id_) + 
                                 ", step=" + std::to_string(stepIndex) + 
                                 ", stepName=" + stepName + 
                                 ", position=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
    return checkCompletion();
}

bool Puzzle::checkCompletion()
{
    for (bool s : completedSteps_) {
        if (!s) return false;
    }
    return true;
}

void Puzzle::playerJoin(entities::Player* player)
{
    if (!player) return;
    interactingPlayers_.push_back(player);
}

void Puzzle::playerLeave(entities::Player* player)
{
    if (!player) return;
    interactingPlayers_.erase(std::remove(interactingPlayers_.begin(), interactingPlayers_.end(), player), interactingPlayers_.end());
}
