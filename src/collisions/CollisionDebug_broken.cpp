#include "CollisionDebug.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"
#include <sstream>
#include <iomanip>

namespace collisions {

CollisionDebug::CollisionDebug(const Config& config) : config_(config) {
    loadDebugFont();
}

void CollisionDebug::render(sf::RenderWindow& window, const CollisionManager& manager, 
                           const CollisionSystem& system) {
    if (config_.showColliders) {
        drawColliders(window, manager);
    }
    
    if (config_.showSpatialPartition) {
        drawSpatialPartition(window, manager);
    }
    
    if (config_.showRaycastResults) {
        for (const auto& hit : raycastHits_) {
            drawRaycastResult(window, hit);
        }
    }
    
    if (config_.showCollisionNormals) {
        for (const auto& result : collisionResults_) {
            sf::Vector2f center(
                result.intersection.position.x + result.intersection.size.x * 0.5f,
                result.intersection.position.y + result.intersection.size.y * 0.5f
            );
            drawCollisionNormal(window, center, result.normal);
        }
    }
    
    if (config_.showProfiling) {
        drawProfilingInfo(window, manager, system);
    }
}

void CollisionDebug::drawColliders(sf::RenderWindow& window, const CollisionManager& manager) {
    // Note: This would need access to the internal colliders_ of CollisionManager
    // For now, we'll implement a public method to get debug information
    
    // This is a simplified version - in practice, we'd add a getDebugColliders() method
    // to CollisionManager that returns read-only access to colliders
}

void CollisionDebug::drawSpatialPartition(sf::RenderWindow& window, const CollisionManager& manager) {
    // Draw spatial partition structure (QuadTree nodes, hash grid cells, etc.)
    // This would require additional debug methods in the spatial partition classes
    
    std::string stats = manager.getSpatialPartitionStats();
    if (!stats.empty()) {
        drawText(window, stats, sf::Vector2f(10.f, 100.f), sf::Color::Yellow);
    }
}

void CollisionDebug::drawRaycastResult(sf::RenderWindow& window, const RaycastHit& hit) {
    if (!hit.valid) return;
    
    // Draw hit point
    drawCircle(window, hit.point, 3.f, config_.raycastColor);
    
    // Draw normal
    if (hit.normal.x != 0.f || hit.normal.y != 0.f) {
        sf::Vector2f normalEnd = hit.point + hit.normal * config_.normalLength;
        drawLine(window, hit.point, normalEnd, config_.normalColor);
    }
    
    // Draw distance text
    if (fontLoaded_) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << hit.distance;
        drawText(window, oss.str(), hit.point + sf::Vector2f(5.f, -15.f), config_.raycastColor);
    }
}

void CollisionDebug::drawCollisionNormal(sf::RenderWindow& window, const sf::Vector2f& point, 
                                       const sf::Vector2f& normal) {
    sf::Vector2f normalEnd = point + normal * config_.normalLength;
    drawLine(window, point, normalEnd, config_.normalColor);
    
    // Draw arrowhead
    sf::Vector2f perpendicular(-normal.y, normal.x);
    sf::Vector2f arrowPoint1 = normalEnd - normal * 5.f + perpendicular * 3.f;
    sf::Vector2f arrowPoint2 = normalEnd - normal * 5.f - perpendicular * 3.f;
    
    drawLine(window, normalEnd, arrowPoint1, config_.normalColor);
    drawLine(window, normalEnd, arrowPoint2, config_.normalColor);
}

void CollisionDebug::drawProfilingInfo(sf::RenderWindow& window, const CollisionManager& manager, 
                                      const CollisionSystem& system) {
    if (!fontLoaded_) return;
    
    float yOffset = 10.f;
    const float lineHeight = 20.f;
    
    // Manager profiling info
    auto profileData = manager.getProfileData();
    std::string managerInfo = formatProfileData(profileData);
    drawText(window, "Collision Manager:", sf::Vector2f(10.f, yOffset), sf::Color::White);
    yOffset += lineHeight;
    drawText(window, managerInfo, sf::Vector2f(10.f, yOffset), sf::Color::Cyan);
    yOffset += lineHeight * 2;
    
    // System stats
    auto systemStats = system.getStats();
    std::string systemInfo = formatSystemStats(systemStats);
    drawText(window, "Collision System:", sf::Vector2f(10.f, yOffset), sf::Color::White);
    yOffset += lineHeight;
    drawText(window, systemInfo, sf::Vector2f(10.f, yOffset), sf::Color::Green);
    yOffset += lineHeight * 2;
    
    // Spatial partition info
    std::string spatialInfo = manager.getSpatialPartitionStats();
    drawText(window, spatialInfo, sf::Vector2f(10.f, yOffset), sf::Color::Yellow);
}

