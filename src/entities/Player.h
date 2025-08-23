#ifndef ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H
#define ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H

#include "Entity.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

namespace input { class InputManager; }

namespace entities {

class Player : public Entity {
public:
    enum class State { Idle, Walking, Dead };

    Player(Id id = 0u, const sf::Vector2f& position = {0.f,0.f}, const sf::Vector2f& size = {32.f,32.f}, float speed = 200.f, int health = 100);
    ~Player() override;

    // Process input and update internal velocity
    void handleInput(input::InputManager& inputManager);

    // Entity overrides
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    // Accessors
    float speed() const noexcept { return speed_; }
    int health() const noexcept { return health_; }
    State state() const noexcept { return state_; }

private:
    float speed_;
    int health_;
    State state_;
    sf::Vector2f velocity_;
    sf::RectangleShape shape_;
    // Debug logging: interval (seconds) and accumulator
    float debugLogInterval_{0.5f};
    float debugLogTimer_{0.f};
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_PLAYER_H
