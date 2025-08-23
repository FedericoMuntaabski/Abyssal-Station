#include "OptionsMenu.h"
#include "InputHelper.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../core/ConfigManager.h"
#include "../scene/SceneManager.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ui {

using input::InputManager;

OptionsMenu::OptionsMenu(scene::SceneManager* manager, core::ConfigManager* configManager)
    : Menu("OptionsMenu"), m_manager(manager), m_configManager(configManager) {
    
    m_inputHelper = std::make_unique<InputHelper>();
    initializeOptions();
    initializeResolutions();
    loadSettings();
}

void OptionsMenu::initializeOptions() {
    m_mainOptions = {"Volume", "Resolution", "Language", "Controls", "Back"};
    m_scales.assign(m_mainOptions.size(), 1.0f);
    
    m_actionsToShow = {
        input::Action::MoveUp, 
        input::Action::MoveDown, 
        input::Action::MoveLeft, 
        input::Action::MoveRight, 
        input::Action::Confirm, 
        input::Action::Cancel,
        input::Action::Interact,
        input::Action::Pause
    };
    m_actionScales.assign(m_actionsToShow.size(), 1.0f);
}

void OptionsMenu::initializeResolutions() {
    m_availableResolutions = {
        {1280, 720},
        {1366, 768},
        {1920, 1080},
        {2560, 1440},
        {3840, 2160}
    };
    
    // Find current resolution in list
    for (size_t i = 0; i < m_availableResolutions.size(); ++i) {
        if (m_availableResolutions[i].first == m_resolutionWidth && 
            m_availableResolutions[i].second == m_resolutionHeight) {
            m_selectedResolution = i;
            break;
        }
    }
}

void OptionsMenu::onEnter() {
    m_mode = MenuMode::Main;
    m_selected = 0;
    m_selectedAction = 0;
    m_waitingForRemap = false;
    updateActiveDevice();
    loadSettings();
}

void OptionsMenu::onExit() {
    saveSettings();
}

int OptionsMenu::volume() const noexcept { 
    return m_volume; 
}

void OptionsMenu::setVolume(int vol) {
    m_volume = std::clamp(vol, 0, 100);
    applyVolume();
}

const std::string& OptionsMenu::language() const noexcept {
    return m_language;
}

void OptionsMenu::setLanguage(const std::string& lang) {
    m_language = lang;
    applyLanguage();
}

std::pair<int, int> OptionsMenu::resolution() const noexcept {
    return {m_resolutionWidth, m_resolutionHeight};
}

void OptionsMenu::setResolution(int width, int height) {
    m_resolutionWidth = width;
    m_resolutionHeight = height;
    applyResolution();
}

void OptionsMenu::handleInput() {
    updateActiveDevice();
    
    switch (m_mode) {
        case MenuMode::Main:
            handleMainMenuInput();
            break;
        case MenuMode::Controls:
            handleControlsInput();
            break;
        case MenuMode::Remapping:
            handleRemappingInput();
            break;
    }
}

void OptionsMenu::handleMainMenuInput() {
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selected == 0) m_selected = m_mainOptions.size() - 1;
        else --m_selected;
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_mainOptions.size();
    }

    // Handle left/right for value adjustments
    if (im.isActionJustPressed(input::Action::MoveLeft) || im.isActionJustPressed(input::Action::MoveRight)) {
        int delta = im.isActionJustPressed(input::Action::MoveRight) ? 1 : -1;
        
        switch (m_selected) {
            case 0: // Volume
                adjustVolume(delta * 5);
                break;
            case 1: // Resolution
                if (delta > 0) cycleResolution();
                else {
                    if (m_selectedResolution > 0) {
                        m_selectedResolution--;
                        auto res = m_availableResolutions[m_selectedResolution];
                        setResolution(res.first, res.second);
                    }
                }
                break;
            case 2: // Language
                cycleLanguage();
                break;
        }
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_mainOptions[m_selected];
        core::Logger::instance().info("OptionsMenu selected: " + choice);
        
        if (choice == "Back") {
            deactivate();
        } else if (choice == "Controls") {
            m_mode = MenuMode::Controls;
            m_selectedAction = 0;
        }
    }

    if (im.isActionJustPressed(input::Action::Cancel)) {
        deactivate();
    }
}

