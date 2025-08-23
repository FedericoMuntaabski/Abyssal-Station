#ifndef ABYSSAL_STATION_SRC_UI_UIMANAGER_H
#define ABYSSAL_STATION_SRC_UI_UIMANAGER_H

#include "Menu.h"
#include <SFML/Graphics/Color.hpp>
#include <vector>
#include <memory>
#include <cstdint>

namespace ui {

// Simple manager that owns menus and provides stack-like operations.
class UIManager {
public:
    UIManager() = default;
    ~UIManager();

    // Take ownership of a menu pointer and push it as the active menu.
    // The menu pointer must be allocated with new (owner transferred).
    void pushMenu(Menu* menu);

    // Pop the top menu (if any) and destroy it after calling onExit.
    void popMenu();

    // Return pointer to the current top menu or nullptr.
    Menu* currentMenu() const noexcept;

    // Query helpers
    bool isMenuActive(const std::string& name) const noexcept;
    bool isAnyMenuActive() const noexcept;

    // Delegate update/render to active menus (those with isActive()==true).
    void update(float dt);
    void render(sf::RenderWindow& window);

    // Convenience: close all menus
    void clear();

    // Lightweight notifications from game systems (item pickup, puzzle completion)
    void notifyItemCollected(std::uint32_t id);
    void notifyItemCollected(std::uint32_t id, sf::Color iconColor);
    void notifyPuzzleCompleted(std::uint32_t id);
    void notifyPuzzleCompleted(std::uint32_t id, sf::Color iconColor);

    // Set how long notifications stay visible (seconds)
    void setNotificationDuration(float seconds) { m_notificationDuration = seconds; }

private:
    std::vector<std::unique_ptr<Menu>> m_menus;
    bool m_paused{false};

    struct Notification { std::string text; float elapsed=0.f; float lifetime=3.f; sf::Color iconColor = sf::Color::White; float iconSize = 12.f; };
    std::vector<Notification> m_notifications;
    float m_notificationDuration{3.f};
public:
    void setPaused(bool p) { m_paused = p; }
    bool isPaused() const noexcept { return m_paused; }
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_UIMANAGER_H
