#include "MainMenu.h"
#include "InputHelper.h"
#include "../core/Logger.h"
#include "../input/InputManager.h"
#include "../scene/SceneManager.h"
#include "UIManager.h"
#include "OptionsMenu.h"
#include "../scene/PlayScene.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <cmath>

namespace ui {

using input::InputManager;

MainMenu::MainMenu(scene::SceneManager* manager, ui::UIManager* uiManager)
    : Menu("MainMenu"), m_manager(manager), m_uiManager(uiManager) {
    
    m_inputHelper = std::make_unique<InputHelper>();
    m_options = {"Jugar (Solo)", "Crear Sala", "Opciones", "Salir"};
    m_scales.assign(m_options.size(), 1.0f);
    m_glowIntensity.assign(m_options.size(), 0.0f);
}

void MainMenu::onEnter() {
    // Ensure first option selected when opened
    m_selected = 0;
    updateActiveDevice();
    core::Logger::instance().info("MainMenu: Entered main menu");
}

void MainMenu::onExit() {
    core::Logger::instance().info("MainMenu: Exited main menu");
}

void MainMenu::updateActiveDevice() {
    m_activeDevice = m_inputHelper->detectActiveDevice();
}

void MainMenu::handleInput() {
    updateActiveDevice();
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selected == 0) m_selected = m_options.size() - 1;
        else --m_selected;
        core::Logger::instance().info("MainMenu: Selected option " + std::to_string(m_selected));
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_options.size();
        core::Logger::instance().info("MainMenu: Selected option " + std::to_string(m_selected));
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_options[m_selected];
        core::Logger::instance().info("MainMenu selected: " + choice);
        
        if (choice == "Jugar (Solo)") {
            if (m_uiManager) {
                m_uiManager->triggerStartGame();
            }
            if (m_manager) {
                // Push PlayScene
                m_manager->push(std::make_unique<scene::PlayScene>(m_manager));
            }
        } else if (choice == "Crear Sala") {
            // TODO: Placeholder for multiplayer room creation
            core::Logger::instance().info("Crear Sala selected - Funcionalidad pendiente de implementación");
            if (m_uiManager) {
                // Show a notification that this feature is pending
                m_uiManager->showNotification("Crear Sala: Funcionalidad pendiente para implementación futura", 
                                            ui::UIManager::NotificationPriority::Normal, 4.0f, sf::Color::Yellow);
            }
        } else if (choice == "Opciones") {
            if (m_uiManager) {
                // Open OptionsMenu with enhanced animation and pass ConfigManager if available
                core::ConfigManager* cfg = m_uiManager->getConfigManager();
                m_uiManager->pushMenu(new OptionsMenu(m_manager, cfg), AnimationType::Slide);
            } else {
                core::Logger::instance().info("Options menu not implemented yet.");
            }
        } else if (choice == "Salir") {
            if (m_uiManager) {
                m_uiManager->triggerExit();
            }
            if (m_manager) {
                // Pop current scene so the game can close
                m_manager->pop();
            }
        }
    }

    if (im.isActionJustPressed(input::Action::Cancel)) {
        // On main menu, cancel means exit
        if (m_uiManager) {
            m_uiManager->triggerExit();
        }
        if (m_manager) {
            m_manager->pop();
        }
    }
}

void MainMenu::handleMouseHover(sf::RenderWindow& window) {
    if (m_activeDevice != InputHelper::DeviceType::Mouse) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);
    
    for (std::size_t i = 0; i < m_options.size(); ++i) {
        sf::Vector2f optionPos(120.f, m_startY + static_cast<float>(i) * m_spacing);
        sf::Vector2f optionSize(300.f, 35.f);
        
        sf::FloatRect bounds(optionPos, optionSize);
        if (bounds.contains(mousePosF)) {
            if (m_selected != i) {
                m_selected = i;
                core::Logger::instance().info("MainMenu: Mouse hover selected option " + std::to_string(m_selected));
            }
            break;
        }
    }
}

