#include "PlayerController.h"
#include "EntityTelemetry.h"
#include "EntityDebug.h"
#include "../core/Logger.h"
#include "../core/GameState.h"
#include <cmath>

namespace entities {

using core::Logger;

PlayerController::PlayerController(Player* player, collisions::CollisionManager* collisionManager)
    : player_(player), collisionManager_(collisionManager) {
    if (!player_) {
        Logger::instance().error("[PlayerController] Player pointer is null");
        return;
    }
    if (!collisionManager_) {
        Logger::instance().warning("[PlayerController] CollisionManager pointer is null - movement collision detection disabled");
    }
    
    Logger::instance().info("[PlayerController] Created for player id=" + std::to_string(player_->id()));
}

void PlayerController::setConfig(const Config& config) {
    config_ = config;
    
    if (player_) {
        player_->setDebugLogInterval(config_.debugLogInterval);
        player_->setTrackMovement(config_.enableTelemetry);
    }
    
    if (config_.enableDebugVisualization) {
        auto& debug = getEntityDebugInstance();
        debug.setEnabled(true);
        debug.setShowBounds(true);
        debug.setShowIds(true);
        debug.setShowHealth(true);
    }
    
    EntityTelemetry::instance().setEnabled(config_.enableTelemetry);
    
    Logger::instance().info("[PlayerController] Configuration updated");
}

void PlayerController::update(float deltaTime, input::InputManager& inputManager) {
    if (!player_ || player_->isDead()) return;

    // Handle input
    player_->handleInput(inputManager);
    
    // Update movement
    updateMovement(deltaTime);
    
    // Regular entity update
    player_->update(deltaTime);
}

void PlayerController::render(sf::RenderWindow& window) {
    if (!player_) return;
    
    player_->render(window);
    
    if (config_.enableDebugVisualization) {
        renderDebug(window);
    }
}

void PlayerController::renderDebug(sf::RenderWindow& window) {
    if (!player_) return;
    
    auto& debug = getEntityDebugInstance();
    debug.renderEntityDebug(window, player_);
}

void PlayerController::saveState(core::GameState::PlayerState& state) const {
    if (!player_) return;
    
    state.id = static_cast<std::uint32_t>(player_->id());
    state.x = player_->position().x;
    state.y = player_->position().y;
    state.health = player_->health();
    
    Logger::instance().debug("[PlayerController] Saved state for player id=" + std::to_string(state.id));
}

void PlayerController::loadState(const core::GameState::PlayerState& state) {
    if (!player_) return;
    
    player_->setPosition({state.x, state.y});
    player_->setHealth(state.health);
    
    // Reset statistics
    totalDistanceMoved_ = 0.f;
    collisionCount_ = 0;
    slideCount_ = 0;
    
    Logger::instance().info("[PlayerController] Loaded state for player id=" + std::to_string(state.id));
}

void PlayerController::enableSliding(bool enable) {
    config_.movementMode = enable ? MovementHelper::CollisionMode::Slide : MovementHelper::CollisionMode::Block;
    Logger::instance().info("[PlayerController] Movement mode: " + std::string(enable ? "Slide" : "Block"));
}

void PlayerController::enableDebugVisualization(bool enable) {
    config_.enableDebugVisualization = enable;
    getEntityDebugInstance().setEnabled(enable);
    Logger::instance().info("[PlayerController] Debug visualization: " + std::string(enable ? "enabled" : "disabled"));
}

void PlayerController::exportTelemetryData(const std::string& filename) const {
    if (config_.enableTelemetry) {
        EntityTelemetry::instance().exportToCsv(filename);
        Logger::instance().info("[PlayerController] Exported telemetry data to: " + filename);
    } else {
        Logger::instance().warning("[PlayerController] Telemetry is disabled, cannot export data");
    }
}

void PlayerController::onItemCollected(Entity::Id itemId) {
    if (player_ && player_->canInteract()) {
        player_->onItemCollected(itemId);
    }
}

void PlayerController::onPuzzleSolved(Entity::Id puzzleId) {
    if (player_ && player_->canInteract()) {
        player_->onPuzzleSolved(puzzleId);
    }
}

void PlayerController::onEnemyEncounter(Entity::Id enemyId) {
    if (player_ && player_->canInteract()) {
        player_->onEnemyEncounter(enemyId);
    }
}

void PlayerController::updateMovement(float deltaTime) {
    if (!player_ || !collisionManager_) {
        // Fallback to basic movement if no collision manager
        sf::Vector2f intended = player_->computeIntendedMove(deltaTime);
        player_->commitMove(intended);
        return;
    }

    // Use advanced movement with collision detection
    lastMoveResult_ = player_->computeAdvancedMove(deltaTime, collisionManager_, config_.movementMode);
    player_->commitAdvancedMove(lastMoveResult_);
    
    // Update statistics
    updateStatistics(lastMoveResult_);
}

void PlayerController::updateStatistics(const MovementHelper::MovementResult& result) {
    if (!player_) return;
    
    // Calculate distance moved
    sf::Vector2f currentPos = player_->position();
    static sf::Vector2f lastPos = currentPos;
    
    float distance = std::sqrt((currentPos.x - lastPos.x) * (currentPos.x - lastPos.x) + 
                             (currentPos.y - lastPos.y) * (currentPos.y - lastPos.y));
    totalDistanceMoved_ += distance;
    lastPos = currentPos;
    
    // Track collision events
    if (result.collisionOccurred) {
        collisionCount_++;
        
        if (result.didSlide) {
            slideCount_++;
        }
    }
    
    // Periodically log statistics for telemetry
    static float statLogTimer = 0.f;
    statLogTimer += 0.016f; // Assume ~60 FPS
    if (statLogTimer >= 5.0f) { // Log every 5 seconds
        EntityTelemetry::instance().logPerformanceMetric("total_distance_moved", totalDistanceMoved_);
        EntityTelemetry::instance().logPerformanceMetric("collision_count", static_cast<float>(collisionCount_));
        EntityTelemetry::instance().logPerformanceMetric("slide_count", static_cast<float>(slideCount_));
        statLogTimer = 0.f;
    }
}

} // namespace entities
