#include "UIManager.h"
#include "../core/Logger.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <filesystem>

namespace ui {

UIManager::~UIManager() {
    clear();
}

// Simple transient notification system
void UIManager::notifyItemCollected(std::uint32_t id) {
    notifyItemCollected(id, sf::Color::Yellow);
}

void UIManager::notifyItemCollected(std::uint32_t id, sf::Color iconColor) {
    core::Logger::instance().info(std::string("UIManager: item collected id=") + std::to_string(id));
    Notification n; n.text = std::string("Item collected: #") + std::to_string(id); n.elapsed = 0.f; n.lifetime = m_notificationDuration; n.iconColor = iconColor; n.iconSize = 12.f;
    m_notifications.push_back(std::move(n));
}

void UIManager::notifyPuzzleCompleted(std::uint32_t id) {
    notifyPuzzleCompleted(id, sf::Color::Green);
}

void UIManager::notifyPuzzleCompleted(std::uint32_t id, sf::Color iconColor) {
    core::Logger::instance().info(std::string("UIManager: puzzle completed id=") + std::to_string(id));
    Notification n; n.text = std::string("Puzzle completed: #") + std::to_string(id); n.elapsed = 0.f; n.lifetime = m_notificationDuration; n.iconColor = iconColor; n.iconSize = 12.f;
    m_notifications.push_back(std::move(n));
}

void UIManager::pushMenu(Menu* menu) {
    if (!menu) return;
    menu->activate();
    core::Logger::instance().info(std::string("UIManager: push menu ") + menu->name());
    m_menus.emplace_back(menu);
}

void UIManager::popMenu() {
    if (m_menus.empty()) return;
    Menu* top = m_menus.back().get();
    top->deactivate();
    core::Logger::instance().info(std::string("UIManager: pop menu ") + top->name());
    m_menus.pop_back();
}

Menu* UIManager::currentMenu() const noexcept {
    if (m_menus.empty()) return nullptr;
    return m_menus.back().get();
}

bool UIManager::isMenuActive(const std::string& name) const noexcept {
    if (m_menus.empty()) return false;
    const Menu* top = m_menus.back().get();
    return top && top->isActive() && top->name() == name;
}

bool UIManager::isAnyMenuActive() const noexcept {
    for (const auto& m : m_menus) {
        if (m && m->isActive()) return true;
    }
    return false;
}

void UIManager::update(float dt) {
    for (auto& m : m_menus) {
        if (m->isActive()) {
            m->handleInput();
            m->update(dt);
        }
    }

    // Advance notifications elapsed and remove expired
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->elapsed += dt;
        if (it->elapsed >= it->lifetime) it = m_notifications.erase(it);
        else ++it;
    }
}

void UIManager::render(sf::RenderWindow& window) {
    for (auto& m : m_menus) {
        if (m->isActive()) {
            m->render(window);
        }
    }

    // Render transient notifications on top-left with fade-out
    if (!m_notifications.empty()) {
        static sf::Font font;
        static bool fontLoaded = false;
        if (!fontLoaded) {
            std::string fontPath = "assets/fonts/Long_Shot.ttf";
            if (font.openFromFile(fontPath)) {
                fontLoaded = true;
            }
        }

        // Draw notifications using elapsed to compute fade
        float y = 10.f;
        for (const auto& n : m_notifications) {
            float t = (n.elapsed / n.lifetime);
            if (t > 1.f) t = 1.f;
            uint8_t alpha = static_cast<uint8_t>(255 * (1.f - t));

            // Draw icon rectangle
            sf::RectangleShape icon(sf::Vector2f(n.iconSize, n.iconSize));
            icon.setFillColor(sf::Color(n.iconColor.r, n.iconColor.g, n.iconColor.b, alpha));
            icon.setPosition(sf::Vector2f(10.f, y));
            window.draw(icon);

            // Draw text to the right of icon
            if (fontLoaded) {
                sf::Text txt(font, n.text, 14);
                txt.setFillColor(sf::Color(255,255,255, alpha));
                txt.setPosition(sf::Vector2f(10.f + n.iconSize + 6.f, y - 2.f));
                window.draw(txt);
            }
            y += std::max(18.f, n.iconSize + 4.f);
        }
    }
}

void UIManager::clear() {
    while (!m_menus.empty()) {
        Menu* top = m_menus.back().get();
        top->deactivate();
        core::Logger::instance().info(std::string("UIManager: clear menu ") + top->name());
        m_menus.pop_back();
    }
}

} // namespace ui
