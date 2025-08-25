#include "CustomCursor.h"
#include "../core/Logger.h"
#include "../core/AssetManager.h"
#include <SFML/Window/Event.hpp>
#include <memory>

namespace ui {

CustomCursor::CustomCursor() {
    // Set default scale and offset
    m_scale = 1.0f;
    m_offset = sf::Vector2f(0.f, 0.f);
}

bool CustomCursor::loadTexture(const std::string& texturePath) {
    // Try to load through AssetManager first
    m_texture = AssetManager::instance().getTexture("cursor.jpg");
    
    if (!m_texture) {
        // Fallback: load directly from file
        m_texture = std::make_shared<sf::Texture>();
        if (!m_texture->loadFromFile(texturePath)) {
            core::Logger::instance().warning("CustomCursor: Failed to load texture from " + texturePath);
            m_textureLoaded = false;
            return false;
        }
    }

    // Configure sprite
    m_sprite = std::make_unique<sf::Sprite>(*m_texture);
    m_sprite->setScale(sf::Vector2f(m_scale, m_scale));
    
    // Set origin to center for better cursor behavior
    sf::Vector2u textureSize = m_texture->getSize();
    m_sprite->setOrigin(sf::Vector2f(textureSize.x / 2.0f, textureSize.y / 2.0f));
    
    m_textureLoaded = true;
    core::Logger::instance().info("CustomCursor: Texture loaded successfully");
    return true;
}

void CustomCursor::update(sf::RenderWindow& window) {
    if (!m_textureLoaded) return;

    updateVisibility(window);
    
    if (m_enabled && m_windowHasFocus) {
        // Get mouse position relative to window
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);
        
        // Apply offset and set sprite position
        m_sprite->setPosition(mousePosF + m_offset);
    }
}

void CustomCursor::render(sf::RenderWindow& window) {
    if (!m_textureLoaded || !m_enabled || !m_windowHasFocus) return;
    
    // Only render if mouse is within window bounds
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2u windowSize = window.getSize();
    
    if (mousePos.x >= 0 && mousePos.y >= 0 && 
        mousePos.x < static_cast<int>(windowSize.x) && 
        mousePos.y < static_cast<int>(windowSize.y)) {
        window.draw(*m_sprite);
    }
}

void CustomCursor::setEnabled(bool enabled) {
    if (m_enabled == enabled) return;
    
    m_enabled = enabled;
    
    if (enabled && m_textureLoaded) {
        core::Logger::instance().info("CustomCursor: Custom cursor enabled");
    } else {
        core::Logger::instance().info("CustomCursor: Custom cursor disabled");
    }
}

void CustomCursor::setScale(float scale) {
    m_scale = scale;
    if (m_textureLoaded && m_sprite) {
        m_sprite->setScale(sf::Vector2f(m_scale, m_scale));
    }
}

void CustomCursor::updateVisibility(sf::RenderWindow& window) {
    // Check if window has focus
    bool hadFocus = m_windowHasFocus;
    m_windowHasFocus = window.hasFocus();
    
    // Hide system cursor when custom cursor should be shown
    if (m_enabled && m_windowHasFocus && m_textureLoaded) {
        window.setMouseCursorVisible(false);
        if (!hadFocus && m_windowHasFocus) {
            core::Logger::instance().info("CustomCursor: System cursor hidden, custom cursor active");
        }
    } else {
        window.setMouseCursorVisible(true);
        if (hadFocus && !m_windowHasFocus) {
            core::Logger::instance().info("CustomCursor: System cursor restored");
        }
    }
}

} // namespace ui
