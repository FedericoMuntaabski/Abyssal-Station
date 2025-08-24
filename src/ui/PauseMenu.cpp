#include "PauseMenu.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../scene/SceneManager.h"
#include "UIManager.h"
#include "OptionsMenu.h"

#include "InputHelper.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace ui {

using input::InputManager;

PauseMenu::PauseMenu(scene::SceneManager* manager, ui::UIManager* uiManager)
    : Menu("PauseMenu"), m_manager(manager), m_uiManager(uiManager) {
    // Mirror MainMenu options with small utilities
    m_options = {"Resume", "Options", "Save", "Load", "Quit"};
    m_inputHelper = std::make_unique<InputHelper>();
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

void PauseMenu::updateActiveDevice() {
    if (m_inputHelper) m_activeDevice = m_inputHelper->detectActiveDevice();
}

void PauseMenu::handleMouseHover(sf::RenderWindow& window) {
    if (m_activeDevice != InputHelper::DeviceType::Mouse) return;

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);
    for (std::size_t i = 0; i < m_options.size(); ++i) {
        sf::Vector2f optionPos(120.f, m_startY + static_cast<float>(i) * m_spacing);
        sf::Vector2f optionSize(300.f, 30.f);
        sf::FloatRect bounds(optionPos, optionSize);
        if (bounds.contains(mousePosF)) {
            if (m_selected != i) m_selected = i;
            break;
        }
    }
}

std::string PauseMenu::getContextualHint() const {
    if (!m_inputHelper) return std::string();
    std::string confirmAction = m_inputHelper->getActionDisplayName(input::Action::Confirm, m_activeDevice);
    std::string cancelAction = m_inputHelper->getActionDisplayName(input::Action::Cancel, m_activeDevice);
    std::string moveActions = m_inputHelper->getActionDisplayName(input::Action::MoveUp, m_activeDevice) + 
                             "/" + m_inputHelper->getActionDisplayName(input::Action::MoveDown, m_activeDevice);
    return "Navigate: " + moveActions + "  Select: " + confirmAction + "  Back: " + cancelAction;
}

void PauseMenu::handleInput() {

    updateActiveDevice();
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selected == 0) m_selected = m_options.size() - 1;
        else --m_selected;
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_options.size();
    }

    // Mouse hover selection
    if (m_activeDevice == InputHelper::DeviceType::Mouse) {
        // We need a window to determine mouse position; rely on render-time hover check via handleMouseHover
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_options[m_selected];
        core::Logger::instance().info("PauseMenu selected: " + choice);
        if (choice == "Resume") {
            deactivate();
        } else if (choice == "Options") {
            if (m_uiManager) {
                core::ConfigManager* cfg = m_uiManager->getConfigManager();
                m_uiManager->pushMenu(new OptionsMenu(m_manager, cfg));
            } else {
                core::Logger::instance().info("Options menu not implemented yet.");
            }
        } else if (choice == "Save") {
            // Trigger save through UI events if bound
            if (m_uiManager) {
                m_uiManager->triggerSaveGame();
                m_uiManager->showToast("Game saved (placeholder)", 2.0f, sf::Color::Green);
            }
        } else if (choice == "Load") {
            if (m_uiManager) {
                m_uiManager->triggerLoadGame();
                m_uiManager->showToast("Game loaded (placeholder)", 2.0f, sf::Color::Cyan);
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
        fontLoaded = font.openFromFile("assets/fonts/Secundary_font.ttf");
    }

    handleMouseHover(window);

    // Render contextual hints similar to MainMenu
    if (!fontLoaded) return;

    std::string hint = getContextualHint();
    sf::Text hintText(font, hint, 14);
    hintText.setFillColor(sf::Color(150, 150, 150));
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    hintText.setPosition(sf::Vector2f(120.f, windowSize.y - 80.f));
    window.draw(hintText);

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        sf::Text text(font, m_options[i], 22);
        text.setFillColor(i == m_selected ? sf::Color::Cyan : sf::Color::White);
        text.setPosition(sf::Vector2f(120.f, m_startY + static_cast<float>(i) * m_spacing));
        window.draw(text);
    }
}

} // namespace ui