void OptionsMenu::handleControlsInput() {
    auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selectedAction == 0) m_selectedAction = m_actionsToShow.size() - 1;
        else --m_selectedAction;
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selectedAction = (m_selectedAction + 1) % m_actionsToShow.size();
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        // Start remapping the selected action
        m_mode = MenuMode::Remapping;
        m_waitingForRemap = true;
        m_actionToRemap = m_actionsToShow[m_selectedAction];
        m_remapTimer = 0.0f;
        m_currentCombo.clear();
        core::Logger::instance().info("Starting remap for action: " + std::to_string(static_cast<int>(m_actionToRemap)));
    }

    if (im.isActionJustPressed(input::Action::Cancel)) {
        m_mode = MenuMode::Main;
        m_selected = 3; // Back to "Controls" option
    }
}

void OptionsMenu::handleRemappingInput() {
    auto& im = InputManager::getInstance();
    
    // Check for escape to cancel remapping
    if (im.isActionJustPressed(input::Action::Cancel)) {
        m_mode = MenuMode::Controls;
        m_waitingForRemap = false;
        im.clearLastEvents();
        core::Logger::instance().info("Remapping cancelled");
        return;
    }
    
    // Check for new key input
    auto lastKey = im.getLastKeyEvent();
    if (lastKey.first) {
        // Handle key combinations (Ctrl, Shift, Alt + key)
        sf::Keyboard::Key key = lastKey.second;
        
        // Check if it's a modifier key being held
        bool isModifier = (key == sf::Keyboard::Key::LControl || key == sf::Keyboard::Key::RControl ||
                          key == sf::Keyboard::Key::LShift || key == sf::Keyboard::Key::RShift ||
                          key == sf::Keyboard::Key::LAlt || key == sf::Keyboard::Key::RAlt);
        
        if (!isModifier) {
            // Complete the remapping
            std::vector<sf::Keyboard::Key> finalCombo = m_currentCombo;
            finalCombo.push_back(key);
            
            im.rebindKeys(m_actionToRemap, finalCombo);
            
            std::string comboStr = m_inputHelper->formatKeyCombo(finalCombo);
            core::Logger::instance().info("Rebound action to: " + comboStr);
            
            // Save immediately
            im.saveBindings("config/input_bindings.json");
            
            m_mode = MenuMode::Controls;
            m_waitingForRemap = false;
            im.clearLastEvents();
        } else {
            // Add modifier to current combo
            if (std::find(m_currentCombo.begin(), m_currentCombo.end(), key) == m_currentCombo.end()) {
                m_currentCombo.push_back(key);
            }
        }
    }
    
    // Check for mouse input
    auto lastMouse = im.getLastMouseButtonEvent();
    if (lastMouse.first) {
        im.rebindMouse(m_actionToRemap, {lastMouse.second});
        
        std::string mouseStr = m_inputHelper->getMouseDisplayName(lastMouse.second);
        core::Logger::instance().info("Rebound action to: " + mouseStr);
        
        // Save immediately
        im.saveBindings("config/input_bindings.json");
        
        m_mode = MenuMode::Controls;
        m_waitingForRemap = false;
        im.clearLastEvents();
    }
}

void OptionsMenu::update(float dt) {
    m_remapTimer += dt;
    
    // Timeout remapping after 5 seconds
    if (m_mode == MenuMode::Remapping && m_remapTimer > m_remapTimeout) {
        core::Logger::instance().info("Remapping timed out");
        m_mode = MenuMode::Controls;
        m_waitingForRemap = false;
        auto& im = InputManager::getInstance();
        im.clearLastEvents();
    }
    
    // Update pulsing animation for remapping
    if (m_mode == MenuMode::Remapping) {
        m_remapPulse += dt * 3.0f;
        if (m_remapPulse > 2.0f * 3.14159f) m_remapPulse -= 2.0f * 3.14159f;
    }
    
    // Update scale animations for main menu
    if (m_mode == MenuMode::Main) {
        float speed = 8.0f;
        float targetScaleSelected = 1.15f;
        for (std::size_t i = 0; i < m_scales.size(); ++i) {
            float target = (i == m_selected) ? targetScaleSelected : 1.0f;
            m_scales[i] += (target - m_scales[i]) * std::min(1.0f, speed * dt);
        }
    }
    
    // Update scale animations for controls menu
    if (m_mode == MenuMode::Controls) {
        float speed = 8.0f;
        float targetScaleSelected = 1.1f;
        for (std::size_t i = 0; i < m_actionScales.size(); ++i) {
            float target = (i == m_selectedAction) ? targetScaleSelected : 1.0f;
            m_actionScales[i] += (target - m_actionScales[i]) * std::min(1.0f, speed * dt);
        }
    }
}

