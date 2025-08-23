#include "EntityManager.h"
#include <algorithm>
#include "../core/Logger.h"

namespace entities {

void EntityManager::addEntity(std::unique_ptr<Entity> entity) {
    if (!entity) return;
    core::Logger::instance().info("[EntityManager] Adding entity id=" + std::to_string(entity->id()));
    entities_.push_back(std::move(entity));
}

bool EntityManager::removeEntity(Entity::Id id) {
    auto it = std::find_if(entities_.begin(), entities_.end(), [id](const std::unique_ptr<Entity>& e) {
        return e && e->id() == id;
    });

    if (it == entities_.end()) return false;
    core::Logger::instance().info("[EntityManager] Removing entity id=" + std::to_string((*it)->id()));
    entities_.erase(it);
    return true;
}

void EntityManager::updateAll(float deltaTime) {
    for (const auto& e : entities_) {
        if (e) e->update(deltaTime);
    }
}

void EntityManager::renderAll(sf::RenderWindow& window) const {
    for (const auto& e : entities_) {
        if (e) e->render(window);
    }
}

Entity* EntityManager::getEntity(Entity::Id id) const {
    auto it = std::find_if(entities_.begin(), entities_.end(), [id](const std::unique_ptr<Entity>& e) {
        return e && e->id() == id;
    });
    return (it != entities_.end()) ? it->get() : nullptr;
}

std::size_t EntityManager::count() const noexcept {
    return entities_.size();
}

} // namespace entities