void CollisionDebug::drawBounds(sf::RenderWindow& window, const sf::FloatRect& bounds, 
                               const sf::Color& color) {
    auto shape = createRectShape(bounds, color);
    window.draw(shape);
}

void CollisionDebug::drawLine(sf::RenderWindow& window, const sf::Vector2f& start, 
                             const sf::Vector2f& end, const sf::Color& color) {
    sf::Vertex line[2];
    line[0] = sf::Vertex(start, color);
    line[1] = sf::Vertex(end, color);
    window.draw(line, 2, sf::PrimitiveType::Lines);
}

void CollisionDebug::drawCircle(sf::RenderWindow& window, const sf::Vector2f& center, 
                               float radius, const sf::Color& color) {
    sf::CircleShape circle(radius);
    circle.setPosition(sf::Vector2f(center.x - radius, center.y - radius));
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(config_.lineThickness);
    window.draw(circle);
}

void CollisionDebug::addRaycastResult(const RaycastHit& hit) {
    raycastHits_.push_back(hit);
    
    // Limit stored results to prevent memory issues
    if (raycastHits_.size() > 100) {
        raycastHits_.erase(raycastHits_.begin());
    }
}

void CollisionDebug::clearRaycastResults() {
    raycastHits_.clear();
}

void CollisionDebug::addCollisionResult(const CollisionResult& result) {
    collisionResults_.push_back(result);
    
    // Limit stored results
    if (collisionResults_.size() > 50) {
        collisionResults_.erase(collisionResults_.begin());
    }
}

void CollisionDebug::clearCollisionResults() {
    collisionResults_.clear();
}

void CollisionDebug::loadDebugFont() {
    // Try to load a font for debug text
    // In a real implementation, you'd specify the path to a font file
    // For now, we'll just mark that no font is available
    fontLoaded_ = false;
    
    // Example:
    // if (debugFont_.loadFromFile("assets/fonts/debug.ttf")) {
    //     fontLoaded_ = true;
    // }
}

void CollisionDebug::drawText(sf::RenderWindow& window, const std::string& text, 
                             const sf::Vector2f& position, const sf::Color& color) {
    if (!fontLoaded_) return;
    
    sf::Text textObj;
    textObj.setFont(debugFont_);
    textObj.setString(text);
    textObj.setCharacterSize(12);
    textObj.setFillColor(color);
    textObj.setPosition(position);
    window.draw(textObj);
}

sf::RectangleShape CollisionDebug::createRectShape(const sf::FloatRect& bounds, const sf::Color& color) {
    sf::RectangleShape shape(sf::Vector2f(bounds.getSize().x, bounds.getSize().y));
    shape.setPosition(bounds.position);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(color);
    shape.setOutlineThickness(config_.lineThickness);
    return shape;
}

std::string CollisionDebug::formatProfileData(const CollisionManager::ProfileData& data) {
    std::ostringstream oss;
    oss << "Queries: " << data.totalQueries 
        << " | Total: " << data.totalTime.count() << "μs"
        << " | Broad: " << data.broadPhaseTime.count() << "μs"
        << " | Narrow: " << data.narrowPhaseTime.count() << "μs"
        << " | Tests: " << data.broadPhaseTests << "/" << data.narrowPhaseTests;
    return oss.str();
}

std::string CollisionDebug::formatSystemStats(const CollisionSystem::Stats& stats) {
    std::ostringstream oss;
    oss << "Resolutions: " << stats.totalResolutions
        << " | Events: " << stats.eventsTriggered
        << " | Correction: " << std::fixed << std::setprecision(1) << stats.totalCorrectionDistance
        << " | Sweep Tests: " << stats.continuousDetectionTests;
    return oss.str();
}

