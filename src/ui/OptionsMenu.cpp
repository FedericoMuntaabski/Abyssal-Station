#include "OptionsMenu.h"
#include "InputHelper.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../core/AssetManager.h"
#include "../core/ConfigManager.h"
#include "../scene/SceneManager.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <sstream>

namespace ui {

using input::InputManager;

OptionsMenu::OptionsMenu(scene::SceneManager* manager, core::ConfigManager* configManager)
    : Menu("OptionsMenu"), m_manager(manager), m_configManager(configManager) {
    
    m_inputHelper = std::make_unique<InputHelper>();
    initializeOptions();
    initializeResolutions();
    
    m_scales.assign(m_mainOptions.size(), 1.0f);
    
    // Load settings from config
    loadSettings();
    
    // Load audio assets
    loadAudio();
}

void OptionsMenu::initializeOptions() {
    m_mainOptions = {
        "Volumen Música",
        "Volumen UI", 
        "Resolución",
        "Pantalla Completa",
        "Guardar",
        "Regresar"
    };
}

void OptionsMenu::initializeResolutions() {
    m_availableResolutions = {
        {1024, 576},
        {1280, 720},
        {1366, 768},
        {1920, 1080}
    };
    
    // Ensure m_selectedResolution is valid
    m_selectedResolution = 0;
    
    // Find current resolution in the list
    for (size_t i = 0; i < m_availableResolutions.size(); ++i) {
        if (m_availableResolutions[i].first == m_resolutionWidth && 
            m_availableResolutions[i].second == m_resolutionHeight) {
            m_selectedResolution = i;
            break;
        }
    }
    
    // Double-check that the selected resolution is within bounds
    if (m_selectedResolution >= m_availableResolutions.size()) {
        m_selectedResolution = 0;
        core::Logger::instance().warning("OptionsMenu: Invalid resolution selection, reset to default");
    }
}

void OptionsMenu::onEnter() {
    m_selected = 0;
    updateActiveDevice();
    core::Logger::instance().info("OptionsMenu: Entered options menu");
}

void OptionsMenu::onExit() {
    core::Logger::instance().info("OptionsMenu: Exited options menu");
}

void OptionsMenu::updateActiveDevice() {
    m_activeDevice = m_inputHelper->detectActiveDevice();
}

void OptionsMenu::handleInput() {
    updateActiveDevice();
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_mainOptions.empty()) return;
        if (m_selected == 0) m_selected = m_mainOptions.size() - 1;
        else --m_selected;
        playHoverSound();
        core::Logger::instance().info("OptionsMenu: Selected option " + std::to_string(m_selected));
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        if (m_mainOptions.empty()) return;
        m_selected = (m_selected + 1) % m_mainOptions.size();
        playHoverSound();
        core::Logger::instance().info("OptionsMenu: Selected option " + std::to_string(m_selected));
    }

    // Handle left/right for value adjustments
    if (im.isActionJustPressed(input::Action::MoveLeft) || im.isActionJustPressed(input::Action::MoveRight)) {
        int delta = im.isActionJustPressed(input::Action::MoveRight) ? 1 : -1;
        
        switch (m_selected) {
            case 0: // Volumen Música
                m_musicVolume = std::min(100, std::max(0, m_musicVolume + delta * 10));
                core::Logger::instance().info("OptionsMenu: Music volume set to " + std::to_string(m_musicVolume));
                break;
            case 1: // Volumen UI
                m_uiVolume = std::min(100, std::max(0, m_uiVolume + delta * 10));
                core::Logger::instance().info("OptionsMenu: UI volume set to " + std::to_string(m_uiVolume));
                break;
            case 2: // Resolución
            {
                if (delta > 0) {
                    m_selectedResolution = (m_selectedResolution + 1) % m_availableResolutions.size();
                } else {
                    if (m_selectedResolution == 0) m_selectedResolution = m_availableResolutions.size() - 1;
                    else m_selectedResolution--;
                }
                auto res = m_availableResolutions[m_selectedResolution];
                setResolution(res.first, res.second);
                core::Logger::instance().info("OptionsMenu: Resolution set to " + std::to_string(res.first) + "x" + std::to_string(res.second));
                break;
            }
            case 3: // Pantalla Completa
                m_fullscreen = !m_fullscreen;
                core::Logger::instance().info("OptionsMenu: Fullscreen " + std::string(m_fullscreen ? "enabled" : "disabled"));
                break;
        }
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        if (m_selected >= m_mainOptions.size()) {
            core::Logger::instance().error("OptionsMenu: Invalid selection index: " + std::to_string(m_selected));
            return;
        }
        
        const auto& choice = m_mainOptions[m_selected];
        playConfirmSound();
        core::Logger::instance().info("OptionsMenu selected: " + choice);
        
        if (choice == "Guardar") {
            saveSettings();
        } else if (choice == "Regresar") {
            if (m_manager) {
                core::Logger::instance().info("OptionsMenu: Returning to previous menu");
                m_manager->pop();
            } else {
                core::Logger::instance().error("OptionsMenu: SceneManager is null, cannot return");
            }
        }
    }

    if (im.isActionJustPressed(input::Action::Cancel)) {
        if (m_manager) {
            core::Logger::instance().info("OptionsMenu: Cancelled, returning to previous menu");
            m_manager->pop();
        } else {
            core::Logger::instance().error("OptionsMenu: SceneManager is null, cannot cancel");
        }
    }
}

