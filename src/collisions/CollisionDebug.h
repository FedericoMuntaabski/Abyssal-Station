#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONDEBUG_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONDEBUG_H

#include "CollisionManager.h"
#include "CollisionSystem.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace collisions {

class CollisionDebug {
public:
    struct Config {
        bool showColliders = true;
        bool showSpatialPartition = false;
        bool showRaycastResults = true;
        bool showCollisionNormals = false;
        bool showProfiling = true;
        
        // Visual settings
        sf::Color colliderColor = sf::Color(0, 255, 0, 100); // Green with transparency
        sf::Color triggerColor = sf::Color(0, 0, 255, 100);  // Blue with transparency
        sf::Color spatialPartitionColor = sf::Color(255, 255, 0, 50); // Yellow with transparency
        sf::Color normalColor = sf::Color::Red;
        sf::Color raycastColor = sf::Color::Cyan;
        
        float lineThickness = 1.f;
        float normalLength = 20.f;
    };

    explicit CollisionDebug(const Config& config = Config{});
    ~CollisionDebug() = default;

    // Configuration
    void setConfig(const Config& config) { config_ = config; }
    const Config& getConfig() const { return config_; }

    // Enable/disable specific debug features
    void setShowColliders(bool show) { config_.showColliders = show; }
    void setShowSpatialPartition(bool show) { config_.showSpatialPartition = show; }
    void setShowRaycastResults(bool show) { config_.showRaycastResults = show; }
    void setShowCollisionNormals(bool show) { config_.showCollisionNormals = show; }
    void setShowProfiling(bool show) { config_.showProfiling = show; }

    // Render debug information
    void render(sf::RenderWindow& window, const CollisionManager& manager, 
               const CollisionSystem& system);

    // Draw specific debug elements
    void drawColliders(sf::RenderWindow& window, const CollisionManager& manager);
    void drawSpatialPartition(sf::RenderWindow& window, const CollisionManager& manager);
    void drawRaycastResult(sf::RenderWindow& window, const RaycastHit& hit);
    void drawCollisionNormal(sf::RenderWindow& window, const sf::Vector2f& point, 
                           const sf::Vector2f& normal);
    void drawProfilingInfo(sf::RenderWindow& window, const CollisionManager& manager, 
                         const CollisionSystem& system);

    // Helper methods for manual debug drawing
    void drawBounds(sf::RenderWindow& window, const sf::FloatRect& bounds, 
                   const sf::Color& color = sf::Color::Green);
    void drawLine(sf::RenderWindow& window, const sf::Vector2f& start, 
                 const sf::Vector2f& end, const sf::Color& color = sf::Color::White);
    void drawCircle(sf::RenderWindow& window, const sf::Vector2f& center, 
                   float radius, const sf::Color& color = sf::Color::Blue);

    // Store raycast results for visualization
    void addRaycastResult(const RaycastHit& hit);
    void clearRaycastResults();

    // Store collision results for visualization
    void addCollisionResult(const CollisionResult& result);
    void clearCollisionResults();

private:
    Config config_;
    sf::Font debugFont_;
    bool fontLoaded_ = false;
    
    // Stored debug data
    std::vector<RaycastHit> raycastHits_;
    std::vector<CollisionResult> collisionResults_;
    
    // Helper methods
    void loadDebugFont();
    sf::RectangleShape createRectShape(const sf::FloatRect& bounds, const sf::Color& color);
    std::string formatProfileData(const CollisionManager::ProfileData& data);
    std::string formatSystemStats(const CollisionSystem::Stats& stats);

public:
    // Text drawing for external use
    void drawText(sf::RenderWindow& window, const std::string& text, 
                 const sf::Vector2f& position, const sf::Color& color = sf::Color::White);
};

// Utility class for real-time collision debugging
class CollisionDebugger {
public:
    explicit CollisionDebugger(CollisionManager& manager, CollisionSystem& system);
    ~CollisionDebugger() = default;

    // Update debug information (call once per frame)
    void update(float deltaTime);

    // Render all debug information
    void render(sf::RenderWindow& window);

    // Interactive debug features
    void handleInput(const sf::Event& event);

    // Toggle features with keyboard
    void toggleColliders() { debug_.setShowColliders(!debug_.getConfig().showColliders); }
    void toggleSpatialPartition() { debug_.setShowSpatialPartition(!debug_.getConfig().showSpatialPartition); }
    void toggleProfiling() { debug_.setShowProfiling(!debug_.getConfig().showProfiling); }

    // Manual raycast for debugging
    void performDebugRaycast(const sf::Vector2f& start, const sf::Vector2f& end);

    // Access to debug renderer
    CollisionDebug& getDebug() { return debug_; }
    const CollisionDebug& getDebug() const { return debug_; }

private:
    CollisionManager& manager_;
    CollisionSystem& system_;
    CollisionDebug debug_;
    
    // Interactive state
    bool isMouseRaycasting_ = false;
    sf::Vector2f raycastStart_;
    sf::Vector2f raycastEnd_;
    
    // Performance tracking
    float frameTime_ = 0.f;
    int frameCount_ = 0;
    float fps_ = 0.f;
    
    void updatePerformanceMetrics(float deltaTime);
};

// Global debug utilities
namespace DebugUtils {
    // Convert collision layer to color for visualization
    sf::Color getLayerColor(std::uint32_t layer);
    
    // Get debug name for collision layer
    std::string getLayerName(std::uint32_t layer);
    
    // Format vector for display
    std::string formatVector(const sf::Vector2f& vec);
    
    // Format rectangle for display
    std::string formatRect(const sf::FloatRect& rect);
}

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONDEBUG_H
