#ifndef ABYSSAL_STATION_SRC_AI_ENEMYMANAGER_H
#define ABYSSAL_STATION_SRC_AI_ENEMYMANAGER_H

#include "Enemy.h"
#include <vector>
#include <memory>

namespace ai {

class EnemyManager {
public:
    EnemyManager() = default;
    ~EnemyManager() = default;

    // Add an enemy by pointer ownership
    // Register an enemy pointer (EntityManager keeps ownership via unique_ptr)
    void addEnemyPointer(Enemy* e) { enemies_.push_back(e); }

    // Update all enemies with the player's position (runs FSM)
    void updateAll(float dt, const sf::Vector2f& playerPos);

    // Plan movement for all enemies (collision-aware)
    void planAllMovement(float dt, collisions::CollisionManager* cm);

    // Commit all planned moves after collision checks
    void commitAllMoves(collisions::CollisionManager* cm);

    // Render all enemies
    void renderAll(sf::RenderWindow& window);

    std::vector<Enemy*>& enemies() { return enemies_; }

private:
    std::vector<Enemy*> enemies_;
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_ENEMYMANAGER_H
