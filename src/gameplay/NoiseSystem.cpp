#include "NoiseSystem.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace gameplay {

NoiseSystem::NoiseSystem() {
    core::Logger::instance().info("[NoiseSystem] Initialized");
}

void NoiseSystem::update(float deltaTime) {
    // Remove expired noise events
    auto now = std::chrono::steady_clock::now();
    
    auto it = m_activeNoise.begin();
    while (it != m_activeNoise.end()) {
        auto timeSinceCreation = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->timestamp).count();
        
        if (timeSinceCreation > it->lifetime) {
            it = m_activeNoise.erase(it);
        } else {
            ++it;
        }
    }
}

void NoiseSystem::generateNoise(sf::Vector2f position, float intensity, const std::string& source) {
    NoiseEvent event;
    event.position = position;
    event.intensity = intensity;
    event.source = source;
    event.radius = intensity * 20.0f; // Simple calculation
    event.lifetime = 3000; // 3 seconds
    event.timestamp = std::chrono::steady_clock::now();
    
    m_activeNoise.push_back(event);
    
    if (m_debugMode) {
        core::Logger::instance().info("[NoiseSystem] Generated noise at (" + 
            std::to_string(position.x) + "," + std::to_string(position.y) + 
            ") intensity=" + std::to_string(intensity) + " source=" + source);
    }
}

void NoiseSystem::generatePlayerWalkingNoise(entities::Player* player) {
    if (!player) return;
    generateNoise(player->position(), 3.0f, "walking");
}

void NoiseSystem::generatePlayerRunningNoise(entities::Player* player) {
    if (!player) return;
    generateNoise(player->position(), 8.0f, "running");
}

void NoiseSystem::generatePlayerDoorNoise(sf::Vector2f position) {
    generateNoise(position, 5.0f, "door");
}

void NoiseSystem::generatePlayerCombatNoise(sf::Vector2f position) {
    generateNoise(position, 15.0f, "combat");
}

void NoiseSystem::setNoiseSurfaceMultiplier(const std::string& surface, float multiplier) {
    // Simple implementation - just log for now
    core::Logger::instance().info("[NoiseSystem] Surface multiplier set: " + surface + " = " + std::to_string(multiplier));
}

void NoiseSystem::renderDebug(sf::RenderWindow& window) const {
    if (!m_debugMode) return;
    
    for (const auto& noise : m_activeNoise) {
        sf::CircleShape circle(noise.radius);
        circle.setPosition(sf::Vector2f(noise.position.x - noise.radius, noise.position.y - noise.radius));
        circle.setFillColor(sf::Color(255, 255, 0, 50)); // Yellow with transparency
        circle.setOutlineColor(sf::Color::Yellow);
        circle.setOutlineThickness(1.0f);
        window.draw(circle);
    }
}

void NoiseSystem::clearAllNoise() {
    m_activeNoise.clear();
}

float NoiseSystem::calculateRadius(float intensity, const std::string& source) const {
    return intensity * 20.0f; // Simple calculation
}

int NoiseSystem::calculateLifetime(float intensity) const {
    return static_cast<int>(intensity * 300); // 300ms per intensity unit
}

} // namespace gameplay
