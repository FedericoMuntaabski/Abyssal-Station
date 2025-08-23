#ifndef ABYSSAL_STATION_SRC_UI_MAINMENU_H
#define ABYSSAL_STATION_SRC_UI_MAINMENU_H

#include "Menu.h"
#include "InputHelper.h"
#include <vector>
#include <string>
#include <memory>

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
    std::unique_ptr<InputHelper> m_inputHelper;
    
    std::vector<std::string> m_options;
    std::size_t m_selected{0};
    
    // Visual layout
    float m_startY{150.f};
    float m_spacing{45.f};
    
    // Hover animation state: target and current scale per option
    std::vector<float> m_scales;
    std::vector<float> m_glowIntensity;
    float m_backgroundPulse{0.f};
    
    // Device adaptation
    InputHelper::DeviceType m_activeDevice{InputHelper::DeviceType::Auto};
    
    // Helper methods
    void updateActiveDevice();
    void handleMouseHover(sf::RenderWindow& window);
    void renderBackground(sf::RenderWindow& window);
    void renderTitle(sf::RenderWindow& window);
    void renderOptions(sf::RenderWindow& window);
    void renderHints(sf::RenderWindow& window);
    std::string getContextualHint() const;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_MAINMENU_H