void OptionsMenu::handleMouseHover(sf::RenderWindow& window) {
    if (m_activeDevice != InputHelper::DeviceType::Mouse) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Calculate center menu position
    float menuX = windowSize.x / 2.0f - 150.0f;
    
    for (std::size_t i = 0; i < m_mainOptions.size(); ++i) {
        sf::Vector2f optionPos(menuX, m_startY + static_cast<float>(i) * m_spacing);
        sf::Vector2f optionSize(300.f, 35.f);
        
        sf::FloatRect bounds(optionPos, optionSize);
        if (bounds.contains(mousePosF)) {
            if (m_selected != i) {
                m_selected = i;
                playHoverSound();
                core::Logger::instance().info("OptionsMenu: Mouse hover selected option " + std::to_string(m_selected));
            }
            break;
        }
    }
}

void OptionsMenu::update(float dt) {
    // Update scale animations
    float speed = 10.0f;
    float targetScaleSelected = 1.2f;
    
    for (std::size_t i = 0; i < m_scales.size(); ++i) {
        float targetScale = (i == m_selected) ? targetScaleSelected : 1.0f;
        m_scales[i] += (targetScale - m_scales[i]) * std::min(1.0f, speed * dt);
    }
}

void OptionsMenu::render(sf::RenderWindow& window) {
    handleMouseHover(window);
    
    renderBackground(window);
    
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Secundary_font.ttf");
    }

    if (!fontLoaded) return;

    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Render title
    sf::Text title(font, "OPCIONES", 48);
    title.setFillColor(sf::Color::White);
    title.setPosition(sf::Vector2f((windowSize.x - title.getLocalBounds().size.x) / 2.0f, 50.f));
    window.draw(title);
    
    // Position menu in center
    float menuX = windowSize.x / 2.0f - 150.0f;

    for (std::size_t i = 0; i < m_mainOptions.size(); ++i) {
        sf::Vector2f position(menuX, m_startY + static_cast<float>(i) * m_spacing);
        bool selected = (i == m_selected);
        float scale = (i < m_scales.size()) ? m_scales[i] : 1.0f;

        // Option text with value
        std::string displayText = getOptionDisplayText(i);
        sf::Text text(font, displayText, 24);
        text.setScale(sf::Vector2f(scale, scale));
        text.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
        text.setPosition(position);
        
        window.draw(text);

        // Selection indicator
        if (selected) {
            std::string indicator = ">";
            sf::Text indicatorText(font, indicator, 24);
            indicatorText.setScale(sf::Vector2f(scale, scale));
            indicatorText.setFillColor(sf::Color::Yellow);
            indicatorText.setPosition(position + sf::Vector2f(-30.f, 0.f));
            window.draw(indicatorText);
        }
    }
    
    // Render hint
    std::string hint = getContextualHint();
    sf::Text hintText(font, hint, 16);
    hintText.setFillColor(sf::Color(150, 150, 150));
    hintText.setPosition(sf::Vector2f(20.f, windowSize.y - 50.f));
    window.draw(hintText);
}

void OptionsMenu::renderBackground(sf::RenderWindow& window) {
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Try to get Main Menu background texture
    auto backgroundTexture = AssetManager::instance().getTexture("Main Menu");
    
    if (backgroundTexture) {
        // Use Main Menu texture as background
        sf::Sprite backgroundSprite(*backgroundTexture);
        
        // Scale to fit window
        sf::Vector2u textureSize = backgroundTexture->getSize();
        float scaleX = windowSize.x / static_cast<float>(textureSize.x);
        float scaleY = windowSize.y / static_cast<float>(textureSize.y);
        backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));
        
        window.draw(backgroundSprite);
        
        // Dark overlay for better text readability
        sf::RectangleShape overlay;
        overlay.setSize(windowSize);
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
    } else {
        // Fallback: solid background
        sf::RectangleShape bg;
        bg.setSize(windowSize);
        bg.setFillColor(sf::Color(20, 25, 35));
        window.draw(bg);
    }
}

