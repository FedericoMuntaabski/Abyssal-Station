#include "UIManager.h"
#include "../core/Logger.h"

namespace ui {

UIManager::~UIManager() {
    clear();
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
}

void UIManager::render(sf::RenderWindow& window) {
    for (auto& m : m_menus) {
        if (m->isActive()) {
            m->render(window);
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
