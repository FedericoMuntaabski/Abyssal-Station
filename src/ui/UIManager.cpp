#include "UIManager.h"
#include "../core/ConfigManager.h"
#include "../core/Logger.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace ui {

UIManager::UIManager() {
    // Try to load default font
    if (std::filesystem::exists("assets/fonts/Long_Shot.ttf")) {
        if (m_font.openFromFile("assets/fonts/Long_Shot.ttf")) {
            m_fontLoaded = true;
            core::Logger::instance().info("UIManager: Default font loaded");
        }
    }
}

UIManager::~UIManager() {
    clear();
}

// Enhanced menu management
void UIManager::pushMenu(Menu* menu, AnimationType animation) {
    if (!menu) return;
    
    MenuEntry entry;
    entry.menu = std::unique_ptr<Menu>(menu);
    entry.enterAnimation = animation;
    entry.animationDuration = 0.3f;
    entry.isEntering = (animation != AnimationType::None);
    entry.animationTime = 0.0f;
    
    // activate() already calls onEnter() via Menu::activate(), avoid double-calling lifecycle hooks
    menu->activate();
    
    core::Logger::instance().info(std::string("UIManager: push menu ") + menu->name());
    m_menus.push_back(std::move(entry));
}

void UIManager::popMenu(AnimationType animation) {
    if (m_menus.empty()) return;
    
    auto& entry = m_menus.back();
    Menu* top = entry.menu.get();
    
    if (animation != AnimationType::None) {
        entry.exitAnimation = animation;
        entry.isExiting = true;
        entry.animationTime = 0.0f;
        // Mark for exit; let deactivate() handle onExit() to avoid duplicate calls
        top->deactivate();
    } else {
        // deactivate() will call onExit()
        top->deactivate();
        core::Logger::instance().info(std::string("UIManager: pop menu ") + top->name());
        m_menus.pop_back();
    }
}

void UIManager::replaceMenu(Menu* menu, AnimationType animation) {
    if (!menu) return;
    
    if (!m_menus.empty()) {
        popMenu(animation);
    }
    pushMenu(menu, animation);
}

void UIManager::insertMenuBelow(Menu* menu, size_t index) {
    if (!menu || index >= m_menus.size()) return;
    
    MenuEntry entry;
    entry.menu = std::unique_ptr<Menu>(menu);
    entry.enterAnimation = AnimationType::None;
    
    m_menus.insert(m_menus.begin() + index, std::move(entry));
    core::Logger::instance().info(std::string("UIManager: inserted menu ") + menu->name() + " at index " + std::to_string(index));
}

Menu* UIManager::currentMenu() const noexcept {
    if (m_menus.empty()) return nullptr;
    return m_menus.back().menu.get();
}

bool UIManager::isMenuActive(const std::string& name) const noexcept {
    for (const auto& entry : m_menus) {
        if (entry.menu && entry.menu->name() == name && entry.menu->isActive()) {
            return true;
        }
    }
    return false;
}

bool UIManager::isAnyMenuActive() const noexcept {
    for (const auto& entry : m_menus) {
        if (entry.menu && entry.menu->isActive()) {
            return true;
        }
    }
    return false;
}

size_t UIManager::menuStackSize() const noexcept {
    return m_menus.size();
}

void UIManager::clear() {
    for (auto& entry : m_menus) {
        if (entry.menu) {
            // deactivate() will call onExit() if active
            entry.menu->deactivate();
        }
    }
    m_menus.clear();
    core::Logger::instance().info("UIManager: cleared all menus");
}

// Enhanced notifications
void UIManager::showNotification(const std::string& text, NotificationPriority priority, 
                                float duration, sf::Color color) {
    Notification n;
    n.text = text;
    n.elapsed = 0.f;
    n.lifetime = duration;
    n.iconColor = color;
    n.priority = priority;
    n.isToast = false;
    
    // Insert based on priority
    auto insertPos = m_notifications.end();
    for (auto it = m_notifications.begin(); it != m_notifications.end(); ++it) {
        if (it->priority < priority) {
            insertPos = it;
            break;
        }
    }
    
    m_notifications.insert(insertPos, std::move(n));
    core::Logger::instance().info("UIManager: notification added - " + text);
}

void UIManager::showToast(const std::string& text, float duration, sf::Color color) {
    Notification n;
    n.text = text;
    n.elapsed = 0.f;
    n.lifetime = duration;
    n.iconColor = color;
    n.priority = NotificationPriority::Normal;
    n.isToast = true;
    
    m_notifications.push_back(std::move(n));
}

// Legacy notification methods
void UIManager::notifyItemCollected(std::uint32_t id) {
    notifyItemCollected(id, sf::Color::Yellow);
}

