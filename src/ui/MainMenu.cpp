#include "MainMenu.h"
#include "../core/Logger.h"
#include "../input/InputManager.h"
#include "../scene/SceneManager.h"
#include "UIManager.h"
#include "OptionsMenu.h"
#include "../scene/PlayScene.h"

#include <thread>
#include <chrono>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace ui {

using input::InputManager;

MainMenu::MainMenu(scene::SceneManager* manager, ui::UIManager* uiManager)
    : Menu("MainMenu"), m_manager(manager), m_uiManager(uiManager) {
    m_options = {"Start Game", "Options", "Exit"};
    m_scales.assign(m_options.size(), 1.0f);
}

void MainMenu::onEnter() {
    // Ensure first option selected when opened
    m_selected = 0;
}

void MainMenu::onExit() {
    // nothing for now
}

void MainMenu::handleInput() {
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
        core::Logger::instance().info("MainMenu selected: " + choice);
        if (choice == "Start Game") {
            if (m_manager) {
                // push PlayScene
                m_manager->push(std::make_unique<scene::PlayScene>(m_manager));
            }
        } else if (choice == "Options") {
            if (m_uiManager) {
                core::Logger::instance().info(std::string("MainMenu: opening OptionsMenu; m_manager=") + (m_manager ? "valid" : "null") + std::string(" m_uiManager=valid"));
                // open OptionsMenu on top
                m_uiManager->pushMenu(new OptionsMenu(m_manager));
                core::Logger::instance().info("MainMenu: OptionsMenu push completed -- now sleeping briefly for diagnostics");
                // brief sleep to let async issues surface and provide a clearer log window
                try {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    core::Logger::instance().info("MainMenu: wake after sleep");
                } catch (...) {}
            } else {
                core::Logger::instance().info("Options menu not implemented yet.");
            }
        } else if (choice == "Exit") {
            // For now, exit game by popping all scenes or similar
            if (m_manager) {
                // Pop current scene so the game can close or return
                m_manager->pop();
            }
        }
    }
}

void MainMenu::update(float dt) {
    float speed = 8.0f;
    float targetScaleSelected = 1.12f;
    for (std::size_t i = 0; i < m_scales.size(); ++i) {
        float target = (i == m_selected) ? targetScaleSelected : 1.0f;
        m_scales[i] += (target - m_scales[i]) * std::min(1.0f, speed * dt);
    }
}

void MainMenu::render(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        // Try to load a font from assets/fonts
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    // Determine mouse hover selection: if cursor is over a text item, update m_selected
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    for (std::size_t i = 0; i < m_options.size(); ++i) {
        sf::Text text(font, m_options[i], 24);
        float s = (i < m_scales.size()) ? m_scales[i] : 1.0f;
        text.setScale(sf::Vector2f(s, s));
        text.setFillColor(i == m_selected ? sf::Color::Yellow : sf::Color::White);
        text.setPosition(sf::Vector2f(100.f, m_startY + static_cast<float>(i) * m_spacing));

        auto bounds = text.getGlobalBounds();
        if (bounds.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
            m_selected = static_cast<int>(i);
        }

        window.draw(text);
    }
}

} // namespace ui