void OptionsMenu::render(sf::RenderWindow& window) {
    switch (m_mode) {
        case MenuMode::Main:
            renderMainMenu(window);
            break;
        case MenuMode::Controls:
            renderControlsMenu(window);
            break;
        case MenuMode::Remapping:
            renderControlsMenu(window);
            renderRemappingOverlay(window);
            break;
    }
}

void OptionsMenu::renderMainMenu(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    // Title
    sf::Text title(font, "Options", 32);
    title.setFillColor(sf::Color::White);
    title.setPosition(sf::Vector2f(100.f, 50.f));
    window.draw(title);

    // Options with values
    for (std::size_t i = 0; i < m_mainOptions.size(); ++i) {
        sf::Vector2f position(120.f, m_startY + static_cast<float>(i) * m_spacing);
        bool selected = (i == m_selected);
        float scale = (i < m_scales.size()) ? m_scales[i] : 1.0f;

        std::string value;
        switch (i) {
            case 0: // Volume
                value = std::to_string(m_volume) + "%";
                break;
            case 1: // Resolution  
                value = std::to_string(m_resolutionWidth) + "x" + std::to_string(m_resolutionHeight);
                break;
            case 2: // Language
                value = m_language;
                break;
            case 3: // Controls
                value = "";
                break;
            case 4: // Back
                value = "";
                break;
        }

        renderOption(window, m_mainOptions[i], value, position, selected, scale);
    }

    // Contextual hints
    std::string hint = getContextualHint();
    renderHint(window, hint, sf::Vector2f(120.f, m_startY + m_mainOptions.size() * m_spacing + 40.f));
}

void OptionsMenu::renderControlsMenu(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    // Title
    sf::Text title(font, "Control Bindings", 28);
    title.setFillColor(sf::Color::White);
    title.setPosition(sf::Vector2f(100.f, 50.f));
    window.draw(title);

    // Instructions
    sf::Text instructions(font, "Select action and press [ACTION] to remap", 16);
    instructions.setFillColor(sf::Color(128, 128, 128));
    instructions.setPosition(sf::Vector2f(120.f, 80.f));
    window.draw(instructions);

    // Action bindings
    for (std::size_t i = 0; i < m_actionsToShow.size(); ++i) {
        sf::Vector2f position(120.f, m_controlsStartY + static_cast<float>(i) * m_controlsSpacing);
        bool selected = (i == m_selectedAction);
        float scale = (i < m_actionScales.size()) ? m_actionScales[i] : 1.0f;

        renderControlBinding(window, m_actionsToShow[i], position, selected, scale);
    }

    // Back hint
    renderHint(window, "Press [ACTION] to go back", 
              sf::Vector2f(120.f, m_controlsStartY + m_actionsToShow.size() * m_controlsSpacing + 30.f));
}

