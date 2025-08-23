#include "CollisionManager.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"
#include "../ai/Enemy.h"
#include "../gameplay/Item.h"

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

    // Not found -> add new and set its layer from owner
    colliders_.emplace_back(owner, bounds);
    // Set layer on the newly added collision box from owner
    colliders_.back().setLayer(owner->collisionLayer());
}

void CollisionManager::updateColliderBounds(entities::Entity* owner, const sf::FloatRect& bounds) {
    // This is just an alias for addCollider since it already handles updates
    addCollider(owner, bounds);
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

entities::Entity* CollisionManager::firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude, std::uint32_t allowedLayers) const {
    for (const auto& cb : colliders_) {
        if (cb.owner() == exclude) continue;
        // Filter by allowedLayers mask
        if (allowedLayers != 0xFFFFFFFFu) {
            if ((cb.layer() & allowedLayers) == 0) continue;
        }
        auto opt = bounds.findIntersection(cb.getBounds());
        if (opt.has_value()) return cb.owner();
    }
    return nullptr;
}

// Helper: check whether a segment intersects any collider's bounds
bool CollisionManager::segmentIntersectsAny(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude, std::uint32_t allowedLayers) const {
    auto segmentIntersectsRect = [](const sf::Vector2f& a, const sf::Vector2f& b, const sf::FloatRect& r) {
        // Liang-Barsky algorithm for segment vs AABB
        float t0 = 0.f, t1 = 1.f;
        sf::Vector2f d = b - a;
        auto clip = [&](float p, float q) -> bool {
            if (std::abs(p) < 1e-6f) {
                if (q < 0.f) return false;
                return true;
            }
            float t = q / p;
            if (p < 0.f) {
                if (t > t1) return false;
                if (t > t0) t0 = t;
            } else {
                if (t < t0) return false;
                if (t < t1) t1 = t;
            }
            return true;
        };

        float minX = r.position.x;
        float minY = r.position.y;
        float maxX = r.position.x + r.size.x;
        float maxY = r.position.y + r.size.y;

        if (!clip(-d.x, a.x - minX)) return false;
        if (!clip( d.x, maxX - a.x)) return false;
        if (!clip(-d.y, a.y - minY)) return false;
        if (!clip( d.y, maxY - a.y)) return false;
        return t0 <= t1;
    };

    for (const auto& cb : colliders_) {
        if (cb.owner() == exclude) continue;
        if (allowedLayers != 0xFFFFFFFFu) {
            if ((cb.layer() & allowedLayers) == 0) continue;
        }
        if (segmentIntersectsRect(p0, p1, cb.getBounds())) return true;
    }
    return false;
}

} // namespace collisions
