#ifndef ABYSSAL_STATION_SRC_ENTITIES_PLAYERCONTROLLER_H
#define ABYSSAL_STATION_SRC_ENTITIES_PLAYERCONTROLLER_H

#include "Player.h"
#include "MovementHelper.h"
#include "EntityDebug.h"
#include "../input/InputManager.h"
#include "../collisions/CollisionManager.h"

namespace entities {

/**
 * High-level controller for player entity that integrates all improvements.
 * Demonstrates best practices for using the enhanced entities module.
 */
class PlayerController {
public:
    struct Config {
        MovementHelper::CollisionMode movementMode{MovementHelper::CollisionMode::Slide};
        bool enableTelemetry{true};
        bool enableDebugVisualization{false};
        float debugLogInterval{1.0f};
    };

    PlayerController(Player* player, collisions::CollisionManager* collisionManager);
    ~PlayerController() = default;

    // Configuration
    void setConfig(const Config& config);
    const Config& getConfig() const { return config_; }

    // Main update loop
    void update(float deltaTime, input::InputManager& inputManager);
    
    // Rendering
    void render(sf::RenderWindow& window);
    void renderDebug(sf::RenderWindow& window);

    // State management
    void saveState(core::GameState::PlayerState& state) const;
    void loadState(const core::GameState::PlayerState& state);

    // Advanced features
    void enableSliding(bool enable);
    void enableDebugVisualization(bool enable);
    void exportTelemetryData(const std::string& filename) const;

    // Event callbacks
    void onItemCollected(Entity::Id itemId);
    void onPuzzleSolved(Entity::Id puzzleId);
    void onEnemyEncounter(Entity::Id enemyId);

private:
    Player* player_;
    collisions::CollisionManager* collisionManager_;
    Config config_;
    MovementHelper::MovementResult lastMoveResult_;
    
    // Statistics tracking
    float totalDistanceMoved_{0.f};
    std::uint32_t collisionCount_{0.f};
    std::uint32_t slideCount_{0.f};

    void updateMovement(float deltaTime);
    void updateStatistics(const MovementHelper::MovementResult& result);
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_PLAYERCONTROLLER_H
