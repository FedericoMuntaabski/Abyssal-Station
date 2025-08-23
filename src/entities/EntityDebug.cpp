#include "EntityDebug.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "../core/Logger.h"
#include <sstream>
#include <iomanip>

namespace entities {

using core::Logger;

EntityDebug::EntityDebug() {
    // Initialize shapes and text with default settings
    boundsShape_.setFillColor(sf::Color::Transparent);
    boundsShape_.setOutlineThickness(1.f);
    
    // debugText_ will be initialized when a font is loaded
}

bool EntityDebug::loadFont(const std::string& fontPath) {
    if (font_.openFromFile(fontPath)) {
        hasFont_ = true;
        // Initialize debugText_ now that we have a font
        debugText_ = std::make_unique<sf::Text>(font_, "", static_cast<unsigned int>(config_.textSize));
        debugText_->setFillColor(config_.textColor);
        Logger::instance().info("[EntityDebug] Loaded font: " + fontPath);
        return true;
    }
    Logger::instance().warning("[EntityDebug] Failed to load font: " + fontPath);
    return false;
}

void EntityDebug::renderEntityDebug(sf::RenderWindow& window, Entity* entity) {
    if (!enabled_ || !entity) return;

    if (config_.showBounds) {
        renderBounds(window, entity);
    }

    if (hasFont_ && (config_.showIds || config_.showHealth || config_.showCollisionLayers)) {
        renderEntityInfo(window, entity);
    }
}

void EntityDebug::renderManagerDebug(sf::RenderWindow& window, EntityManager* manager) {
    if (!enabled_ || !manager) return;

    auto entities = manager->allEntities();
    for (auto* entity : entities) {
        renderEntityDebug(window, entity);
    }

    if (config_.showPerformanceStats) {
        renderPerformanceOverlay(window, manager);
    }
}

void EntityDebug::renderPerformanceOverlay(sf::RenderWindow& window, EntityManager* manager) {
    if (!hasFont_ || !manager) return;

    auto stats = manager->getPerformanceStats();
    auto view = window.getView();
    auto viewSize = view.getSize();
    auto viewCenter = view.getCenter();

    // Position overlay in top-right corner
    sf::Vector2f overlayPos(
        viewCenter.x + viewSize.x * 0.5f - 250.f,
        viewCenter.y - viewSize.y * 0.5f + 10.f
    );

    if (!hasFont_ || !debugText_) return;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Entity Performance:\n";
    ss << "Total: " << stats.totalEntities << "\n";
    ss << "Updated: " << stats.entitiesUpdated << "\n";
    ss << "Rendered: " << stats.entitiesRendered << "\n";
    ss << "Update Time: " << stats.lastUpdateTime << "ms\n";
    ss << "Render Time: " << stats.lastRenderTime << "ms";

    debugText_->setString(ss.str());
    debugText_->setPosition(overlayPos);
    debugText_->setCharacterSize(14);
    debugText_->setFillColor(sf::Color::Yellow);

    // Draw background
    sf::FloatRect textBounds = debugText_->getLocalBounds();
    sf::RectangleShape background;
    background.setPosition(overlayPos);
    background.setSize({textBounds.size.x + 10.f, textBounds.size.y + 10.f});
    background.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(background);
    window.draw(*debugText_);
}

void EntityDebug::renderBounds(sf::RenderWindow& window, Entity* entity) {
    auto bounds = entity->getBounds();
    boundsShape_.setPosition(bounds.position);
    boundsShape_.setSize(bounds.size);
    
    // Color based on collision layer
    sf::Color layerColor = getLayerColor(entity->collisionLayer());
    boundsShape_.setOutlineColor(layerColor);
    
    window.draw(boundsShape_);
}

void EntityDebug::renderEntityInfo(sf::RenderWindow& window, Entity* entity) {
    std::stringstream info;
    
    if (config_.showIds) {
        info << "ID:" << entity->id();
    }
    
    if (config_.showCollisionLayers) {
        if (!info.str().empty()) info << "\n";
        info << "Layer:" << getCollisionLayerName(entity->collisionLayer());
    }
    
    // Special handling for Player entities
    if (config_.showHealth) {
        if (auto* player = dynamic_cast<Player*>(entity)) {
            if (!info.str().empty()) info << "\n";
            info << "HP:" << player->health();
            info << " State:" << static_cast<int>(player->state());
        }
    }
    
    if (config_.showVelocity) {
        if (auto* player = dynamic_cast<Player*>(entity)) {
            // Would need velocity accessor for this
            if (!info.str().empty()) info << "\n";
            info << "Speed:" << player->speed();
        }
    }
    
    if (!info.str().empty() && hasFont_ && debugText_) {
        debugText_->setString(info.str());
        debugText_->setCharacterSize(static_cast<unsigned int>(config_.textSize));
        debugText_->setFillColor(config_.textColor);
        
        // Position text above entity
        sf::Vector2f textPos = entity->position();
        textPos.y -= 20.f;
        debugText_->setPosition(textPos);
        
        window.draw(*debugText_);
    }
}

std::string EntityDebug::getCollisionLayerName(std::uint32_t layer) const {
    if (layer == static_cast<std::uint32_t>(Entity::Layer::None)) return "None";
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Default)) return "Default";
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Player)) return "Player";
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Enemy)) return "Enemy";
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Item)) return "Item";
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Wall)) return "Wall";
    return "Unknown";
}

sf::Color EntityDebug::getLayerColor(std::uint32_t layer) const {
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Player)) return sf::Color::Blue;
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Enemy)) return sf::Color::Red;
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Item)) return sf::Color::Green;
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Wall)) return sf::Color::Yellow;
    return sf::Color::White;
}

EntityDebug& getEntityDebugInstance() {
    static EntityDebug instance;
    return instance;
}

} // namespace entities
