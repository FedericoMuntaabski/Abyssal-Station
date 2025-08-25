#include "VisionSystem.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

namespace gameplay {

const float PI = 3.14159265f;

VisionSystem::VisionSystem() {
    core::Logger::instance().info("[VisionSystem] Initialized");
}

bool VisionSystem::initialize(unsigned int windowWidth, unsigned int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    
    // Create a simple texture for vision effects
    if (!m_visionTexture.resize({windowWidth, windowHeight})) {
        core::Logger::instance().error("[VisionSystem] Failed to create vision texture");
        return false;
    }
    
    m_initialized = true;
    core::Logger::instance().info("[VisionSystem] Initialized successfully");
    return true;
}

void VisionSystem::update(float deltaTime, entities::Player* player) {
    if (!m_initialized || !player) return;
    
    // Update flashlight battery
    if (player->isFlashlightOn()) {
        m_flashlightBattery -= deltaTime * 2.0f; // Drain 2% per second
        if (m_flashlightBattery <= 0.0f) {
            m_flashlightBattery = 0.0f;
        }
    }
    
    // Store player position for rendering
    m_playerPosition = player->position();
    m_playerDirection = player->getFacingDirection();
    m_flashlightOn = player->isFlashlightOn() && m_flashlightBattery > 0.0f;
}

void VisionSystem::render(sf::RenderWindow& window, entities::Player* player) {
    if (!m_initialized || !player) return;
    
    // Simple implementation: render a dark overlay with a vision cone
    sf::RectangleShape darkOverlay;
    darkOverlay.setSize(sf::Vector2f(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)));
    darkOverlay.setFillColor(sf::Color(0, 0, 0, 150)); // Dark overlay
    window.draw(darkOverlay);
    
    // Create a simple vision cone
    if (m_flashlightOn) {
        sf::CircleShape visionCircle(100.0f); // Fixed radius for simplicity
        visionCircle.setPosition(sf::Vector2f(player->position().x - 100.0f, player->position().y - 100.0f));
        visionCircle.setFillColor(sf::Color(255, 255, 255, 100)); // White light
        window.draw(visionCircle);
    }
}

bool VisionSystem::canSee(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const {
    // Simple distance check for now
    float distance = std::sqrt(std::pow(targetPos.x - fromPos.x, 2) + std::pow(targetPos.y - fromPos.y, 2));
    return distance < 150.0f; // Simple vision range
}

bool VisionSystem::isInVisionCone(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const {
    return canSee(fromPos, fromAngle, targetPos);
}

bool VisionSystem::isInFlashlightBeam(entities::Player* player, sf::Vector2f targetPos) const {
    if (!player || !m_flashlightOn) return false;
    return canSee(player->position(), 0.0f, targetPos);
}

float VisionSystem::getVisibilityAt(sf::Vector2f position, entities::Player* player) const {
    if (!player) return 0.0f;
    
    float distance = std::sqrt(std::pow(position.x - player->position().x, 2) + std::pow(position.y - player->position().y, 2));
    
    if (m_flashlightOn && distance < 100.0f) {
        return 1.0f;
    } else if (distance < 50.0f) {
        return 0.5f;
    }
    return 0.1f;
}

void VisionSystem::addLightSource(sf::Vector2f position, float radius, sf::Color color, float intensity) {
    // Simple implementation - just log for now
    core::Logger::instance().info("[VisionSystem] Light source added at (" + 
        std::to_string(position.x) + "," + std::to_string(position.y) + ")");
}

void VisionSystem::removeLightSource(size_t index) {
    core::Logger::instance().info("[VisionSystem] Light source removed");
}

void VisionSystem::setAmbientLight(float level) {
    m_ambientLightLevel = level;
}

void VisionSystem::setDebugMode(bool enabled) {
    m_debugMode = enabled;
}

} // namespace gameplay
