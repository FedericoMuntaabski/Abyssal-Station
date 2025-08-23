#include "Entity.h"

namespace entities {

Entity::Entity(Id id, const sf::Vector2f& position, const sf::Vector2f& size)
    : id_(id), position_(position), size_(size) {}

Entity::~Entity() = default;

Entity::Id Entity::id() const noexcept { return id_; }

const sf::Vector2f& Entity::position() const noexcept { return position_; }
void Entity::setPosition(const sf::Vector2f& pos) noexcept { position_ = pos; }

const sf::Vector2f& Entity::size() const noexcept { return size_; }
void Entity::setSize(const sf::Vector2f& s) noexcept { size_ = s; }

} // namespace entities
