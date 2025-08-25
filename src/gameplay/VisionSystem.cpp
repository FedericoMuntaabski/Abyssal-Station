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
    
    // Get player's view (camera position relative to player)
    sf::View currentView = window.getView();
    sf::Vector2f viewCenter = currentView.getCenter();
    sf::Vector2f viewSize = currentView.getSize();
    
    // Create dark overlay covering the entire view
    sf::RectangleShape darkOverlay;
    darkOverlay.setSize(viewSize);
    darkOverlay.setPosition(viewCenter - viewSize / 2.0f);
    darkOverlay.setFillColor(sf::Color(0, 0, 0, 200)); // Very dark overlay
    window.draw(darkOverlay);
    
    // Create vision cone (65 degrees)
    sf::Vector2f playerPos = player->position();
    sf::Vector2f facingDir = player->getFacingDirection();
    
    // Normalize facing direction
    float length = std::sqrt(facingDir.x * facingDir.x + facingDir.y * facingDir.y);
    if (length > 0.001f) {
        facingDir.x /= length;
        facingDir.y /= length;
    } else {
        facingDir = sf::Vector2f(0.0f, -1.0f); // Default facing up
    }
    
    // Vision cone parameters
    const float VISION_RANGE = 200.0f;
    const float CONE_ANGLE_DEGREES = 65.0f;
    const float CONE_ANGLE_RADIANS = CONE_ANGLE_DEGREES * PI / 180.0f;
    const int CONE_SEGMENTS = 32;
    
    // Create vision cone shape
    sf::VertexArray visionCone(sf::PrimitiveType::TriangleFan, CONE_SEGMENTS + 2);
    visionCone[0].position = playerPos;
    visionCone[0].color = sf::Color(255, 255, 255, 80); // Semi-transparent white
    
    // Calculate cone vertices
    float baseAngle = std::atan2(facingDir.y, facingDir.x);
    for (int i = 0; i <= CONE_SEGMENTS; ++i) {
        float angle = baseAngle - CONE_ANGLE_RADIANS / 2.0f + (CONE_ANGLE_RADIANS * i) / CONE_SEGMENTS;
        float x = playerPos.x + VISION_RANGE * std::cos(angle);
        float y = playerPos.y + VISION_RANGE * std::sin(angle);
        
        visionCone[i + 1].position = sf::Vector2f(x, y);
        visionCone[i + 1].color = sf::Color(255, 255, 255, 0); // Fade to transparent at edges
    }
    
    // Set blend mode to lighten (subtract darkness)
    sf::RenderStates states;
    states.blendMode = sf::BlendAdd;
    window.draw(visionCone, states);
    
    // Additional flashlight beam if flashlight is on
    if (m_flashlightOn) {
        const float FLASHLIGHT_RANGE = 150.0f;
        const float FLASHLIGHT_ANGLE_DEGREES = 45.0f;
        const float FLASHLIGHT_ANGLE_RADIANS = FLASHLIGHT_ANGLE_DEGREES * PI / 180.0f;
        
        sf::VertexArray flashlightBeam(sf::PrimitiveType::TriangleFan, CONE_SEGMENTS + 2);
        flashlightBeam[0].position = playerPos;
        flashlightBeam[0].color = sf::Color(255, 255, 200, 120); // Warm white light
        
        for (int i = 0; i <= CONE_SEGMENTS; ++i) {
            float angle = baseAngle - FLASHLIGHT_ANGLE_RADIANS / 2.0f + (FLASHLIGHT_ANGLE_RADIANS * i) / CONE_SEGMENTS;
            float x = playerPos.x + FLASHLIGHT_RANGE * std::cos(angle);
            float y = playerPos.y + FLASHLIGHT_RANGE * std::sin(angle);
            
            flashlightBeam[i + 1].position = sf::Vector2f(x, y);
            flashlightBeam[i + 1].color = sf::Color(255, 255, 200, 0);
        }
        
        window.draw(flashlightBeam, states);
    }
}

