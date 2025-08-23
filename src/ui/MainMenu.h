#ifndef ABYSSAL_STATION_SRC_UI_MAINMENU_H
#define ABYSSAL_STATION_SRC_UI_MAINMENU_H

#include "Menu.h"
#include <vector>
#include <string>

namespace scene { class SceneManager; }
namespace ui { class UIManager; }

namespace ui {

class MainMenu : public Menu {
public:
    explicit MainMenu(scene::SceneManager* manager, ui::UIManager* uiManager = nullptr);
    ~MainMenu() override = default;

    void handleInput() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

private:
    scene::SceneManager* m_manager{nullptr};
    ui::UIManager* m_uiManager{nullptr};
    std::vector<std::string> m_options;
    std::size_t m_selected{0};
    // simple layout constants
    float m_startY{150.f};
    float m_spacing{40.f};
    // Hover animation state: target and current scale per option
    std::vector<float> m_scales;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_MAINMENU_H
