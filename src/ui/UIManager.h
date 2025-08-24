#ifndef ABYSSAL_STATION_SRC_UI_UIMANAGER_H
#define ABYSSAL_STATION_SRC_UI_UIMANAGER_H

#include "Menu.h"
#include "CustomCursor.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

// Forward declarations
namespace core { class ConfigManager; }

namespace ui {

// Event callbacks for decoupled communication with other systems
struct UIEvents {
    std::function<void()> onStartGame;
    std::function<void()> onExit;
    std::function<void(int)> onVolumeChanged;
    std::function<void(const std::string&)> onLanguageChanged;
    std::function<void(int, int)> onResolutionChanged;
    std::function<void()> onBindingChanged;
    std::function<void()> onSaveGame;
    std::function<void()> onLoadGame;
};

// Animation types for menu transitions
enum class AnimationType {
    None,
    Fade,
    Slide,
    Scale
};

// Improved manager that owns menus and provides stack-like operations with animations and events.
class UIManager {
public:
    UIManager();
    ~UIManager();

    // Enhanced menu management
    void pushMenu(Menu* menu, AnimationType animation = AnimationType::Fade);
    void popMenu(AnimationType animation = AnimationType::Fade);
    void replaceMenu(Menu* menu, AnimationType animation = AnimationType::Fade);
    void insertMenuBelow(Menu* menu, size_t index = 0);

    // Return pointer to the current top menu or nullptr.
    Menu* currentMenu() const noexcept;

    // Query helpers
    bool isMenuActive(const std::string& name) const noexcept;
    bool isAnyMenuActive() const noexcept;
    size_t menuStackSize() const noexcept;

    // Delegate update/render to active menus (those with isActive()==true).
    void update(float dt);
    void render(sf::RenderWindow& window);

    // Convenience: close all menus
    void clear();

    // Enhanced notifications with priorities and stacking
    enum class NotificationPriority { Low, Normal, High, Critical };
    
    void showNotification(const std::string& text, NotificationPriority priority = NotificationPriority::Normal, 
                         float duration = 3.0f, sf::Color color = sf::Color::White);
    void showToast(const std::string& text, float duration = 2.0f, sf::Color color = sf::Color::White);
    
    // Lightweight notifications from game systems (item pickup, puzzle completion)
    void notifyItemCollected(std::uint32_t id);
    void notifyItemCollected(std::uint32_t id, sf::Color iconColor);
    void notifyPuzzleCompleted(std::uint32_t id);
    void notifyPuzzleCompleted(std::uint32_t id, sf::Color iconColor);
    void notifyAchievement(const std::string& title, const std::string& description);

    // Animation and visual settings
    void setAnimationSpeed(float speed) { m_animationSpeed = speed; }
    void setDefaultAnimation(AnimationType type) { m_defaultAnimation = type; }
    void setNotificationDuration(float seconds) { m_notificationDuration = seconds; }
    
    // Font and styling
    void setFont(const sf::Font& font);
    void setTheme(const std::string& themeName);
    
    // Custom cursor management
    void enableCustomCursor(bool enabled = true);
    void disableCustomCursor() { enableCustomCursor(false); }
    bool isCustomCursorEnabled() const;
    void setCustomCursorScale(float scale);
    void setCustomCursorOffset(const sf::Vector2f& offset);
    
    // UI state and accessibility
    void setPaused(bool p) { m_paused = p; }
    bool isPaused() const noexcept { return m_paused; }
    void setAccessibilityMode(bool enabled) { m_accessibilityMode = enabled; }
    bool isAccessibilityMode() const noexcept { return m_accessibilityMode; }
    
    // Input device adaptation
    enum class InputDevice { Keyboard, Mouse, Gamepad };
    void setActiveInputDevice(InputDevice device) { m_activeInputDevice = device; }
    InputDevice getActiveInputDevice() const noexcept { return m_activeInputDevice; }
    
    // Event system
    void setEventCallbacks(const UIEvents& events) { m_events = events; }
    const UIEvents& getEventCallbacks() const noexcept { return m_events; }

    // Config manager forwarding for menus that need to access persistent settings
    void setConfigManager(core::ConfigManager* cfg) { m_configManager = cfg; }
    core::ConfigManager* getConfigManager() const noexcept { return m_configManager; }
    
    // Trigger events (called by menus)
    void triggerStartGame();
    void triggerExit();
    void triggerVolumeChanged(int volume);
    void triggerLanguageChanged(const std::string& language);
    void triggerResolutionChanged(int width, int height);
    void triggerBindingChanged();
    void triggerSaveGame();
    void triggerLoadGame();

private:
    struct MenuEntry {
        std::unique_ptr<Menu> menu;
        AnimationType enterAnimation = AnimationType::None;
        AnimationType exitAnimation = AnimationType::None;
        float animationTime = 0.0f;
        float animationDuration = 0.3f;
        bool isEntering = false;
        bool isExiting = false;
    };
    
    std::vector<MenuEntry> m_menus;
    bool m_paused{false};
    
    struct Notification { 
        std::string text; 
        float elapsed = 0.f; 
        float lifetime = 3.f; 
        sf::Color iconColor = sf::Color::White; 
        float iconSize = 12.f;
        NotificationPriority priority = NotificationPriority::Normal;
        bool isToast = false;
    };
    
    std::vector<Notification> m_notifications;
    float m_notificationDuration{3.f};
    
    // Animation and visual settings
    float m_animationSpeed{1.0f};
    AnimationType m_defaultAnimation{AnimationType::Fade};
    sf::Font m_font;
    bool m_fontLoaded{false};
    std::string m_currentTheme{"default"};
    
    // Custom cursor
    std::unique_ptr<CustomCursor> m_customCursor;
    
    // Accessibility and input
    bool m_accessibilityMode{false};
    InputDevice m_activeInputDevice{InputDevice::Keyboard};
    
    // Event callbacks
    UIEvents m_events;
    // Optional pointer to ConfigManager (not owned)
    core::ConfigManager* m_configManager{nullptr};
    
    // Animation helpers
    void updateAnimations(float dt);
    void renderMenuWithAnimation(sf::RenderWindow& window, const MenuEntry& entry);
    float getAnimationProgress(const MenuEntry& entry) const;
    sf::Transform getAnimationTransform(const MenuEntry& entry, const sf::Vector2f& windowSize) const;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_UIMANAGER_H
