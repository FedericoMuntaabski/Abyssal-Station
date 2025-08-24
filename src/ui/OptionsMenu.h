#ifndef ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
#define ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H

#include "Menu.h"
#include "InputHelper.h"
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <vector>
#include <string>
#include <memory>

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
    scene::SceneManager* m_manager{nullptr};
    core::ConfigManager* m_configManager{nullptr};
    std::unique_ptr<InputHelper> m_inputHelper;
    
    std::vector<std::string> m_mainOptions;
    
    std::size_t m_selected{0};
    
    // Settings
    int m_musicVolume{80}; // 0..100
    int m_uiVolume{90}; // 0..100
    std::string m_language{"es"};
    int m_resolutionWidth{1920};
    int m_resolutionHeight{1080};
    bool m_fullscreen{false};
    std::vector<std::pair<int, int>> m_availableResolutions;
    std::size_t m_selectedResolution{0};
    
    // Visual settings
    float m_startY{120.f};
    float m_spacing{40.f};
    
    // Animation
    std::vector<float> m_scales;
    
    // Device adaptation
    InputHelper::DeviceType m_activeDevice{InputHelper::DeviceType::Auto};
    
    // Audio members
    std::unique_ptr<sf::Sound> m_hoverSound;
    std::unique_ptr<sf::Sound> m_confirmSound;
    std::shared_ptr<sf::SoundBuffer> m_hoverBuffer;
    std::shared_ptr<sf::SoundBuffer> m_confirmBuffer;
    bool m_audioLoaded{false};
    
    // Helper methods
    void initializeOptions();
    void initializeResolutions();
    void saveSettings();
    void loadSettings();
    void updateActiveDevice();
    void handleMouseHover(sf::RenderWindow& window);
    void renderBackground(sf::RenderWindow& window);
    std::string getOptionDisplayText(size_t index) const;
    std::string getContextualHint() const;
    void loadAudio();
    void playHoverSound();
    void playConfirmSound();
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
