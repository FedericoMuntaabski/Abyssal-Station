#ifndef A2B9B8_GAME_H
#define A2B9B8_GAME_H

#include <SFML/Graphics.hpp>
#include <string>

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
};

#endif // A2B9B8_GAME_H
