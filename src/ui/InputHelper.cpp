#include "InputHelper.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include <SFML/Graphics/Text.hpp>
#include <filesystem>

namespace ui {

InputHelper::InputHelper() {
    initializeKeyNames();
    initializeMouseNames();
    initializeGamepadNames();
    loadDefaultIcons();
}

void InputHelper::loadIconTextures(const std::string& iconPath) {
    if (!std::filesystem::exists(iconPath)) {
        core::Logger::instance().warning("InputHelper: Icon path not found - " + iconPath);
        return;
    }
    
    // Load icon textures for common actions
    // This would load from sprite sheets or individual files
    core::Logger::instance().info("InputHelper: Loading icon textures from " + iconPath);
    
    // TODO: Implement texture loading when assets are available
}

std::string InputHelper::getActionDisplayName(input::Action action, DeviceType device) const {
    if (device == DeviceType::Auto) {
        device = detectActiveDevice();
    }
    
    auto& inputManager = input::InputManager::getInstance();
    
    switch (device) {
        case DeviceType::Keyboard: {
            auto keys = inputManager.getKeyBindings(action);
            if (!keys.empty()) {
                return formatKeyCombo(keys);
            }
            break;
        }
        case DeviceType::Mouse: {
            auto buttons = inputManager.getMouseBindings(action);
            if (!buttons.empty()) {
                auto it = m_mouseNames.find(buttons[0]);
                return (it != m_mouseNames.end()) ? it->second : "Mouse " + std::to_string(static_cast<int>(buttons[0]));
            }
            break;
        }
        case DeviceType::Gamepad: {
            if (m_gamepadSupported) {
                // TODO: Implement gamepad binding detection when gamepad support is added
                return "Gamepad Button";
            }
            break;
        }
        default:
            break;
    }
    
    // Fallback to InputManager's binding name
    return inputManager.getBindingName(action);
}

std::string InputHelper::getKeyDisplayName(sf::Keyboard::Key key, bool includeModifiers) const {
    auto it = m_keyNames.find(key);
    if (it != m_keyNames.end()) {
        return it->second;
    }
    
    // Handle special cases
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z) {
        return std::string(1, 'A' + static_cast<char>(static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::A)));
    }
    if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9) {
        return std::string(1, '0' + static_cast<char>(static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num0)));
    }
    if (key >= sf::Keyboard::Key::F1 && key <= sf::Keyboard::Key::F15) {
        return "F" + std::to_string(static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::F1) + 1);
    }
    
    return "Key " + std::to_string(static_cast<int>(key));
}

std::string InputHelper::getMouseDisplayName(sf::Mouse::Button button) const {
    auto it = m_mouseNames.find(button);
    return (it != m_mouseNames.end()) ? it->second : "Mouse " + std::to_string(static_cast<int>(button));
}

std::string InputHelper::getGamepadDisplayName(int buttonId) const {
    auto it = m_gamepadNames.find(buttonId);
    return (it != m_gamepadNames.end()) ? it->second : "Button " + std::to_string(buttonId);
}

void InputHelper::renderActionIcon(sf::RenderWindow& window, input::Action action, 
                                  const sf::Vector2f& position, DeviceType device) const {
    auto it = m_actionIcons.find(action);
    if (it != m_actionIcons.end()) {
        sf::Sprite sprite(it->second.texture);
        sprite.setPosition(position);
        sprite.setScale(sf::Vector2f(
            it->second.size.x / static_cast<float>(sprite.getTextureRect().size.x), 
            it->second.size.y / static_cast<float>(sprite.getTextureRect().size.y)
        ));
        sprite.setColor(it->second.tint);
        window.draw(sprite);
    } else {
        // Fallback: draw a simple rectangle with text
        sf::RectangleShape rect(sf::Vector2f(32.f, 32.f));
        rect.setPosition(position);
        rect.setFillColor(sf::Color(50, 50, 50));
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(1.f);
        window.draw(rect);
    }
}

