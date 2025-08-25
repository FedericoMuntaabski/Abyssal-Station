#ifndef ABYSSAL_STATION_SRC_DEBUG_DEBUGOVERLAY_H
#define ABYSSAL_STATION_SRC_DEBUG_DEBUGOVERLAY_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>

namespace entities { class EntityManager; }
namespace ai { class EnemyManager; }
namespace collisions { class CollisionManager; }

namespace debug {

class DebugOverlay {
public:
    DebugOverlay();
    ~DebugOverlay() = default;

    // State management
    void toggle();
    void setVisible(bool visible) { m_isVisible = visible; }
    bool isVisible() const { return m_isVisible; }

    // Update metrics
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    // Set references for metric collection
    void setEntityManager(entities::EntityManager* entityManager) { m_entityManager = entityManager; }
    void setEnemyManager(ai::EnemyManager* enemyManager) { m_enemyManager = enemyManager; }
    void setCollisionManager(collisions::CollisionManager* collisionManager) { m_collisionManager = collisionManager; }

    // Manual metrics updates
    void setCustomMetric(const std::string& name, const std::string& value);
    void updateFrameTime(float frameTime);

private:
    struct Metric {
        std::string name;
        std::string value;
        sf::Color color;
    };

    // State
    bool m_isVisible{false};
    
    // Font and rendering
    sf::Font m_font;
    bool m_fontLoaded{false};
    sf::Vector2f m_position{10.0f, 50.0f};
    float m_lineHeight{16.0f};
    
    // Metrics storage
    std::vector<Metric> m_metrics;
    
    // Performance tracking
    std::chrono::steady_clock::time_point m_lastUpdate;
    float m_frameTime{0.0f};
    float m_fps{0.0f};
    float m_fpsUpdateTimer{0.0f};
    static constexpr float FPS_UPDATE_INTERVAL = 0.5f; // Update FPS every 0.5 seconds
    
    // Game system references
    entities::EntityManager* m_entityManager{nullptr};
    ai::EnemyManager* m_enemyManager{nullptr};
    collisions::CollisionManager* m_collisionManager{nullptr};
    
    // Helper methods
    void loadFont();
    void updatePerformanceMetrics(float deltaTime);
    void updateGameMetrics();
    void addMetric(const std::string& name, const std::string& value, sf::Color color = sf::Color::White);
    void clearMetrics();
    
    // Utility functions
    std::string formatMemorySize(size_t bytes);
    std::string formatFloat(float value, int precision = 2);
    size_t getMemoryUsage(); // Platform-specific memory usage
};

} // namespace debug

#endif // ABYSSAL_STATION_SRC_DEBUG_DEBUGOVERLAY_H
