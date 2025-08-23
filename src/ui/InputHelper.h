#ifndef ABYSSAL_STATION_SRC_UI_INPUTHELPER_H
#define ABYSSAL_STATION_SRC_UI_INPUTHELPER_H

#include "../input/Action.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace ui {

// Helper class for rendering input icons and device-specific hints
class InputHelper {
public:
    enum class DeviceType {
        Keyboard,
        Mouse, 
        Gamepad,
        Auto // Automatically detect from InputManager
    };

    struct InputIcon {
        sf::Texture texture;
        sf::Vector2f size{32.f, 32.f};
        sf::Color tint{sf::Color::White};
    };

    InputHelper();
    ~InputHelper() = default;

    // Initialize with custom icon textures (optional)
    void loadIconTextures(const std::string& iconPath);
    
    // Get display name for an action based on current device
    std::string getActionDisplayName(input::Action action, DeviceType device = DeviceType::Auto) const;
    
    // Get keyboard key display name (e.g., "W", "Ctrl+Shift+C", "Left Arrow")
    std::string getKeyDisplayName(sf::Keyboard::Key key, bool includeModifiers = true) const;
    
    // Get mouse button display name (e.g., "Left Click", "Right Click", "Wheel")
    std::string getMouseDisplayName(sf::Mouse::Button button) const;
    
    // Get gamepad button display name (e.g., "A Button", "Left Stick")
    std::string getGamepadDisplayName(int buttonId) const;
    
    // Render action icon at specified position
    void renderActionIcon(sf::RenderWindow& window, input::Action action, 
                         const sf::Vector2f& position, DeviceType device = DeviceType::Auto) const;
    
    // Render key combination text/icon
    void renderKeyCombo(sf::RenderWindow& window, const std::vector<sf::Keyboard::Key>& keys,
                       const sf::Vector2f& position, const sf::Font& font) const;
    
    // Create text with action hint (e.g., "Press [W] to move up")
    sf::Text createActionHint(const std::string& baseText, input::Action action, 
                             const sf::Font& font, DeviceType device = DeviceType::Auto) const;
    
    // Detect if a specific device type is currently active
    DeviceType detectActiveDevice() const;
    
    // Enable/disable gamepad icon support
    void setGamepadSupported(bool supported) { m_gamepadSupported = supported; }
    bool isGamepadSupported() const { return m_gamepadSupported; }
    
    // Set icon style and theming
    void setIconTheme(const std::string& theme) { m_iconTheme = theme; }
    const std::string& getIconTheme() const { return m_iconTheme; }
    
    // Public helper for combo formatting
    std::string formatKeyCombo(const std::vector<sf::Keyboard::Key>& keys) const;

private:
    std::unordered_map<sf::Keyboard::Key, std::string> m_keyNames;
    std::unordered_map<sf::Mouse::Button, std::string> m_mouseNames;
    std::unordered_map<int, std::string> m_gamepadNames;
    std::unordered_map<input::Action, InputIcon> m_actionIcons;
    
    bool m_gamepadSupported{true};
    std::string m_iconTheme{"default"};
    DeviceType m_lastActiveDevice{DeviceType::Keyboard};
    
    void initializeKeyNames();
    void initializeMouseNames();
    void initializeGamepadNames();
    void loadDefaultIcons();
    
    // Check if key combination includes modifiers
    bool hasModifiers(const std::vector<sf::Keyboard::Key>& keys) const;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_INPUTHELPER_H
