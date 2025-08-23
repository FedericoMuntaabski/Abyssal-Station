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
    // Store the menu first so it has an owner during activation. If activation
    // throws, remove it and log the error instead of letting the app terminate.
    try {
        core::Logger::instance().info("UIManager: about to emplace new menu");
    } catch(...) {}
    m_menus.emplace_back(menu);
    try {
        try { core::Logger::instance().info(std::string("UIManager: activating menu instance at ") + std::to_string(reinterpret_cast<std::uintptr_t>(m_menus.back().get()))); } catch(...) {}
        m_menus.back()->activate();
        try { core::Logger::instance().info(std::string("UIManager: push menu ") + m_menus.back()->name()); } catch(...) {}
        try { core::Logger::instance().info(std::string("UIManager: menu active state = ") + (m_menus.back()->isActive() ? "true" : "false")); } catch(...) {}
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("UIManager: exception activating menu: ") + e.what()); } catch(...) {}
        m_menus.pop_back();
    } catch (...) {
        try { core::Logger::instance().error("UIManager: unknown exception activating menu"); } catch(...) {}
        m_menus.pop_back();
    }
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
    try {
    core::Logger::instance().info(std::string("UIManager: update start; menus=") + std::to_string(m_menus.size()));
    for (auto& m : m_menus) {
        if (m && m->isActive()) {
            try {
                core::Logger::instance().info(std::string("UIManager: updating menu ") + m->name());
            } catch(...) {}
            try {
                m->handleInput();
            } catch (const std::exception& e) {
                try { core::Logger::instance().error(std::string("UIManager: menu handleInput exception: ") + e.what()); } catch(...) {}
                continue;
            } catch (...) {
                try { core::Logger::instance().error("UIManager: menu handleInput unknown exception"); } catch(...) {}
                continue;
            }

            try {
                m->update(dt);
            } catch (const std::exception& e) {
                try { core::Logger::instance().error(std::string("UIManager: menu update exception: ") + e.what()); } catch(...) {}
            } catch (...) {
                try { core::Logger::instance().error("UIManager: menu update unknown exception"); } catch(...) {}
            }
        }
    }
    core::Logger::instance().info("UIManager: update end");
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("UIManager::update exception: ") + e.what()); } catch(...) {}
    } catch (...) {
        try { core::Logger::instance().error("UIManager::update unknown exception"); } catch(...) {}
    }

    // Advance notifications elapsed and remove expired
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->elapsed += dt;
        if (it->elapsed >= it->lifetime) it = m_notifications.erase(it);
        else ++it;
    }
}

void UIManager::render(sf::RenderWindow& window) {
    try {
    core::Logger::instance().info(std::string("UIManager: render start; menus=") + std::to_string(m_menus.size()));
    for (auto& m : m_menus) {
        if (m && m->isActive()) {
            try {
                core::Logger::instance().info(std::string("UIManager: rendering menu ") + m->name());
            } catch(...) {}
            try {
                m->render(window);
            } catch (const std::exception& e) {
                try { core::Logger::instance().error(std::string("UIManager: menu render exception: ") + e.what()); } catch(...) {}
            } catch (...) {
                try { core::Logger::instance().error("UIManager: menu render unknown exception"); } catch(...) {}
            }
        }
    }
    core::Logger::instance().info("UIManager: render end");
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("UIManager::render exception: ") + e.what()); } catch(...) {}
    } catch (...) {
        try { core::Logger::instance().error("UIManager::render unknown exception"); } catch(...) {}
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