void UIManager::notifyItemCollected(std::uint32_t id, sf::Color iconColor) {
    showNotification("Item collected: #" + std::to_string(id), NotificationPriority::Normal, 
                    m_notificationDuration, iconColor);
}

void UIManager::notifyPuzzleCompleted(std::uint32_t id) {
    notifyPuzzleCompleted(id, sf::Color::Green);
}

void UIManager::notifyPuzzleCompleted(std::uint32_t id, sf::Color iconColor) {
    showNotification("Puzzle completed: #" + std::to_string(id), NotificationPriority::High, 
                    m_notificationDuration, iconColor);
}

void UIManager::notifyAchievement(const std::string& title, const std::string& description) {
    showNotification("Achievement: " + title + " - " + description, NotificationPriority::High, 
                    5.0f, sf::Color(255, 215, 0)); // Gold color
}

// Font and styling
void UIManager::setFont(const sf::Font& font) {
    m_font = font;
    m_fontLoaded = true;
    core::Logger::instance().info("UIManager: custom font set");
}

void UIManager::setTheme(const std::string& themeName) {
    m_currentTheme = themeName;
    core::Logger::instance().info("UIManager: theme changed to " + themeName);
}

// Event triggers
void UIManager::triggerStartGame() {
    if (m_events.onStartGame) {
        m_events.onStartGame();
        core::Logger::instance().info("UIManager: triggered start game event");
    }
}

void UIManager::triggerExit() {
    if (m_events.onExit) {
        m_events.onExit();
        core::Logger::instance().info("UIManager: triggered exit event");
    }
}

void UIManager::triggerVolumeChanged(int volume) {
    if (m_events.onVolumeChanged) {
        m_events.onVolumeChanged(volume);
        core::Logger::instance().info("UIManager: triggered volume changed event: " + std::to_string(volume));
    }
}

void UIManager::triggerLanguageChanged(const std::string& language) {
    if (m_events.onLanguageChanged) {
        m_events.onLanguageChanged(language);
        core::Logger::instance().info("UIManager: triggered language changed event: " + language);
    }
}

void UIManager::triggerResolutionChanged(int width, int height) {
    if (m_events.onResolutionChanged) {
        m_events.onResolutionChanged(width, height);
        core::Logger::instance().info("UIManager: triggered resolution changed event: " + 
                                     std::to_string(width) + "x" + std::to_string(height));
    }
}

void UIManager::triggerBindingChanged() {
    if (m_events.onBindingChanged) {
        m_events.onBindingChanged();
        core::Logger::instance().info("UIManager: triggered binding changed event");
    }
}

void UIManager::triggerSaveGame() {
    if (m_events.onSaveGame) {
        m_events.onSaveGame();
        core::Logger::instance().info("UIManager: triggered save game event");
    }
}

void UIManager::triggerLoadGame() {
    if (m_events.onLoadGame) {
        m_events.onLoadGame();
        core::Logger::instance().info("UIManager: triggered load game event");
    }
}

void UIManager::update(float dt) {
    if (m_paused) return;
    
    updateAnimations(dt);
    
    // Remove completed exit animations
    m_menus.erase(std::remove_if(m_menus.begin(), m_menus.end(), 
        [this](const MenuEntry& entry) {
            if (entry.isExiting && entry.animationTime >= entry.animationDuration) {
                core::Logger::instance().info(std::string("UIManager: removed exited menu ") + entry.menu->name());
                return true;
            }
            return false;
        }), m_menus.end());
    
    // Update active menus (only top menu handles input to prevent conflicts)
    for (size_t i = 0; i < m_menus.size(); ++i) {
        auto& entry = m_menus[i];
        if (entry.menu && entry.menu->isActive()) {
            // Only the top menu handles input
            if (i == m_menus.size() - 1 && !entry.isExiting) {
                entry.menu->handleInput();
            }
            entry.menu->update(dt);
        }
    }

    // Advance notifications elapsed and remove expired
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->elapsed += dt;
        if (it->elapsed >= it->lifetime) {
            it = m_notifications.erase(it);
        } else {
            ++it;
        }
    }
}

