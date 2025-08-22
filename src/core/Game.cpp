#include "Game.h"
#include <iostream>

Game::Game(unsigned int width, unsigned int height, const std::string& title)
    : m_window(), m_clock(), m_isRunning(false)
{
    // Initialize SFML window
    try {
        initWindow(width, height, title);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Game: " << e.what() << std::endl;
        throw;
    }
}

Game::~Game()
{
    // Ensure the window is closed
    if (m_window.isOpen()) {
        m_window.close();
    }
}

void Game::initWindow(unsigned int width, unsigned int height, const std::string& title)
{
    // SFML 3: VideoMode expects a Vector2u (size) and optional bitsPerPixel.
    sf::VideoMode vm(sf::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));
    m_window.create(vm, title, sf::Style::Close | sf::Style::Titlebar);
    if (!m_window.isOpen()) {
        throw std::runtime_error("Failed to create SFML window");
    }

    // Set a reasonable frame limit; the user can modify later
    m_window.setFramerateLimit(60);
}

void Game::run()
{
    m_isRunning = true;
    m_clock.restart();

    while (m_isRunning && m_window.isOpen()) {
        // Calculate delta time
        float deltaTime = m_clock.restart().asSeconds();

        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::stop()
{
    m_isRunning = false;
}

void Game::processEvents()
{
    // SFML 3: pollEvent() returns std::optional<Event>
    while (auto maybeEvent = m_window.pollEvent()) {
        const sf::Event& event = *maybeEvent;
        if (event.is<sf::Event::Closed>()) {
            stop();
        }
        else if (event.is<sf::Event::Resized>()) {
            if (auto resized = event.getIf<sf::Event::Resized>()) {
                // Vector2u uses .x and .y members
                unsigned int newWidth = resized->size.x;
                unsigned int newHeight = resized->size.y;

                // Create a view matching the new size and center it
                sf::View view;
                view.setSize(sf::Vector2f(static_cast<float>(newWidth), static_cast<float>(newHeight)));
                view.setCenter(sf::Vector2f(static_cast<float>(newWidth) / 2.f, static_cast<float>(newHeight) / 2.f));
                m_window.setView(view);
            }
        }
        // No player/AI/networking logic here per requirements
    }
}

void Game::update(float deltaTime)
{
    // Placeholder for update logic; intentionally empty
    // deltaTime can be used by game systems for animations/physics
    (void)deltaTime;
}

void Game::render()
{
    m_window.clear(sf::Color::Black);

    // Placeholder: no draw calls per requirements

    m_window.display();
}
