#include "PauseMenu.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../core/AssetManager.h"
#include "../scene/SceneManager.h"
#include "UIManager.h"
#include "OptionsMenu.h"

#include "InputHelper.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>

namespace ui {

using input::InputManager;

PauseMenu::PauseMenu(scene::SceneManager* manager, ui::UIManager* uiManager)
    : Menu("PauseMenu"), m_manager(manager), m_uiManager(uiManager) {
    // Mirror MainMenu options with small utilities
    m_options = {"Resume", "Options", "Save", "Load", "Volver al menú principal"};
    m_inputHelper = std::make_unique<InputHelper>();
    m_scales.assign(m_options.size(), 1.0f);
    m_glowIntensity.assign(m_options.size(), 0.0f);
    loadAudio();
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
            if (m_selected != i) {
                m_selected = i;
                playHoverSound();
                core::Logger::instance().info("PauseMenu: Mouse hover selected option " + std::to_string(m_selected));
            }
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
        playHoverSound();
        core::Logger::instance().info("PauseMenu: Selected option " + std::to_string(m_selected));
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_options.size();
        playHoverSound();
        core::Logger::instance().info("PauseMenu: Selected option " + std::to_string(m_selected));
    }

    // Mouse hover selection
    if (m_activeDevice == InputHelper::DeviceType::Mouse) {
        // We need a window to determine mouse position; rely on render-time hover check via handleMouseHover
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_options[m_selected];
        playConfirmSound();
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
        } else if (choice == "Volver al menú principal") {
            // Go back to main menu instead of quitting
            if (m_uiManager) {
                m_uiManager->triggerReturnToMainMenu();
            }
            if (m_manager) {
                // Pop current scene to return to previous (should be main menu)
                m_manager->pop();
            }
        }
    }

    if (im.isActionJustPressed(input::Action::Cancel)) {
        // Cancel means resume from pause menu
        deactivate();
    }
}

void PauseMenu::update(float dt) {
    // Update scale animations with enhanced easing
    float speed = 10.0f;
    float targetScaleSelected = 1.2f;
    float glowSpeed = 8.0f;
    
    for (std::size_t i = 0; i < m_scales.size(); ++i) {
        float targetScale = (i == m_selected) ? targetScaleSelected : 1.0f;
        float targetGlow = (i == m_selected) ? 1.0f : 0.0f;
        
        m_scales[i] += (targetScale - m_scales[i]) * std::min(1.0f, speed * dt);
        if (i < m_glowIntensity.size()) {
            m_glowIntensity[i] += (targetGlow - m_glowIntensity[i]) * std::min(1.0f, glowSpeed * dt);
        }
    }
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
        sf::Vector2f position(120.f, m_startY + static_cast<float>(i) * m_spacing);
        bool selected = (i == m_selected);
        float scale = (i < m_scales.size()) ? m_scales[i] : 1.0f;
        float glow = (i < m_glowIntensity.size()) ? m_glowIntensity[i] : 0.0f;

        // Background highlight for selected option
        if (selected) {
            sf::RectangleShape highlight;
            highlight.setSize(sf::Vector2f(300.f * scale, 35.f * scale));
            highlight.setPosition(position + sf::Vector2f(-10.f, -5.f));
            std::uint8_t highlightAlpha = static_cast<std::uint8_t>(50 * glow);
            highlight.setFillColor(sf::Color(255, 100, 100, highlightAlpha)); // Red tint
            window.draw(highlight);
        }

        // Option text with glow and UTF-8 support
        sf::String textStr = sf::String::fromUtf8(m_options[i].begin(), m_options[i].end());
        sf::Text text(font, textStr, 22);
        text.setScale(sf::Vector2f(scale, scale));
        text.setFillColor(selected ? sf::Color::Cyan : sf::Color::White);
        text.setPosition(position);

        // Glow effect for selected option
        if (selected && glow > 0.1f) {
            sf::Text textGlow = text;
            std::uint8_t glowAlpha = static_cast<std::uint8_t>(150 * glow);
            textGlow.setFillColor(sf::Color(0, 255, 255, glowAlpha));
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    textGlow.setPosition(position + sf::Vector2f(static_cast<float>(dx), static_cast<float>(dy)));
                    window.draw(textGlow);
                }
            }
        }

        window.draw(text);
    }
}

void PauseMenu::loadAudio() {
    // Load sound effects
    m_hoverBuffer = AssetManager::instance().getSound("hover_select");
    if (m_hoverBuffer) {
        m_hoverSound = std::make_unique<sf::Sound>(*m_hoverBuffer);
        core::Logger::instance().info("PauseMenu: Hover sound loaded successfully");
    } else {
        core::Logger::instance().warning("PauseMenu: Failed to load hover sound");
    }
    
    m_confirmBuffer = AssetManager::instance().getSound("confirm");
    if (m_confirmBuffer) {
        m_confirmSound = std::make_unique<sf::Sound>(*m_confirmBuffer);
        core::Logger::instance().info("PauseMenu: Confirm sound loaded successfully");
    } else {
        core::Logger::instance().warning("PauseMenu: Failed to load confirm sound");
    }
    
    m_audioLoaded = true;
}

void PauseMenu::playHoverSound() {
    if (m_audioLoaded && m_hoverSound && m_hoverSound->getStatus() != sf::SoundSource::Status::Playing) {
        m_hoverSound->play();
    }
}

void PauseMenu::playConfirmSound() {
    if (m_audioLoaded && m_confirmSound) {
        m_confirmSound->play();
    }
}

} // namespace ui