void UIManager::render(sf::RenderWindow& window) {
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Render menus with animations
    for (const auto& entry : m_menus) {
        if (entry.menu && entry.menu->isActive()) {
            renderMenuWithAnimation(window, entry);
        }
    }

    // Render enhanced notifications
    if (!m_notifications.empty() && m_fontLoaded) {
        float y = 10.f;
        const float maxWidth = windowSize.x * 0.3f;
        
        for (const auto& n : m_notifications) {
            float t = std::clamp(n.elapsed / n.lifetime, 0.0f, 1.0f);
            uint8_t alpha = static_cast<uint8_t>(255 * (1.0f - t));
            
            // Calculate position based on type
            float x = n.isToast ? windowSize.x - maxWidth - 10.f : 10.f;
            
            // Background for better readability
            sf::RectangleShape bg;
            bg.setSize(sf::Vector2f(maxWidth, 30.f));
            bg.setPosition(sf::Vector2f(x - 5.f, y - 5.f));
            bg.setFillColor(sf::Color(0, 0, 0, alpha / 2));
            window.draw(bg);

            // Priority indicator (colored bar on the left)
            sf::RectangleShape priorityBar;
            priorityBar.setSize(sf::Vector2f(3.f, 30.f));
            priorityBar.setPosition(sf::Vector2f(x - 5.f, y - 5.f));
            sf::Color priorityColor = sf::Color::White;
            switch (n.priority) {
                case NotificationPriority::Low: priorityColor = sf::Color::Blue; break;
                case NotificationPriority::Normal: priorityColor = sf::Color::White; break;
                case NotificationPriority::High: priorityColor = sf::Color::Yellow; break;
                case NotificationPriority::Critical: priorityColor = sf::Color::Red; break;
            }
            priorityColor.a = alpha;
            priorityBar.setFillColor(priorityColor);
            window.draw(priorityBar);

            // Draw icon
            sf::RectangleShape icon(sf::Vector2f(n.iconSize, n.iconSize));
            sf::Color iconColor = n.iconColor;
            iconColor.a = alpha;
            icon.setFillColor(iconColor);
            icon.setPosition(sf::Vector2f(x, y));
            window.draw(icon);

            // Draw text
            sf::Text txt(m_font, n.text, 14);
            sf::Color textColor = sf::Color::White;
            textColor.a = alpha;
            txt.setFillColor(textColor);
            txt.setPosition(sf::Vector2f(x + n.iconSize + 6.f, y - 2.f));
            window.draw(txt);
            
            y += 35.f;
        }
    }
}

// Animation helpers implementation
void UIManager::updateAnimations(float dt) {
    for (auto& entry : m_menus) {
        if (entry.isEntering || entry.isExiting) {
            entry.animationTime += dt * m_animationSpeed;
            if (entry.animationTime >= entry.animationDuration) {
                entry.animationTime = entry.animationDuration;
                entry.isEntering = false;
                // isExiting entries will be removed in update()
            }
        }
    }
}

void UIManager::renderMenuWithAnimation(sf::RenderWindow& window, const MenuEntry& entry) {
    if (!entry.menu) return;
    
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    sf::Transform transform = getAnimationTransform(entry, windowSize);
    
    // Apply animation transform
    sf::RenderStates states;
    states.transform = transform;
    
    // For complex animations, we'd need to render to a texture first
    // For now, just apply basic transforms directly
    if (entry.isEntering || entry.isExiting) {
        // Create a view for the transformed rendering
        sf::View originalView = window.getView();
        sf::View animatedView = originalView;
        
        float progress = getAnimationProgress(entry);
        
        // Apply animation effects
        if ((entry.isEntering && entry.enterAnimation == AnimationType::Fade) || 
            (entry.isExiting && entry.exitAnimation == AnimationType::Fade)) {
            // For fade, we'll just render normally but could implement alpha blending
            entry.menu->render(window);
        } else {
            // For other animations, apply the transform
            window.setView(animatedView);
            entry.menu->render(window);
            window.setView(originalView);
        }
    } else {
        entry.menu->render(window);
    }
}

float UIManager::getAnimationProgress(const MenuEntry& entry) const {
    if (entry.animationDuration <= 0.0f) return 1.0f;
    return std::clamp(entry.animationTime / entry.animationDuration, 0.0f, 1.0f);
}

sf::Transform UIManager::getAnimationTransform(const MenuEntry& entry, const sf::Vector2f& windowSize) const {
    sf::Transform transform;
    float progress = getAnimationProgress(entry);
    
    if (entry.isExiting) {
        progress = 1.0f - progress; // Reverse for exit
    }
    
    AnimationType animType = entry.isEntering ? entry.enterAnimation : entry.exitAnimation;
    
    switch (animType) {
        case AnimationType::Slide: {
            float offset = windowSize.x * (1.0f - progress);
            transform.translate(sf::Vector2f(offset, 0.0f));
            break;
        }
        case AnimationType::Scale: {
            float scale = progress;
            transform.scale(sf::Vector2f(scale, scale), sf::Vector2f(windowSize.x * 0.5f, windowSize.y * 0.5f));
            break;
        }
        case AnimationType::Fade:
        case AnimationType::None:
        default:
            break;
    }
    
    return transform;
}

} // namespace ui