void MainMenu::update(float dt) {
    m_backgroundPulse += dt * 0.5f;
    if (m_backgroundPulse > 2.0f * 3.14159f) m_backgroundPulse -= 2.0f * 3.14159f;
    
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

void MainMenu::render(sf::RenderWindow& window) {
    handleMouseHover(window);
    
    renderBackground(window);
    renderTitle(window);
    renderOptions(window);
    renderHints(window);
}

void MainMenu::renderBackground(sf::RenderWindow& window) {
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Animated background with subtle pulse
    sf::RectangleShape bg;
    bg.setSize(windowSize);
    float pulseIntensity = 15.0f + 5.0f * std::sin(m_backgroundPulse);
    std::uint8_t bgAlpha = static_cast<std::uint8_t>(pulseIntensity);
    bg.setFillColor(sf::Color(10, 15, 25, bgAlpha));
    window.draw(bg);
}

void MainMenu::renderTitle(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    // Main title with glow effect
    sf::Text title(font, "ABYSSAL STATION", 48);
    title.setFillColor(sf::Color::White);
    title.setPosition(sf::Vector2f(100.f, 60.f));
    
    // Glow effect (multiple renders with different colors and positions)
    sf::Text titleGlow = title;
    titleGlow.setFillColor(sf::Color(0, 150, 255, 100));
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            if (dx == 0 && dy == 0) continue;
            titleGlow.setPosition(title.getPosition() + sf::Vector2f(dx, dy));
            window.draw(titleGlow);
        }
    }
    
    window.draw(title);
}

void MainMenu::renderOptions(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

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
            highlight.setFillColor(sf::Color(255, 255, 255, highlightAlpha));
            window.draw(highlight);
        }

        // Option text with glow
        sf::Text text(font, m_options[i], 24);
        text.setScale(sf::Vector2f(scale, scale));
        text.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
        text.setPosition(position);

        // Glow effect for selected option
        if (selected && glow > 0.1f) {
            sf::Text textGlow = text;
            std::uint8_t glowAlpha = static_cast<std::uint8_t>(150 * glow);
            textGlow.setFillColor(sf::Color(255, 255, 0, glowAlpha));
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    textGlow.setPosition(position + sf::Vector2f(dx, dy));
                    window.draw(textGlow);
                }
            }
        }

        window.draw(text);

        // Selection indicator with device-specific styling
        if (selected) {
            std::string indicator = ">";
            if (m_activeDevice == InputHelper::DeviceType::Gamepad) {
                indicator = "◆";
            } else if (m_activeDevice == InputHelper::DeviceType::Mouse) {
                indicator = "▶";
            }
            
            sf::Text indicatorText(font, indicator, 24);
            indicatorText.setScale(sf::Vector2f(scale, scale));
            indicatorText.setFillColor(sf::Color::Yellow);
            indicatorText.setPosition(position + sf::Vector2f(-40.f, 0.f));
            window.draw(indicatorText);
        }
    }
}

void MainMenu::renderHints(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    // Contextual hints based on active device
    std::string hint = getContextualHint();
    sf::Text hintText(font, hint, 16);
    hintText.setFillColor(sf::Color(150, 150, 150));
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    hintText.setPosition(sf::Vector2f(120.f, windowSize.y - 80.f));
    window.draw(hintText);
    
    // Device indicator
    std::string deviceName;
    sf::Color deviceColor;
    switch (m_activeDevice) {
        case InputHelper::DeviceType::Keyboard:
            deviceName = "Keyboard";
            deviceColor = sf::Color::Green;
            break;
        case InputHelper::DeviceType::Mouse:
            deviceName = "Mouse";
            deviceColor = sf::Color::Blue;
            break;
        case InputHelper::DeviceType::Gamepad:
            deviceName = "Gamepad";
            deviceColor = sf::Color::Magenta;
            break;
        default:
            deviceName = "Auto";
            deviceColor = sf::Color(128, 128, 128); // Gray
            break;
    }
    
    sf::Text deviceText(font, "Input: " + deviceName, 14);
    deviceText.setFillColor(deviceColor);
    deviceText.setPosition(sf::Vector2f(windowSize.x - 150.f, windowSize.y - 30.f));
    window.draw(deviceText);
}

std::string MainMenu::getContextualHint() const {
    std::string confirmAction = m_inputHelper->getActionDisplayName(input::Action::Confirm, m_activeDevice);
    std::string cancelAction = m_inputHelper->getActionDisplayName(input::Action::Cancel, m_activeDevice);
    std::string moveActions = m_inputHelper->getActionDisplayName(input::Action::MoveUp, m_activeDevice) + 
                             "/" + m_inputHelper->getActionDisplayName(input::Action::MoveDown, m_activeDevice);
    
    return "Navigate: " + moveActions + "  Select: " + confirmAction + "  Exit: " + cancelAction;
}

} // namespace ui
