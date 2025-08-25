#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_VISIONSYSTEM_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_VISIONSYSTEM_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace entities { class Player; }

namespace gameplay {

/**
 * @brief Simple vision system with basic lighting effects
 */
class VisionSystem {
private:
    // Core state
    bool m_initialized = false;
    unsigned int m_windowWidth = 800;
    unsigned int m_windowHeight = 600;
    
    // Vision state
    sf::Vector2f m_playerPosition;
    sf::Vector2f m_playerDirection;
    bool m_flashlightOn = false;
    float m_flashlightBattery = 100.0f;
    
    // Lighting
    float m_ambientLightLevel = 0.2f;
    
    // Rendering
    sf::RenderTexture m_visionTexture;
    
    // Debug
    bool m_debugMode = false;
    
public:
    VisionSystem();
    ~VisionSystem() = default;

    // Initialization
    bool initialize(unsigned int windowWidth, unsigned int windowHeight);
    
    // Core functionality
    void update(float deltaTime, entities::Player* player);
    void render(sf::RenderWindow& window, entities::Player* player);
    
    // Vision queries
    bool canSee(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const;
    bool isInVisionCone(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const;
    bool isInFlashlightBeam(entities::Player* player, sf::Vector2f targetPos) const;
    float getVisibilityAt(sf::Vector2f position, entities::Player* player) const;
    
    // Light management
    void addLightSource(sf::Vector2f position, float radius, sf::Color color = sf::Color::White, float intensity = 1.0f);
    void removeLightSource(size_t index);
    void setAmbientLight(float level);
    
    // Configuration
    void setDebugMode(bool enabled);
    bool isDebugMode() const { return m_debugMode; }
    
    // Battery management
    float getFlashlightBattery() const { return m_flashlightBattery; }
    void setFlashlightBattery(float level) { m_flashlightBattery = level; }
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_VISIONSYSTEM_H
