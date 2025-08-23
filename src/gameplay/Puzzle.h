#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLE_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLE_H

#include "../entities/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <vector>
#include <string>

namespace entities { class Player; }

namespace gameplay {

enum class PuzzleState { Locked, Active, Completed };

class Puzzle : public entities::Entity {
public:
    // steps are human-readable identifiers for each sub-goal
    Puzzle(entities::Entity::Id id = 0u,
           const sf::Vector2f& position = {0.f,0.f},
           const sf::Vector2f& size = {64.f, 64.f},
           const std::vector<std::string>& steps = {});
    ~Puzzle() override;

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    // Mark a step complete (safe if already completed). Returns true if this caused puzzle completion.
    bool markStepCompleted(std::size_t stepIndex);

    // Check if all steps are complete and update state accordingly
    bool checkCompletion();

    PuzzleState state() const noexcept { return state_; }

    // Players can join/leave the puzzle interaction (multiplayer support)
    void playerJoin(entities::Player* player);
    void playerLeave(entities::Player* player);

private:
    std::vector<std::string> steps_;
    std::vector<bool> completedSteps_;
    PuzzleState state_;
    std::vector<entities::Player*> interactingPlayers_;

    sf::RectangleShape shape_;
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLE_H
