#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_NOISESYSTEM_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_NOISESYSTEM_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <chrono>

namespace entities { class Player; }

namespace gameplay {

/**
 * @brief Simple noise system for tracking sound events
 */
class NoiseSystem {
public:
    struct NoiseEvent {
        sf::Vector2f position;
        float intensity;
        std::string source;
        float radius;
        int lifetime; // milliseconds
        std::chrono::steady_clock::time_point timestamp;
    };

private:
    std::vector<NoiseEvent> m_activeNoise;
    bool m_debugMode = false;

public:
    NoiseSystem();
    ~NoiseSystem() = default;

    // Core functionality
    void update(float deltaTime);
    void generateNoise(sf::Vector2f position, float intensity, const std::string& source);
    
    // Player noise generation helpers
    void generatePlayerWalkingNoise(entities::Player* player);
    void generatePlayerRunningNoise(entities::Player* player);
    void generatePlayerDoorNoise(sf::Vector2f position);
    void generatePlayerCombatNoise(sf::Vector2f position);
    
    // Configuration
    void setNoiseSurfaceMultiplier(const std::string& surface, float multiplier);
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }
    
    // Debug rendering
    void renderDebug(sf::RenderWindow& window) const;
    
    // Cleanup
    void clearAllNoise();
    size_t getActiveNoiseCount() const { return m_activeNoise.size(); }

private:
    float calculateRadius(float intensity, const std::string& source) const;
    int calculateLifetime(float intensity) const;
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_NOISESYSTEM_H
