#ifndef ABYSSAL_STATION_SRC_UI_PAUSEMENU_H
#define ABYSSAL_STATION_SRC_UI_PAUSEMENU_H

#include "Menu.h"
#include "InputHelper.h"
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <vector>
#include <string>
#include <memory>

namespace scene { class SceneManager; }
namespace ui { class UIManager; }

namespace ui {

class PauseMenu : public Menu {
public:
    explicit PauseMenu(scene::SceneManager* manager, ui::UIManager* uiManager = nullptr);
    ~PauseMenu() override = default;

    // Menu interface
    void handleInput() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

    // Should be called by the owner each frame to allow opening the pause
    // menu when the Pause action (e.g., Escape) is pressed.
    void processToggleInput();

private:
    scene::SceneManager* m_manager{nullptr};
    ui::UIManager* m_uiManager{nullptr};
    std::vector<std::string> m_options;
    std::size_t m_selected{0};
    float m_startY{120.f};
    float m_spacing{36.f};
    
    // Visual animation state
    std::vector<float> m_scales;
    std::vector<float> m_glowIntensity;
    
    // Device detection and input helper
    std::unique_ptr<InputHelper> m_inputHelper;
    InputHelper::DeviceType m_activeDevice{InputHelper::DeviceType::Keyboard};
    
    // Audio members
    std::unique_ptr<sf::Sound> m_hoverSound;
    std::unique_ptr<sf::Sound> m_confirmSound;
    std::shared_ptr<sf::SoundBuffer> m_hoverBuffer;
    std::shared_ptr<sf::SoundBuffer> m_confirmBuffer;
    bool m_audioLoaded{false};
    
    // Helper methods
    void updateActiveDevice();
    void handleMouseHover(sf::RenderWindow& window);
    std::string getContextualHint() const;
    void loadAudio();
    void playHoverSound();
    void playConfirmSound();
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_PAUSEMENU_H
