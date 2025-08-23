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
    void setPosition(const sf::Vector2f& pos) noexcept;
    const sf::Vector2f& size() const noexcept;
    void setSize(const sf::Vector2f& s) noexcept;

    // Game loop hooks - must be implemented by concrete entities
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

protected:
    Id id_;
    sf::Vector2f position_;
    sf::Vector2f size_;
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITY_H