bool VisionSystem::canSee(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const {
    // Calculate distance
    float distance = std::sqrt(std::pow(targetPos.x - fromPos.x, 2) + std::pow(targetPos.y - fromPos.y, 2));
    if (distance > 200.0f) return false; // Beyond vision range
    
    // Calculate angle to target
    sf::Vector2f toTarget = targetPos - fromPos;
    float targetAngle = std::atan2(toTarget.y, toTarget.x);
    
    // Normalize angles to [-PI, PI]
    auto normalizeAngle = [](float angle) {
        while (angle > PI) angle -= 2 * PI;
        while (angle < -PI) angle += 2 * PI;
        return angle;
    };
    
    fromAngle = normalizeAngle(fromAngle);
    targetAngle = normalizeAngle(targetAngle);
    
    // Calculate angle difference
    float angleDiff = normalizeAngle(targetAngle - fromAngle);
    
    // Check if within 65-degree cone (32.5 degrees on each side)
    const float HALF_CONE_ANGLE = (65.0f * PI / 180.0f) / 2.0f;
    return std::abs(angleDiff) <= HALF_CONE_ANGLE;
}

bool VisionSystem::isInVisionCone(sf::Vector2f fromPos, float fromAngle, sf::Vector2f targetPos) const {
    return canSee(fromPos, fromAngle, targetPos);
}

bool VisionSystem::isInFlashlightBeam(entities::Player* player, sf::Vector2f targetPos) const {
    if (!player || !m_flashlightOn) return false;
    
    sf::Vector2f facingDir = player->getFacingDirection();
    float facingAngle = std::atan2(facingDir.y, facingDir.x);
    
    // Calculate distance
    float distance = std::sqrt(std::pow(targetPos.x - player->position().x, 2) + std::pow(targetPos.y - player->position().y, 2));
    if (distance > 150.0f) return false; // Flashlight range
    
    // Calculate angle to target
    sf::Vector2f toTarget = targetPos - player->position();
    float targetAngle = std::atan2(toTarget.y, toTarget.x);
    
    // Normalize angles
    auto normalizeAngle = [](float angle) {
        while (angle > PI) angle -= 2 * PI;
        while (angle < -PI) angle += 2 * PI;
        return angle;
    };
    
    facingAngle = normalizeAngle(facingAngle);
    targetAngle = normalizeAngle(targetAngle);
    
    // Calculate angle difference
    float angleDiff = normalizeAngle(targetAngle - facingAngle);
    
    // Check if within 45-degree flashlight cone (22.5 degrees on each side)
    const float HALF_FLASHLIGHT_ANGLE = (45.0f * PI / 180.0f) / 2.0f;
    return std::abs(angleDiff) <= HALF_FLASHLIGHT_ANGLE;
}

float VisionSystem::getVisibilityAt(sf::Vector2f position, entities::Player* player) const {
    if (!player) return m_ambientLightLevel;
    
    sf::Vector2f playerPos = player->position();
    sf::Vector2f facingDir = player->getFacingDirection();
    float facingAngle = std::atan2(facingDir.y, facingDir.x);
    
    float distance = std::sqrt(std::pow(position.x - playerPos.x, 2) + std::pow(position.y - playerPos.y, 2));
    
    // Check if in vision cone
    bool inVisionCone = canSee(playerPos, facingAngle, position);
    
    // Check if in flashlight beam
    bool inFlashlight = isInFlashlightBeam(player, position);
    
    // Calculate visibility based on distance and cone
    float visibility = m_ambientLightLevel;
    
    if (inVisionCone) {
        // Base vision visibility decreases with distance
        float visionFactor = std::max(0.0f, 1.0f - distance / 200.0f);
        visibility = std::max(visibility, 0.3f + 0.4f * visionFactor);
    }
    
    if (inFlashlight) {
        // Flashlight provides bright illumination
        float flashlightFactor = std::max(0.0f, 1.0f - distance / 150.0f);
        visibility = std::max(visibility, 0.7f + 0.3f * flashlightFactor);
    }
    
    return std::min(1.0f, visibility);
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