void OptionsMenu::renderRemappingOverlay(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Semi-transparent overlay
    sf::RectangleShape overlay;
    overlay.setSize(windowSize);
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);

    // Remapping dialog
    sf::Vector2f dialogSize(400.f, 200.f);
    sf::Vector2f dialogPos = (windowSize - dialogSize) * 0.5f;
    
    sf::RectangleShape dialog;
    dialog.setSize(dialogSize);
    dialog.setPosition(dialogPos);
    dialog.setFillColor(sf::Color(40, 40, 40));
    dialog.setOutlineColor(sf::Color::White);
    dialog.setOutlineThickness(2.f);
    window.draw(dialog);

    // Pulsing effect
    float pulseAlpha = 0.5f + 0.5f * std::sin(m_remapPulse);
    
    sf::Text remapText(font, "Press new key/button for:", 18);
    remapText.setFillColor(sf::Color::White);
    remapText.setPosition(dialogPos + sf::Vector2f(20.f, 30.f));
    window.draw(remapText);

    // Action name with pulsing
    std::string actionName = "Action " + std::to_string(static_cast<int>(m_actionToRemap));
    sf::Text actionText(font, actionName, 24);
    sf::Color actionColor = sf::Color::Yellow;
    actionColor.a = static_cast<std::uint8_t>(255 * pulseAlpha);
    actionText.setFillColor(actionColor);
    actionText.setPosition(dialogPos + sf::Vector2f(20.f, 70.f));
    window.draw(actionText);

    // Current combo if any
    if (!m_currentCombo.empty()) {
        std::string comboStr = m_inputHelper->formatKeyCombo(m_currentCombo) + " + ...";
        sf::Text comboText(font, comboStr, 18);
        comboText.setFillColor(sf::Color::Cyan);
        comboText.setPosition(dialogPos + sf::Vector2f(20.f, 110.f));
        window.draw(comboText);
    }

    // Timeout indicator
    float timeLeft = m_remapTimeout - m_remapTimer;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << timeLeft << "s";
    sf::Text timeoutText(font, "Timeout: " + ss.str(), 14);
    timeoutText.setFillColor(sf::Color(128, 128, 128));
    timeoutText.setPosition(dialogPos + sf::Vector2f(20.f, 150.f));
    window.draw(timeoutText);

    // Cancel hint
    sf::Text cancelText(font, "Press [Cancel] to abort", 14);
    cancelText.setFillColor(sf::Color(128, 128, 128));
    cancelText.setPosition(dialogPos + sf::Vector2f(250.f, 150.f));
    window.draw(cancelText);
}

// UI helper methods
void OptionsMenu::renderOption(sf::RenderWindow& window, const std::string& label, const std::string& value,
                              const sf::Vector2f& position, bool selected, float scale) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    sf::Text labelText(font, label, 20);
    labelText.setScale(sf::Vector2f(scale, scale));
    labelText.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
    labelText.setPosition(position);
    window.draw(labelText);

    if (!value.empty()) {
        sf::Text valueText(font, value, 20);
        valueText.setScale(sf::Vector2f(scale, scale));
        valueText.setFillColor(selected ? sf::Color::Cyan : sf::Color(128, 128, 128));
        valueText.setPosition(position + sf::Vector2f(200.f, 0.f));
        window.draw(valueText);
    }

    // Selection indicator
    if (selected) {
        sf::Text indicator(font, ">", 20);
        indicator.setScale(sf::Vector2f(scale, scale));
        indicator.setFillColor(sf::Color::Yellow);
        indicator.setPosition(position + sf::Vector2f(-30.f, 0.f));
        window.draw(indicator);
    }
}

void OptionsMenu::renderControlBinding(sf::RenderWindow& window, input::Action action, 
                                     const sf::Vector2f& position, bool selected, float scale) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    // Action name
    std::string actionName = "Action " + std::to_string(static_cast<int>(action));
    sf::Text labelText(font, actionName, 18);
    labelText.setScale(sf::Vector2f(scale, scale));
    labelText.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
    labelText.setPosition(position);
    window.draw(labelText);

    // Current binding
    std::string bindingName = m_inputHelper->getActionDisplayName(action, m_activeDevice);
    sf::Text bindingText(font, bindingName, 18);
    bindingText.setScale(sf::Vector2f(scale, scale));
    bindingText.setFillColor(selected ? sf::Color::Cyan : sf::Color(128, 128, 128));
    bindingText.setPosition(position + sf::Vector2f(200.f, 0.f));
    window.draw(bindingText);

    // Device icon (simplified - just text indicator)
    std::string deviceStr;
    switch (m_activeDevice) {
        case InputHelper::DeviceType::Keyboard: deviceStr = "[KB]"; break;
        case InputHelper::DeviceType::Mouse: deviceStr = "[MS]"; break;
        case InputHelper::DeviceType::Gamepad: deviceStr = "[GP]"; break;
        default: deviceStr = "[?]"; break;
    }
    
    sf::Text deviceText(font, deviceStr, 14);
    deviceText.setFillColor(sf::Color::Green);
    deviceText.setPosition(position + sf::Vector2f(350.f, 2.f));
    window.draw(deviceText);

    // Selection indicator
    if (selected) {
        sf::Text indicator(font, ">", 18);
        indicator.setScale(sf::Vector2f(scale, scale));
        indicator.setFillColor(sf::Color::Yellow);
        indicator.setPosition(position + sf::Vector2f(-25.f, 0.f));
        window.draw(indicator);
    }
}

