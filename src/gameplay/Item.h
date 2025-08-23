#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_ITEM_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_ITEM_H

#include "../entities/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace entities { class Player; }
namespace collisions { class CollisionManager; }

namespace gameplay {

enum class ItemType { Key, Tool, Collectible };

class Item : public entities::Entity {
public:
    Item(entities::Entity::Id id = 0u,
         const sf::Vector2f& position = {0.f, 0.f},
         const sf::Vector2f& size = {16.f, 16.f},
         ItemType type = ItemType::Collectible,
         collisions::CollisionManager* collisionManager = nullptr);
    ~Item() override;

    // Interaction API
    void interact(entities::Player& player);

    // Game loop
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    bool isCollected() const noexcept { return collected_; }
    ItemType type() const noexcept { return type_; }

    // Animation support
    bool isAnimating() const noexcept { return isAnimating_; }
    void setDisabled(bool disabled) { disabled_ = disabled; }
    bool isDisabled() const noexcept { return disabled_; }

    // Optional: attach or change collision manager after construction
    void setCollisionManager(collisions::CollisionManager* cm);

private:
    ItemType type_;
    bool collected_;
    bool disabled_{false};
    bool isAnimating_{false};
    float animationTime_{0.0f};
    sf::Vector2f originalScale_{1.0f, 1.0f};

    sf::RectangleShape shape_;
    collisions::CollisionManager* collisionManager_;
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_ITEM_H
