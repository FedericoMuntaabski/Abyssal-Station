#ifndef ABYSSAL_STATION_SRC_ENTITIES_WALL_H
#define ABYSSAL_STATION_SRC_ENTITIES_WALL_H

#include "Entity.h"
#include <SFML/Graphics/RectangleShape.hpp>

namespace entities {

class Wall : public Entity {
public:
    Wall(Id id = 0u, const sf::Vector2f& position = {0.f,0.f}, const sf::Vector2f& size = {32.f,32.f});
    ~Wall() override;

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::RectangleShape shape_;
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_WALL_H