void InputHelper::renderKeyCombo(sf::RenderWindow& window, const std::vector<sf::Keyboard::Key>& keys,
                                const sf::Vector2f& position, const sf::Font& font) const {
    if (keys.empty()) return;
    
    std::string comboText = formatKeyCombo(keys);
    sf::Text text(font, comboText, 16);
    text.setPosition(position);
    text.setFillColor(sf::Color::White);
    window.draw(text);
}

sf::Text InputHelper::createActionHint(const std::string& baseText, input::Action action, 
                                      const sf::Font& font, DeviceType device) const {
    std::string actionName = getActionDisplayName(action, device);
    std::string hintText = baseText;
    
    // Replace [ACTION] placeholder with actual binding
    size_t pos = hintText.find("[ACTION]");
    if (pos != std::string::npos) {
        hintText.replace(pos, 8, "[" + actionName + "]");
    } else {
        // Append action if no placeholder found
        hintText += " [" + actionName + "]";
    }
    
    sf::Text text(font, hintText, 16);
    text.setFillColor(sf::Color::White);
    return text;
}

InputHelper::DeviceType InputHelper::detectActiveDevice() const {
    // Simple heuristic: check what was used last
    // In a full implementation, this would check InputManager for recent input events
    auto& inputManager = input::InputManager::getInstance();
    
    // Check if any mouse buttons were pressed recently
    auto mouseEvent = inputManager.getLastMouseButtonEvent();
    if (mouseEvent.first) {
        return DeviceType::Mouse;
    }
    
    // Check if any keys were pressed recently  
    auto keyEvent = inputManager.getLastKeyEvent();
    if (keyEvent.first) {
        return DeviceType::Keyboard;
    }
    
    // TODO: Add gamepad detection when gamepad support is implemented
    
    return m_lastActiveDevice;
}

void InputHelper::initializeKeyNames() {
    // Navigation keys
    m_keyNames[sf::Keyboard::Key::Up] = "↑";
    m_keyNames[sf::Keyboard::Key::Down] = "↓";
    m_keyNames[sf::Keyboard::Key::Left] = "←";
    m_keyNames[sf::Keyboard::Key::Right] = "→";
    
    // Common keys
    m_keyNames[sf::Keyboard::Key::Space] = "Space";
    m_keyNames[sf::Keyboard::Key::Enter] = "Enter";
    m_keyNames[sf::Keyboard::Key::Escape] = "Esc";
    m_keyNames[sf::Keyboard::Key::Tab] = "Tab";
    m_keyNames[sf::Keyboard::Key::Backspace] = "Backspace";
    m_keyNames[sf::Keyboard::Key::Delete] = "Del";
    
    // Modifiers
    m_keyNames[sf::Keyboard::Key::LControl] = "Ctrl";
    m_keyNames[sf::Keyboard::Key::RControl] = "Ctrl";
    m_keyNames[sf::Keyboard::Key::LShift] = "Shift";
    m_keyNames[sf::Keyboard::Key::RShift] = "Shift";
    m_keyNames[sf::Keyboard::Key::LAlt] = "Alt";
    m_keyNames[sf::Keyboard::Key::RAlt] = "Alt";
    m_keyNames[sf::Keyboard::Key::LSystem] = "Win";
    m_keyNames[sf::Keyboard::Key::RSystem] = "Win";
    
    // Special characters
    m_keyNames[sf::Keyboard::Key::Semicolon] = ";";
    m_keyNames[sf::Keyboard::Key::Equal] = "=";
    m_keyNames[sf::Keyboard::Key::Comma] = ",";
    m_keyNames[sf::Keyboard::Key::Hyphen] = "-";
    m_keyNames[sf::Keyboard::Key::Period] = ".";
    m_keyNames[sf::Keyboard::Key::Slash] = "/";
    // m_keyNames[sf::Keyboard::Key::Tilde] = "~";  // Not available in SFML 3.x
    m_keyNames[sf::Keyboard::Key::LBracket] = "[";
    m_keyNames[sf::Keyboard::Key::Backslash] = "\\";
    m_keyNames[sf::Keyboard::Key::RBracket] = "]";
    // m_keyNames[sf::Keyboard::Key::Quote] = "'";  // Not available in SFML 3.x
}

