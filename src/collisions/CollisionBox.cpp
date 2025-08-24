#include "CollisionBox.h"
#include "../entities/Entity.h"
#include <algorithm>

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

void CollisionBox::addShape(std::unique_ptr<CollisionShape> shape, const std::string& name) {
    if (!shape) return;
    
    // Remove existing shape with same name if it exists
    removeShape(name);
    
    shapes_.emplace_back(name, std::move(shape));
}

void CollisionBox::removeShape(const std::string& name) {
    shapes_.erase(
        std::remove_if(shapes_.begin(), shapes_.end(),
            [&name](const auto& pair) {
                return pair.first == name;
            }),
        shapes_.end());
}

void CollisionBox::clearShapes() {
    shapes_.clear();
}

std::vector<sf::FloatRect> CollisionBox::getAllBounds() const {
    std::vector<sf::FloatRect> bounds;
    
    if (!shapes_.empty()) {
        // Use multi-shape system
        sf::Vector2f entityPos = getEntityPosition();
        for (const auto& [name, shape] : shapes_) {
            bounds.push_back(shape->getBounds(entityPos));
        }
    } else {
        // Fallback to legacy single bounds
        bounds.push_back(bounds_);
    }
    
    return bounds;
}

std::vector<const CollisionShape*> CollisionBox::getAllShapes() const {
    std::vector<const CollisionShape*> shapes;
    for (const auto& [name, shape] : shapes_) {
        shapes.push_back(shape.get());
    }
    return shapes;
}

sf::FloatRect CollisionBox::getShapeBounds(const std::string& name) const {
    for (const auto& [shapeName, shape] : shapes_) {
        if (shapeName == name) {
            return shape->getBounds(getEntityPosition());
        }
    }
    return sf::FloatRect();
}

const CollisionShape* CollisionBox::getShape(const std::string& name) const {
    for (const auto& [shapeName, shape] : shapes_) {
        if (shapeName == name) {
            return shape.get();
        }
    }
    return nullptr;
}

entities::Entity* CollisionBox::owner() const noexcept {
    return owner_;
}

void CollisionBox::updateFromEntity() {
    if (!owner_ || !dynamicResize_) return;
    
    // Update legacy bounds from entity
    bounds_ = owner_->getBounds();
    
    // Multi-shape bounds are updated dynamically via getEntityPosition()
}

sf::Vector2f CollisionBox::getEntityPosition() const {
    return owner_ ? owner_->position() : sf::Vector2f{0.f, 0.f};
}

// Note: layer is managed by caller; keep default if owner is null

} // namespace collisions
