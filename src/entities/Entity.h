#ifndef ABYSSAL_STATION_SRC_ENTITIES_ENTITY_H
#define ABYSSAL_STATION_SRC_ENTITIES_ENTITY_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>

namespace entities {

class Entity {
public:
    using Id = std::uint32_t;

    // Construct an entity with optional id, position and size
    Entity(Id id = 0u, const sf::Vector2f& position = {0.f, 0.f}, const sf::Vector2f& size = {0.f, 0.f});
    virtual ~Entity();

    // Basic accessors
    Id id() const noexcept;
    const sf::Vector2f& position() const noexcept;
    virtual void setPosition(const sf::Vector2f& pos) noexcept;
    const sf::Vector2f& size() const noexcept;
    void setSize(const sf::Vector2f& s) noexcept;

    // Entity state management
    bool isActive() const noexcept { return active_; }
    void setActive(bool active) noexcept { active_ = active; }
    bool isVisible() const noexcept { return visible_; }
    void setVisible(bool visible) noexcept { visible_ = visible; }

    // Bounds helper
    sf::FloatRect getBounds() const noexcept {
        return sf::FloatRect(position_, size_);
    }

    // Simple collision layer support (bitmask)
    enum class Layer : std::uint32_t {
        None = 0,
        Default = 1 << 0,
        Player = 1 << 1,
        Enemy = 1 << 2,
        Item = 1 << 3,
        Wall = 1 << 4,
    };

    void setCollisionLayer(Layer l) noexcept { collisionLayer_ = static_cast<std::uint32_t>(l); }
    std::uint32_t collisionLayer() const noexcept { return collisionLayer_; }

    // Game loop hooks - must be implemented by concrete entities
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

protected:
    Id id_;
    sf::Vector2f position_;
    sf::Vector2f size_;
    std::uint32_t collisionLayer_{static_cast<std::uint32_t>(Layer::Default)};
    bool active_{true};
    bool visible_{true};
};

// Predefined masks for convenience
static constexpr std::uint32_t kLayerMaskNone = 0u;
static constexpr std::uint32_t kLayerMaskDefault = static_cast<std::uint32_t>(Entity::Layer::Default);
static constexpr std::uint32_t kLayerMaskPlayer = static_cast<std::uint32_t>(Entity::Layer::Player);
static constexpr std::uint32_t kLayerMaskEnemy = static_cast<std::uint32_t>(Entity::Layer::Enemy);
static constexpr std::uint32_t kLayerMaskItem = static_cast<std::uint32_t>(Entity::Layer::Item);
static constexpr std::uint32_t kLayerMaskWall = static_cast<std::uint32_t>(Entity::Layer::Wall);
static constexpr std::uint32_t kLayerMaskAll = 0xFFFFFFFFu;

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITY_H
