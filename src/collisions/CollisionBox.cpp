#include "CollisionBox.h"
#include "../entities/Entity.h"

namespace collisions {

CollisionBox::CollisionBox(entities::Entity* owner, const sf::FloatRect& bounds)
    : owner_(owner), bounds_(bounds) {}

CollisionBox::~CollisionBox() = default;

const sf::FloatRect& CollisionBox::getBounds() const noexcept {
    return bounds_;
}

void CollisionBox::setBounds(const sf::FloatRect& bounds) noexcept {
    bounds_ = bounds;
}

entities::Entity* CollisionBox::owner() const noexcept {
    return owner_;
}

// Note: layer is managed by caller; keep default if owner is null

} // namespace collisions
