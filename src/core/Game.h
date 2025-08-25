#ifndef A2B9B8_GAME_H
#define A2B9B8_GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <string>

namespace scene { class SceneManager; }

namespace core { class ConfigManager; }

namespace audio { class AudioManager; }

class Game {
public:
    // Construct a Game with an optional window size and title
    Game(unsigned int width = 1280, unsigned int height = 720, const std::string& title = "Abyssal Station");
    ~Game();

    // Run the main loop (blocking)
    void run();

    // Stop the main loop and begin shutdown
    void stop();

private:
    void initWindow(unsigned int width, unsigned int height, const std::string& title);
    void processEvents();
    void update(float deltaTime);
    void render();

    sf::RenderWindow m_window;
    sf::Clock m_clock; // for delta time
    bool m_isRunning;
    // FPS display helpers
    std::string m_title;
    float m_fps{0.0f};
    float m_fpsAccumulator{0.0f};
    int m_fpsFrames{0};

    // Background & sound test
    std::shared_ptr<sf::Texture> m_backgroundTexture;

    std::shared_ptr<sf::SoundBuffer> m_sfxBuffer;
    std::unique_ptr<sf::Sound> m_sound;

    // Scene manager for game scenes
    std::unique_ptr<scene::SceneManager> m_sceneManager;
    // Global config manager (owned by the executable)
    std::unique_ptr<core::ConfigManager> m_configManager;
    // Audio manager for dynamic audio
    std::unique_ptr<audio::AudioManager> m_audioManager;
};

#endif // A2B9B8_GAME_H
