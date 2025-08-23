#include "Wall.h"
#include "../core/Logger.h"

namespace entities {

Wall::Wall(Id id, const sf::Vector2f& position, const sf::Vector2f& size)
    : Entity(id, position, size) {
    shape_.setSize(size_);
    shape_.setFillColor(sf::Color::Green);
    shape_.setPosition(position_);
    core::Logger::instance().info("Wall: created id=" + std::to_string(id_));
}

Wall::~Wall() {
    core::Logger::instance().info("Wall: destroyed id=" + std::to_string(id_));
}

void Wall::update(float /*deltaTime*/) {
    // Static wall - nothing to update
}

void Wall::render(sf::RenderWindow& window) {
    window.draw(shape_);
}

} // namespace entities