std::string OptionsMenu::getOptionDisplayText(size_t index) const {
    if (index >= m_mainOptions.size()) {
        return "Invalid Option";
    }
    
    switch (index) {
        case 0: // Volumen Música
            return "Volumen Música: " + std::to_string(m_musicVolume) + "%";
        case 1: // Volumen UI
            return "Volumen UI: " + std::to_string(m_uiVolume) + "%";
        case 2: // Resolución
        {
            if (m_selectedResolution >= m_availableResolutions.size()) {
                return "Resolución: Error";
            }
            auto res = m_availableResolutions[m_selectedResolution];
            return "Resolución: " + std::to_string(res.first) + "x" + std::to_string(res.second);
        }
        case 3: // Pantalla Completa
            return "Pantalla Completa: " + std::string(m_fullscreen ? "Sí" : "No");
        case 4: // Guardar
            return "Guardar";
        case 5: // Regresar
            return "Regresar";
        default:
            return m_mainOptions[index];
    }
}

std::string OptionsMenu::getContextualHint() const {
    std::string confirmAction = m_inputHelper->getActionDisplayName(input::Action::Confirm, m_activeDevice);
    std::string cancelAction = m_inputHelper->getActionDisplayName(input::Action::Cancel, m_activeDevice);
    std::string moveActions = m_inputHelper->getActionDisplayName(input::Action::MoveUp, m_activeDevice) + 
                             "/" + m_inputHelper->getActionDisplayName(input::Action::MoveDown, m_activeDevice);
    std::string adjustActions = m_inputHelper->getActionDisplayName(input::Action::MoveLeft, m_activeDevice) + 
                               "/" + m_inputHelper->getActionDisplayName(input::Action::MoveRight, m_activeDevice);
    
    return "Navigate: " + moveActions + "  Adjust: " + adjustActions + "  Select: " + confirmAction + "  Back: " + cancelAction;
}

void OptionsMenu::loadSettings() {
    if (m_configManager) {
        m_musicVolume = m_configManager->musicVolume();
        m_uiVolume = m_configManager->uiVolume();
        m_fullscreen = m_configManager->fullscreen();
        // Resolution will be set by initializeResolutions
    }
}

void OptionsMenu::saveSettings() {
    if (!m_configManager) {
        core::Logger::instance().error("OptionsMenu: ConfigManager is null, cannot save settings");
        return;
    }
    
    if (m_selectedResolution >= m_availableResolutions.size()) {
        core::Logger::instance().error("OptionsMenu: Invalid resolution index, cannot save settings");
        return;
    }
    
    try {
        m_configManager->setMusicVolume(m_musicVolume);
        m_configManager->setUiVolume(m_uiVolume);
        m_configManager->setFullscreen(m_fullscreen);
        
        auto res = m_availableResolutions[m_selectedResolution];
        m_configManager->setResolution(res.first, res.second);
        
        m_configManager->saveConfig();
        core::Logger::instance().info("OptionsMenu: Settings saved successfully");
    } catch (const std::exception& e) {
        core::Logger::instance().error("OptionsMenu: Error saving settings: " + std::string(e.what()));
    }
}

void OptionsMenu::loadAudio() {
    // Load sound effects
    m_hoverBuffer = AssetManager::instance().getSound("hover_select");
    if (m_hoverBuffer) {
        m_hoverSound = std::make_unique<sf::Sound>(*m_hoverBuffer);
        core::Logger::instance().info("OptionsMenu: Hover sound loaded successfully");
    } else {
        core::Logger::instance().warning("OptionsMenu: Failed to load hover sound");
    }
    
    m_confirmBuffer = AssetManager::instance().getSound("confirm");
    if (m_confirmBuffer) {
        m_confirmSound = std::make_unique<sf::Sound>(*m_confirmBuffer);
        core::Logger::instance().info("OptionsMenu: Confirm sound loaded successfully");
    } else {
        core::Logger::instance().warning("OptionsMenu: Failed to load confirm sound");
    }
    
    m_audioLoaded = true;
}

void OptionsMenu::playHoverSound() {
    if (m_audioLoaded && m_hoverSound && m_hoverSound->getStatus() != sf::SoundSource::Status::Playing) {
        m_hoverSound->play();
    }
}

void OptionsMenu::playConfirmSound() {
    if (m_audioLoaded && m_confirmSound) {
        m_confirmSound->play();
    }
}

// Simple interface methods for external use/tests
int OptionsMenu::volume() const noexcept {
    return m_musicVolume; // Return music volume as main volume
}

void OptionsMenu::setVolume(int vol) {
    m_musicVolume = std::min(100, std::max(0, vol));
}

const std::string& OptionsMenu::language() const noexcept {
    return m_language;
}

void OptionsMenu::setLanguage(const std::string& lang) {
    m_language = lang;
}

std::pair<int, int> OptionsMenu::resolution() const noexcept {
    return {m_resolutionWidth, m_resolutionHeight};
}

void OptionsMenu::setResolution(int width, int height) {
    m_resolutionWidth = width;
    m_resolutionHeight = height;
}

} // namespace ui
