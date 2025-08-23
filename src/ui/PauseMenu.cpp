#include "PauseMenu.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../scene/SceneManager.h"
#include "UIManager.h"
#include "OptionsMenu.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace ui {

using input::InputManager;

PauseMenu::PauseMenu(scene::SceneManager* manager, ui::UIManager* uiManager)
    : Menu("PauseMenu"), m_manager(manager), m_uiManager(uiManager) {
    m_options = {"Resume", "Options", "Quit"};
}

void PauseMenu::onEnter() {
    m_selected = 0;
    if (m_uiManager) m_uiManager->setPaused(true);
}

void PauseMenu::onExit() {
    if (m_uiManager) m_uiManager->setPaused(false);
}

void PauseMenu::processToggleInput() {
    auto& im = InputManager::getInstance();
    if (im.isActionJustPressed(input::Action::Pause)) {
        if (isActive()) deactivate();
        else activate();
    }
}

void PauseMenu::handleInput() {
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selected == 0) m_selected = m_options.size() - 1;
        else --m_selected;
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_options.size();
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_options[m_selected];
        core::Logger::instance().info("PauseMenu selected: " + choice);
        if (choice == "Resume") {
            deactivate();
        } else if (choice == "Options") {
            if (m_uiManager) {
                m_uiManager->pushMenu(new OptionsMenu(m_manager));
            } else {
                core::Logger::instance().info("Options menu not implemented yet.");
            }
        } else if (choice == "Quit") {
            if (m_manager) {
                // Pop current scene to attempt to quit to previous
                m_manager->pop();
            }
        }
    }
}

void PauseMenu::update(float dt) {
    (void)dt;
}

void PauseMenu::render(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        sf::Text text(font, m_options[i], 22);
        text.setFillColor(i == m_selected ? sf::Color::Cyan : sf::Color::White);
        text.setPosition(sf::Vector2f(120.f, m_startY + static_cast<float>(i) * m_spacing));
        window.draw(text);
    }
}

} // namespace ui