void OptionsMenu::renderHint(sf::RenderWindow& window, const std::string& text, const sf::Vector2f& position) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (!fontLoaded) return;

    sf::Text hintText(font, text, 14);
    hintText.setFillColor(sf::Color(150, 150, 150));
    hintText.setPosition(position);
    window.draw(hintText);
}

// Value adjustment helpers
void OptionsMenu::adjustVolume(int delta) {
    setVolume(m_volume + delta);
}

void OptionsMenu::cycleLanguage() {
    std::vector<std::string> languages = {"en", "es", "fr", "de", "it"};
    auto it = std::find(languages.begin(), languages.end(), m_language);
    if (it != languages.end()) {
        ++it;
        if (it == languages.end()) it = languages.begin();
        setLanguage(*it);
    } else {
        setLanguage(languages[0]);
    }
}

void OptionsMenu::cycleResolution() {
    m_selectedResolution = (m_selectedResolution + 1) % m_availableResolutions.size();
    auto res = m_availableResolutions[m_selectedResolution];
    setResolution(res.first, res.second);
}

// Settings persistence
void OptionsMenu::saveSettings() {
    if (m_configManager) {
        // Save through ConfigManager if available
        core::Logger::instance().info("OptionsMenu: Saving settings through ConfigManager");
    } else {
        core::Logger::instance().info("OptionsMenu: ConfigManager not available, settings saved locally");
    }
}

void OptionsMenu::loadSettings() {
    if (m_configManager) {
        m_volume = m_configManager->volume();
        m_language = m_configManager->language();
        m_resolutionWidth = m_configManager->width();
        m_resolutionHeight = m_configManager->height();
        
        // Update resolution selection
        for (size_t i = 0; i < m_availableResolutions.size(); ++i) {
            if (m_availableResolutions[i].first == m_resolutionWidth && 
                m_availableResolutions[i].second == m_resolutionHeight) {
                m_selectedResolution = i;
                break;
            }
        }
        
        core::Logger::instance().info("OptionsMenu: Settings loaded from ConfigManager");
    }
}

// Apply settings
void OptionsMenu::applyVolume() {
    core::Logger::instance().info("Volume set to " + std::to_string(m_volume));
    // TODO: integrate with AudioManager when available
}

void OptionsMenu::applyLanguage() {
    core::Logger::instance().info("Language set to " + m_language);
    // TODO: integrate with localization system when available
}

void OptionsMenu::applyResolution() {
    core::Logger::instance().info("Resolution set to " + 
                                 std::to_string(m_resolutionWidth) + "x" + std::to_string(m_resolutionHeight));
    // TODO: integrate with window resizing when available
}

// Device detection and hints
void OptionsMenu::updateActiveDevice() {
    m_activeDevice = m_inputHelper->detectActiveDevice();
}

std::string OptionsMenu::getContextualHint() const {
    switch (m_mode) {
        case MenuMode::Main:
            switch (m_selected) {
                case 0: return "Use ← → to adjust volume, [ACTION] to confirm";
                case 1: return "Use ← → to change resolution, [ACTION] to confirm";
                case 2: return "Use ← → to change language, [ACTION] to confirm";
                case 3: return "Press [ACTION] to configure controls";
                case 4: return "Press [ACTION] to go back";
                default: return "Use ↑ ↓ to navigate, [ACTION] to select";
            }
        case MenuMode::Controls:
            return "Select action and press [ACTION] to remap, [Cancel] to go back";
        case MenuMode::Remapping:
            return "Press the new key/button combination, [Cancel] to abort";
        default:
            return "";
    }
}

} // namespace ui
