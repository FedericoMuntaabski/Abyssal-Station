#ifndef ABYSSAL_STATION_SRC_ENTITIES_ENTITYDEBUG_H
#define ABYSSAL_STATION_SRC_ENTITIES_ENTITYDEBUG_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <vector>
#include <memory>

namespace entities {

class Entity;
class EntityManager;

/**
 * Debug visualization system for entities.
 * Provides visual debugging tools for development and testing.
 */
class EntityDebug {
public:
    struct DebugConfig {
        bool showBounds{false};
        bool showIds{false};
        bool showHealth{false};
        bool showVelocity{false};
        bool showCollisionLayers{false};
        bool showPerformanceStats{false};
        sf::Color boundsColor{sf::Color::Red};
        sf::Color textColor{sf::Color::White};
        float textSize{12.f};
    };

    EntityDebug();
    ~EntityDebug() = default;

    // Configuration
    void setConfig(const DebugConfig& config) { config_ = config; }
    const DebugConfig& getConfig() const { return config_; }
    
    // Enable/disable specific debug features
    void setShowBounds(bool show) { config_.showBounds = show; }
    void setShowIds(bool show) { config_.showIds = show; }
    void setShowHealth(bool show) { config_.showHealth = show; }
    void setShowVelocity(bool show) { config_.showVelocity = show; }
    void setShowCollisionLayers(bool show) { config_.showCollisionLayers = show; }
    void setShowPerformanceStats(bool show) { config_.showPerformanceStats = show; }
    
    // Toggle debug mode entirely
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    
    // Rendering
    void renderEntityDebug(sf::RenderWindow& window, Entity* entity);
    void renderManagerDebug(sf::RenderWindow& window, EntityManager* manager);
    void renderPerformanceOverlay(sf::RenderWindow& window, EntityManager* manager);

    // Font management
    bool loadFont(const std::string& fontPath);
    void setFont(const sf::Font& font) { font_ = font; hasFont_ = true; }

private:
    DebugConfig config_;
    bool enabled_{false};
    sf::Font font_;
    bool hasFont_{false};
    mutable sf::RectangleShape boundsShape_;
    mutable std::unique_ptr<sf::Text> debugText_;

    // Helper methods
    void renderBounds(sf::RenderWindow& window, Entity* entity);
    void renderEntityInfo(sf::RenderWindow& window, Entity* entity);
    std::string getCollisionLayerName(std::uint32_t layer) const;
    sf::Color getLayerColor(std::uint32_t layer) const;
};

/**
 * Global debug instance for easy access.
 */
EntityDebug& getEntityDebugInstance();

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITYDEBUG_H
