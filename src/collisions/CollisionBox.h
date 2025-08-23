#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H

#include <SFML/Graphics/Rect.hpp>

namespace entities { class Entity; }

namespace collisions {

class CollisionBox {
public:
    CollisionBox(entities::Entity* owner = nullptr, const sf::FloatRect& bounds = sf::FloatRect());
    ~CollisionBox();

    const sf::FloatRect& getBounds() const noexcept;
    void setBounds(const sf::FloatRect& bounds) noexcept;

    entities::Entity* owner() const noexcept;

private:
    entities::Entity* owner_;
    sf::FloatRect bounds_;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONBOX_H
