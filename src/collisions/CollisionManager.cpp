#include "CollisionManager.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"

#include <algorithm>

namespace collisions {

void CollisionManager::addCollider(entities::Entity* owner, const sf::FloatRect& bounds) {
    if (!owner) return;

    // Try to find existing collider for owner and update
    for (auto& cb : colliders_) {
        if (cb.owner() == owner) {
            cb.setBounds(bounds);
            return;
        }
    }

    // Not found -> add new
    colliders_.emplace_back(owner, bounds);
}

void CollisionManager::removeCollider(entities::Entity* owner) {
    if (!owner) return;
    colliders_.erase(std::remove_if(colliders_.begin(), colliders_.end(), [owner](const CollisionBox& cb) {
        return cb.owner() == owner;
    }), colliders_.end());
}

std::vector<entities::Entity*> CollisionManager::checkCollisions(entities::Entity* owner) const {
    std::vector<entities::Entity*> result;
    if (!owner) return result;

    const CollisionBox* subject = nullptr;
    for (const auto& cb : colliders_) {
        if (cb.owner() == owner) {
            subject = &cb;
            break;
        }
    }

    if (!subject) return result;

    for (const auto& cb : colliders_) {
        if (cb.owner() == owner) continue;
        auto inter = subject->getBounds().findIntersection(cb.getBounds());
        if (inter.has_value()) {
            // Log collision for debug
            core::Logger::instance().info("[CollisionManager] Collision detected between entities id=" + std::to_string(owner->id()) +
                " and id=" + std::to_string(cb.owner()->id()));
            result.push_back(cb.owner());
        }
    }

    return result;
}

entities::Entity* CollisionManager::firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude) const {
    for (const auto& cb : colliders_) {
        if (cb.owner() == exclude) continue;
        auto opt = bounds.findIntersection(cb.getBounds());
        if (opt.has_value()) return cb.owner();
    }
    return nullptr;
}

} // namespace collisions
