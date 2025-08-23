#ifndef ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
#define ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H

#include "Menu.h"
#include "InputHelper.h"
#include <vector>
#include <string>
#include <memory>
#include "../input/Action.h"

namespace scene { class SceneManager; }
namespace core { class ConfigManager; }

namespace ui {

class OptionsMenu : public Menu {
public:
    explicit OptionsMenu(scene::SceneManager* manager = nullptr, core::ConfigManager* configManager = nullptr);
    ~OptionsMenu() override = default;

    void handleInput() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

    // Simple getters/setters for tests or external UIManager
    int volume() const noexcept;
    void setVolume(int vol);
    
    const std::string& language() const noexcept;
    void setLanguage(const std::string& lang);
    
    std::pair<int, int> resolution() const noexcept;
    void setResolution(int width, int height);

private:
    enum class MenuMode {
        Main,       // Main options (Volume, Resolution, Language, Controls, Back)
        Controls,   // Control bindings list
        Remapping   // Waiting for new input during remapping
    };
    
    scene::SceneManager* m_manager{nullptr};
    core::ConfigManager* m_configManager{nullptr};
    std::unique_ptr<InputHelper> m_inputHelper;
    
    std::vector<std::string> m_mainOptions;
    std::vector<input::Action> m_actionsToShow;
    
    MenuMode m_mode{MenuMode::Main};
    std::size_t m_selected{0};
    std::size_t m_selectedAction{0};
    
    // Settings
    int m_volume{100}; // 0..100
    std::string m_language{"en"};
    int m_resolutionWidth{1920};
    int m_resolutionHeight{1080};
    std::vector<std::pair<int, int>> m_availableResolutions;
    std::size_t m_selectedResolution{0};
    
    // Visual settings
    float m_startY{120.f};
    float m_spacing{40.f};
    float m_controlsStartY{100.f};
    float m_controlsSpacing{35.f};
    
    // Animation and feedback
    std::vector<float> m_scales;
    std::vector<float> m_actionScales;
    float m_remapPulse{0.f};
    
    // Remapping state
    bool m_waitingForRemap{false};
    input::Action m_actionToRemap{input::Action::MoveUp};
    bool m_acceptingCombo{false};
    std::vector<sf::Keyboard::Key> m_currentCombo;
    float m_remapTimeout{5.0f};
    float m_remapTimer{0.0f};
    
    // Device adaptation
    InputHelper::DeviceType m_activeDevice{InputHelper::DeviceType::Auto};
    
    // Helper methods
    void initializeOptions();
    void initializeResolutions();
    void applyVolume();
    void applyLanguage();
    void applyResolution();
    void saveSettings();
    void loadSettings();
    
    // Input handling for different modes
    void handleMainMenuInput();
    void handleControlsInput();
    void handleRemappingInput();
    
    // Rendering for different modes
    void renderMainMenu(sf::RenderWindow& window);
    void renderControlsMenu(sf::RenderWindow& window);
    void renderRemappingOverlay(sf::RenderWindow& window);
    
    // UI helpers
    void renderOption(sf::RenderWindow& window, const std::string& label, const std::string& value,
                     const sf::Vector2f& position, bool selected, float scale = 1.0f);
    void renderControlBinding(sf::RenderWindow& window, input::Action action, 
                            const sf::Vector2f& position, bool selected, float scale = 1.0f);
    void renderHint(sf::RenderWindow& window, const std::string& text, const sf::Vector2f& position);
    
    // Value adjustment helpers
    void adjustVolume(int delta);
    void cycleLanguage();
    void cycleResolution();
    
    // Device detection and hints
    void updateActiveDevice();
    std::string getContextualHint() const;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