void InputHelper::initializeMouseNames() {
    m_mouseNames[sf::Mouse::Button::Left] = "Left Click";
    m_mouseNames[sf::Mouse::Button::Right] = "Right Click";
    m_mouseNames[sf::Mouse::Button::Middle] = "Middle Click";
    // m_mouseNames[sf::Mouse::Button::XButton1] = "Mouse 4";  // Not available in SFML 3.x
    // m_mouseNames[sf::Mouse::Button::XButton2] = "Mouse 5";  // Not available in SFML 3.x
}

void InputHelper::initializeGamepadNames() {
    // Xbox-style naming (most common)
    m_gamepadNames[0] = "A Button";
    m_gamepadNames[1] = "B Button";
    m_gamepadNames[2] = "X Button";
    m_gamepadNames[3] = "Y Button";
    m_gamepadNames[4] = "Left Bumper";
    m_gamepadNames[5] = "Right Bumper";
    m_gamepadNames[6] = "Back";
    m_gamepadNames[7] = "Start";
    m_gamepadNames[8] = "Left Stick";
    m_gamepadNames[9] = "Right Stick";
    m_gamepadNames[10] = "D-Pad Up";
    m_gamepadNames[11] = "D-Pad Down";
    m_gamepadNames[12] = "D-Pad Left";
    m_gamepadNames[13] = "D-Pad Right";
}

void InputHelper::loadDefaultIcons() {
    // Create placeholder icons for common actions
    // In a full implementation, these would be loaded from asset files
    
    InputIcon defaultIcon;
    // Create a simple white square texture as fallback
    // This would be replaced with actual icon loading
    
    m_actionIcons[input::Action::MoveUp] = defaultIcon;
    m_actionIcons[input::Action::MoveDown] = defaultIcon;
    m_actionIcons[input::Action::MoveLeft] = defaultIcon;
    m_actionIcons[input::Action::MoveRight] = defaultIcon;
    m_actionIcons[input::Action::Confirm] = defaultIcon;
    m_actionIcons[input::Action::Cancel] = defaultIcon;
    m_actionIcons[input::Action::Interact] = defaultIcon;
    m_actionIcons[input::Action::Pause] = defaultIcon;
}

bool InputHelper::hasModifiers(const std::vector<sf::Keyboard::Key>& keys) const {
    for (auto key : keys) {
        if (key == sf::Keyboard::Key::LControl || key == sf::Keyboard::Key::RControl ||
            key == sf::Keyboard::Key::LShift || key == sf::Keyboard::Key::RShift ||
            key == sf::Keyboard::Key::LAlt || key == sf::Keyboard::Key::RAlt ||
            key == sf::Keyboard::Key::LSystem || key == sf::Keyboard::Key::RSystem) {
            return true;
        }
    }
    return false;
}

std::string InputHelper::formatKeyCombo(const std::vector<sf::Keyboard::Key>& keys) const {
    if (keys.empty()) return "";
    
    if (keys.size() == 1) {
        return getKeyDisplayName(keys[0]);
    }
    
    // Sort modifiers first, then regular keys
    std::vector<sf::Keyboard::Key> modifiers;
    std::vector<sf::Keyboard::Key> regularKeys;
    
    for (auto key : keys) {
        if (key == sf::Keyboard::Key::LControl || key == sf::Keyboard::Key::RControl ||
            key == sf::Keyboard::Key::LShift || key == sf::Keyboard::Key::RShift ||
            key == sf::Keyboard::Key::LAlt || key == sf::Keyboard::Key::RAlt ||
            key == sf::Keyboard::Key::LSystem || key == sf::Keyboard::Key::RSystem) {
            modifiers.push_back(key);
        } else {
            regularKeys.push_back(key);
        }
    }
    
    std::string result;
    
    // Add modifiers first
    for (auto modifier : modifiers) {
        if (!result.empty()) result += "+";
        result += getKeyDisplayName(modifier);
    }
    
    // Add regular keys
    for (auto key : regularKeys) {
        if (!result.empty()) result += "+";
        result += getKeyDisplayName(key);
    }
    
    return result;
}

} // namespace ui