// CollisionDebugger Implementation
CollisionDebugger::CollisionDebugger(CollisionManager& manager, CollisionSystem& system)
    : manager_(manager), system_(system) {}

void CollisionDebugger::update(float deltaTime) {
    updatePerformanceMetrics(deltaTime);
    
    // Clear old debug data periodically
    frameCount_++;
    if (frameCount_ % 300 == 0) { // Every 5 seconds at 60 FPS
        debug_.clearRaycastResults();
        debug_.clearCollisionResults();
    }
}

void CollisionDebugger::render(sf::RenderWindow& window) {
    debug_.render(window, manager_, system_);
    
    // Draw FPS
    if (debug_.getConfig().showProfiling) {
        std::ostringstream oss;
        oss << "FPS: " << std::fixed << std::setprecision(1) << fps_
            << " | Frame: " << std::setprecision(2) << frameTime_ * 1000.f << "ms";
        debug_.drawText(window, oss.str(), sf::Vector2f(10.f, window.getSize().y - 30.f), sf::Color::White);
    }
}

void CollisionDebugger::handleInput(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::F1:
                toggleColliders();
                break;
            case sf::Keyboard::F2:
                toggleSpatialPartition();
                break;
            case sf::Keyboard::F3:
                toggleProfiling();
                break;
            default:
                break;
        }
    }
    
    // Mouse raycast debugging
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            isMouseRaycasting_ = true;
            raycastStart_ = sf::Vector2f(
                static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y)
            );
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right && isMouseRaycasting_) {
            raycastEnd_ = sf::Vector2f(
                static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y)
            );
            performDebugRaycast(raycastStart_, raycastEnd_);
            isMouseRaycasting_ = false;
        }
    }
}

void CollisionDebugger::performDebugRaycast(const sf::Vector2f& start, const sf::Vector2f& end) {
    auto hit = manager_.segmentIntersection(start, end);
    if (hit.valid) {
        debug_.addRaycastResult(hit);
        core::Logger::instance().info("[CollisionDebugger] Raycast hit entity " + 
            std::to_string(hit.entity ? hit.entity->id() : 0) + 
            " at distance " + std::to_string(hit.distance));
    }
}

void CollisionDebugger::updatePerformanceMetrics(float deltaTime) {
    frameTime_ = deltaTime;
    frameCount_++;
    
    static float fpsTimer = 0.f;
    fpsTimer += deltaTime;
    
    if (fpsTimer >= 1.f) {
        fps_ = static_cast<float>(frameCount_) / fpsTimer;
        frameCount_ = 0;
        fpsTimer = 0.f;
    }
}

// Debug Utilities Implementation
namespace DebugUtils {

sf::Color getLayerColor(std::uint32_t layer) {
    using entities::Entity;
    
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Player)) {
        return sf::Color::Blue;
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Enemy)) {
        return sf::Color::Red;
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Wall)) {
        return sf::Color::White;
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Item)) {
        return sf::Color::Yellow;
    } else {
        return sf::Color::Green; // Default
    }
}

std::string getLayerName(std::uint32_t layer) {
    using entities::Entity;
    
    if (layer == static_cast<std::uint32_t>(Entity::Layer::Player)) {
        return "Player";
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Enemy)) {
        return "Enemy";
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Wall)) {
        return "Wall";
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Item)) {
        return "Item";
    } else if (layer == static_cast<std::uint32_t>(Entity::Layer::Default)) {
        return "Default";
    } else {
        return "Unknown";
    }
}

std::string formatVector(const sf::Vector2f& vec) {
    std::ostringstream oss;
    oss << "(" << std::fixed << std::setprecision(1) << vec.x << "," << vec.y << ")";
    return oss.str();
}

std::string formatRect(const sf::FloatRect& rect) {
    std::ostringstream oss;
    oss << "(" << std::fixed << std::setprecision(1) 
        << rect.position.x << "," << rect.position.y 
        << " " << rect.size.x << "x" << rect.size.y << ")";
    return oss.str();
}

} // namespace DebugUtils

} // namespace collisions
