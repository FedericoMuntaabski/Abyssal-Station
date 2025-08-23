#include "ai/EnemyManager.h"
#include "../collisions/CollisionManager.h"
#include "../collisions/CollisionSystem.h"
#include <SFML/Graphics/Rect.hpp>
#include "../core/Logger.h"

namespace ai {

void EnemyManager::updateAll(float dt, const sf::Vector2f& playerPos) {
    for (auto& e : enemies_) {
        if (e) e->update(dt, playerPos);
    }
}

void EnemyManager::renderAll(sf::RenderWindow& window) {
    for (auto& e : enemies_) {
        if (e) e->render(window);
    }
}

void EnemyManager::planAllMovement(float dt, collisions::CollisionManager* cm) {
    for (auto& e : enemies_) {
        if (e) e->performMovementPlanning(dt, cm);
    }
}

void EnemyManager::commitAllMoves(collisions::CollisionManager* cm) {
    for (auto& e : enemies_) {
        if (!e) continue;
        sf::Vector2f intended = e->computeIntendedMove(0.f);
        sf::FloatRect testBounds;
        testBounds.position.x = intended.x; testBounds.position.y = intended.y;
        testBounds.size.x = e->size().x; testBounds.size.y = e->size().y;
        auto blocker = cm ? cm->firstColliderForBounds(testBounds, e) : nullptr;
        if (!blocker) {
            e->commitMove(intended);
        } else {
            core::Logger::instance().info("[EnemyManager] Enemy movement blocked id=" + std::to_string(e->id()) +
                " by entity id=" + std::to_string(blocker->id()));
        }
    }
}

} // namespace ai
