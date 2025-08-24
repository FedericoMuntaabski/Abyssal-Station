#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H

#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <string>
#include <memory>

namespace entities { class Entity; }

namespace collisions {

// Collision shape types
enum class CollisionShapeType {
    Rectangle,
    Circle,
    // Polygon // Future extension
};

// Base collision shape
struct CollisionShape {
    CollisionShapeType type;
    sf::Vector2f offset{0.f, 0.f}; // Offset from entity position
    bool isTrigger{false}; // True for sensor/trigger shapes
    
    virtual ~CollisionShape() = default;
    virtual sf::FloatRect getBounds(const sf::Vector2f& entityPos) const = 0;
    virtual std::unique_ptr<CollisionShape> clone() const = 0;
};

// Rectangle collision shape
struct RectangleShape : public CollisionShape {
    sf::Vector2f size{1.f, 1.f};
    
    RectangleShape(const sf::Vector2f& s = {1.f, 1.f}, const sf::Vector2f& off = {0.f, 0.f}, bool trigger = false) {
        type = CollisionShapeType::Rectangle;
        size = s;
        offset = off;
        isTrigger = trigger;
    }
    
    sf::FloatRect getBounds(const sf::Vector2f& entityPos) const override {
        return sf::FloatRect({entityPos.x + offset.x, entityPos.y + offset.y}, {size.x, size.y});
    }
    
    std::unique_ptr<CollisionShape> clone() const override {
        return std::make_unique<RectangleShape>(*this);
    }
};

// Circle collision shape
struct CircleShape : public CollisionShape {
    float radius{1.f};
    
    CircleShape(float r = 1.f, const sf::Vector2f& off = {0.f, 0.f}, bool trigger = false) {
        type = CollisionShapeType::Circle;
        radius = r;
        offset = off;
        isTrigger = trigger;
    }
    
    sf::FloatRect getBounds(const sf::Vector2f& entityPos) const override {
        sf::Vector2f center = entityPos + offset;
        return sf::FloatRect({center.x - radius, center.y - radius}, {radius * 2.f, radius * 2.f});
    }
    
    std::unique_ptr<CollisionShape> clone() const override {
        return std::make_unique<CircleShape>(*this);
    }
};

class CollisionBox {
public:
    CollisionBox(entities::Entity* owner = nullptr, const sf::FloatRect& bounds = sf::FloatRect());
    ~CollisionBox();
    
    // Make class movable but not copyable
    CollisionBox(const CollisionBox&) = delete;
    CollisionBox& operator=(const CollisionBox&) = delete;
    CollisionBox(CollisionBox&&) = default;
    CollisionBox& operator=(CollisionBox&&) = default;

    // Legacy single bounds interface (for backward compatibility)
    const sf::FloatRect& getBounds() const noexcept;
    void setBounds(const sf::FloatRect& bounds) noexcept;

    // Multi-shape interface
    void addShape(std::unique_ptr<CollisionShape> shape, const std::string& name = "");
    void removeShape(const std::string& name);
    void clearShapes();
    
    // Get all collision bounds for this entity
    std::vector<sf::FloatRect> getAllBounds() const;
    std::vector<const CollisionShape*> getAllShapes() const;
    
    // Get bounds for a specific shape
    sf::FloatRect getShapeBounds(const std::string& name) const;
    const CollisionShape* getShape(const std::string& name) const;

    entities::Entity* owner() const noexcept;
    std::uint32_t layer() const noexcept { return layer_; }
    void setLayer(std::uint32_t l) noexcept { layer_ = l; }
    
    // Dynamic resize support
    void setDynamicResize(bool enabled) { dynamicResize_ = enabled; }
    bool isDynamicResize() const { return dynamicResize_; }
    
    // Update all shapes based on entity's current position/size
    void updateFromEntity();

private:
    entities::Entity* owner_;
    sf::FloatRect bounds_; // Legacy single bounds
    std::uint32_t layer_{0};
    bool dynamicResize_{false};
    
    // Multi-shape support
    std::vector<std::pair<std::string, std::unique_ptr<CollisionShape>>> shapes_;
    
    // Helper to get entity position safely
    sf::Vector2f getEntityPosition() const;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H
