#ifndef ABYSSAL_STATION_SRC_UI_MENU_H
#define ABYSSAL_STATION_SRC_UI_MENU_H

#include <SFML/Graphics.hpp>
#include <string>

namespace ui {

// Abstract base class for UI Menus
class Menu {
public:
    Menu(const std::string& name = "UnnamedMenu");
    virtual ~Menu();

    // Pure virtual interface methods
    // Handle input (polled from InputManager or scene)
    virtual void handleInput() = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    // Optional lifecycle hooks
    virtual void onEnter() {}
    virtual void onExit() {}

    // Activation control
    void activate();
    void deactivate();
    bool isActive() const noexcept;

    const std::string& name() const noexcept;

private:
    bool active_;
    std::string name_;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_MENU_H
