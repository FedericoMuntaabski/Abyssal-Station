#ifndef ABYSSAL_STATION_SRC_UI_PAUSEMENU_H
#define ABYSSAL_STATION_SRC_UI_PAUSEMENU_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <vector>
#include <string>
#include <memory>

namespace scene { class SceneManager; }
namespace input { class InputManager; }

namespace ui {

class PauseMenu {
public:
    explicit PauseMenu(scene::SceneManager* manager);
    ~PauseMenu() = default;

    // Core functionality
    void handleInput(input::InputManager& inputManager);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    // Menu state
    void show();
    void hide();
    void toggle();
    bool isVisible() const { return visible_; }

private:
    enum class Option {
        Resume = 0,
        MainMenu = 1,
        OptionCount = 2
    };

    static constexpr float INPUT_DELAY = 0.2f;

    // Core components
    scene::SceneManager* sceneManager_;
    
    // Menu state
    bool visible_;
    int selectedOption_;
    
    // Audio
    bool audioLoaded_;
    std::shared_ptr<sf::SoundBuffer> hoverBuffer_;
    std::shared_ptr<sf::SoundBuffer> confirmBuffer_;
    std::unique_ptr<sf::Sound> hoverSound_;
    std::unique_ptr<sf::Sound> confirmSound_;
    
    // Input handling
    float inputCooldown_;
    
    // Menu elements
    sf::RectangleShape overlay_;
    sf::RectangleShape menuBackground_;
    sf::Font font_;
    std::vector<std::unique_ptr<sf::Text>> menuTexts_;

    // Private methods
    void initializeMenu();
    void loadAudio();
    void updateMenuSelection();
    void executeSelectedOption();
    void playHoverSound();
    void playConfirmSound();
    std::string getOptionText(Option option) const;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_PAUSEMENU_H
