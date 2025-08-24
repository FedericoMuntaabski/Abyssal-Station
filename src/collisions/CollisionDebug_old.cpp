// Minimal stub implementation to allow compilation with SFML 3.x
// Full debug features disabled due to SFML API compatibility

#include "CollisionDebug.h"
#include "../core/Logger.h"

namespace collisions {

CollisionDebug::CollisionDebug(const Config& config) : config_(config), fontLoaded_(false) {
    // Minimal initialization
}

void CollisionDebug::render(sf::RenderWindow& window, const CollisionManager& manager, 
                          const CollisionSystem& system) {
    // Basic rendering stub - features disabled for SFML 3.x compatibility
    if (config_.showColliders) {
        drawColliders(window, manager);
    }
}

void CollisionDebug::drawColliders(sf::RenderWindow& window, const CollisionManager& manager) {
    // Stub implementation
}

void CollisionDebug::drawSpatialPartition(sf::RenderWindow& window, const CollisionManager& manager) {
    // Stub implementation
}

void CollisionDebug::drawRaycastResult(sf::RenderWindow& window, const RaycastHit& hit) {
    // Stub implementation
}

void CollisionDebug::drawCollisionNormal(sf::RenderWindow& window, const sf::Vector2f& point, 
                                       const sf::Vector2f& normal, const sf::Color& color) {
    // Stub implementation
}

void CollisionDebug::drawProfilingInfo(sf::RenderWindow& window, const CollisionManager& manager, 
                                     const CollisionSystem& system) {
    // Stub implementation
}

void CollisionDebug::drawBounds(sf::RenderWindow& window, const sf::FloatRect& bounds, 
                              const sf::Color& color) {
    // Stub implementation
}

void CollisionDebug::drawLine(sf::RenderWindow& window, const sf::Vector2f& start, 
                             const sf::Vector2f& end, const sf::Color& color) {
    // Stub implementation
}

void CollisionDebug::drawCircle(sf::RenderWindow& window, const sf::Vector2f& center, 
                               float radius, const sf::Color& color) {
    // Stub implementation
}

void CollisionDebug::drawText(sf::RenderWindow& window, const std::string& text, 
                             const sf::Vector2f& position, const sf::Color& color) {
    // Stub implementation - disabled for SFML compatibility
}

void CollisionDebug::addRaycastResult(const RaycastHit& hit) {
    raycastHits_.push_back(hit);
}

void CollisionDebug::clearRaycastResults() {
    raycastHits_.clear();
}

void CollisionDebug::addCollisionResult(const CollisionResult& result) {
    collisionResults_.push_back(result);
}

void CollisionDebug::clearCollisionResults() {
    collisionResults_.clear();
}

void CollisionDebug::loadDebugFont() {
    fontLoaded_ = false; // Disabled for compatibility
}

sf::RectangleShape CollisionDebug::createRectShape(const sf::FloatRect& bounds, const sf::Color& color) {
    sf::RectangleShape shape;
    shape.setSize(sf::Vector2f(bounds.getSize()));
    shape.setPosition(sf::Vector2f(bounds.getPosition()));
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(color);
    shape.setOutlineThickness(1.0f);
    return shape;
}

std::string CollisionDebug::formatProfileData(const CollisionManager::ProfileData& data) {
    return "Profiling disabled";
}

std::string CollisionDebug::formatSystemStats(const CollisionSystem::Stats& stats) {
    return "Stats disabled";
}

// CollisionDebugger stubs
CollisionDebugger::CollisionDebugger(CollisionManager& manager, CollisionSystem& system)
    : manager_(manager), system_(system), debug_(), frameTime_(0.0f), fps_(0.0f), 
      frameCount_(0), isMouseRaycasting_(false) {
}

void CollisionDebugger::update(float deltaTime) {
    updatePerformanceMetrics(deltaTime);
}

void CollisionDebugger::render(sf::RenderWindow& window) {
    debug_.render(window, manager_, system_);
}

void CollisionDebugger::handleInput(const sf::Event& event) {
    // Input handling disabled for SFML compatibility
}

void CollisionDebugger::toggleColliders() {
    auto& config = debug_.getConfig();
    config.showColliders = !config.showColliders;
}

void CollisionDebugger::toggleSpatialPartition() {
    auto& config = debug_.getConfig();
    config.showSpatialPartition = !config.showSpatialPartition;
}

void CollisionDebugger::toggleProfiling() {
    auto& config = debug_.getConfig();
    config.showProfiling = !config.showProfiling;
}

void CollisionDebugger::performDebugRaycast(const sf::Vector2f& start, const sf::Vector2f& end) {
    // Raycast debugging disabled
}

void CollisionDebugger::updatePerformanceMetrics(float deltaTime) {
    frameTime_ = deltaTime;
    frameCount_++;
    
    if (frameCount_ >= 60) {
        fps_ = 60.0f / (frameTime_ * 60.0f);
        frameCount_ = 0;
    }
}

std::string CollisionDebugger::getLayerName(std::uint32_t layer) {
    return "Layer " + std::to_string(layer);
}

std::string CollisionDebugger::formatVector(const sf::Vector2f& vec) {
    return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
}

std::string CollisionDebugger::formatRect(const sf::FloatRect& rect) {
    return "(" + std::to_string(rect.getPosition().x) + ", " + std::to_string(rect.getPosition().y) + 
           ", " + std::to_string(rect.getSize().x) + ", " + std::to_string(rect.getSize().y) + ")";
}

} // namespace collisions
