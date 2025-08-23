#include "CollisionSystem.h"
#include "CollisionManager.h"
#include "../entities/Entity.h"
#include "../entities/Player.h"
#include "../core/Logger.h"

#include <SFML/Graphics/Rect.hpp>
#include <string>

namespace collisions {

using core::Logger;

CollisionSystem::CollisionSystem(CollisionManager& manager)
    : manager_(manager), logTimer_(0.f), logInterval_(0.25f) {}

void CollisionSystem::resolve(entities::Entity* entity, float deltaTime) {
    if (!entity) return;

    // accumulate timer for rate-limited logs
    logTimer_ += deltaTime;

    // Only resolve for Player entities for now
    auto* player = dynamic_cast<entities::Player*>(entity);
    if (!player) return;

    // Get collisions from manager
    auto colliding = manager_.checkCollisions(entity);
    for (auto* other : colliding) {
        if (!other) continue;

        // Compute intersection rect between entity and other
        sf::FloatRect a;
        a.position.x = entity->position().x; a.position.y = entity->position().y;
        a.size.x = entity->size().x; a.size.y = entity->size().y;

        sf::FloatRect b;
        b.position.x = other->position().x; b.position.y = other->position().y;
        b.size.x = other->size().x; b.size.y = other->size().y;

        auto opt = a.findIntersection(b);
        if (opt.has_value()) {
            sf::FloatRect inter = opt.value();

            float moveX = 0.f;
            float moveY = 0.f;

            if (inter.size.x < inter.size.y) {
                // Push in X direction
                if (a.position.x < b.position.x) moveX = -inter.size.x; else moveX = inter.size.x;
            } else {
                // Push in Y direction
                if (a.position.y < b.position.y) moveY = -inter.size.y; else moveY = inter.size.y;
            }

            // Apply translation to player position
            sf::Vector2f newPos = player->position();
            newPos.x += moveX;
            newPos.y += moveY;
            player->setPosition(newPos);

            // Classify collision for logging
            const char* collisionDesc = "Player vs Wall";
            if (dynamic_cast<entities::Player*>(other)) collisionDesc = "Player vs Player";

            // Rate-limit collision logs so they don't flood; similar rhythm to player's debug log
            if (logTimer_ >= logInterval_) {
                logTimer_ = 0.f;
                Logger::instance().info(std::string("[collision] ") + collisionDesc +
                    " idA=" + std::to_string(player->id()) + " idB=" + std::to_string(other->id()));

                Logger::instance().info(std::string("[CollisionSystem] Resolved collision: player id=") + std::to_string(player->id()) +
                    " moved by (" + std::to_string(moveX) + "," + std::to_string(moveY) + ") to (" +
                    std::to_string(newPos.x) + "," + std::to_string(newPos.y) + ")");
            }
        }
    }
}

void CollisionSystem::resolveAll() {
    // Not implemented: caller should iterate relevant entities and call resolve(entity, dt)
}

} // namespace collisions
