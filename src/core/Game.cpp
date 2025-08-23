#include "Game.h"
#include "core/AssetManager.h"
#include "../scene/SceneManager.h"
#include "../scene/MenuScene.h"
#include <filesystem>
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

    // Load assets for a quick smoke test (search up the filesystem for an assets/ folder)
    try {
        namespace fs = std::filesystem;
        auto findAssetsSubdir = [&](const std::string& subdir) -> std::string {
            fs::path p = fs::current_path();
            for (int i = 0; i < 6; ++i) {
                fs::path candidate = p / "assets" / subdir;
                if (fs::exists(candidate) && fs::is_directory(candidate)) {
                    return candidate.string();
                }
                if (p.has_parent_path()) p = p.parent_path();
                else break;
            }
            return std::string();
        };

        std::string texPath = findAssetsSubdir("textures");
        if (!texPath.empty()) {
            AssetManager::instance().loadTexturesFrom(texPath);
        } else {
            std::cerr << "Error: assets/textures folder not found (searched upwards)\n";
        }

        std::string sndPath = findAssetsSubdir("sounds");
        if (!sndPath.empty()) {
            AssetManager::instance().loadSoundsFrom(sndPath);
        } else {
            std::cerr << "Error: assets/sounds folder not found (searched upwards)\n";
        }

        m_backgroundTexture = AssetManager::instance().getTexture("background");
        if (!m_backgroundTexture) {
            std::cerr << "Warning: background texture not found\n";
        }

        m_sfxBuffer = AssetManager::instance().getSound("sound_test");
        if (m_sfxBuffer) {
            // sf::Sound requires a buffer at construction in this SFML version
            m_sound = std::make_unique<sf::Sound>(*m_sfxBuffer);
            // Play looped for demo
            m_sound->setLooping(true);
        } else {
            std::cerr << "Warning: sound_test sound not found\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Asset loading error: " << e.what() << std::endl;
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

    // store base title for FPS updates
    m_title = title;
}

void Game::run()
{
    m_isRunning = true;
    m_clock.restart();

    // Play a short SFX on start (if available)
    if (m_sound) {
        m_sound->play();
    }

    // Initialize scene manager and push the initial MenuScene
    m_sceneManager = std::make_unique<scene::SceneManager>();
    m_sceneManager->push(std::make_unique<scene::MenuScene>(m_sceneManager.get()));

    while (m_isRunning && m_window.isOpen()) {
        // Calculate delta time
        float deltaTime = m_clock.restart().asSeconds();

        processEvents();

        // If no scenes left, stop the game
        if (!m_sceneManager->current()) {
            stop();
            break;
        }

        // Delegate update to scene manager
        m_sceneManager->update(deltaTime);

        // Update other game logic
        update(deltaTime);

        // Render: first background, then current scene, then display
        m_window.clear(sf::Color::Black);

        // Draw background if available
        if (m_backgroundTexture) {
            sf::Sprite bg(*m_backgroundTexture);
            auto winSize = m_window.getSize();
            sf::Vector2f texSize(static_cast<float>(m_backgroundTexture->getSize().x), static_cast<float>(m_backgroundTexture->getSize().y));
            if (texSize.x > 0 && texSize.y > 0) {
                float scaleX = static_cast<float>(winSize.x) / texSize.x;
                float scaleY = static_cast<float>(winSize.y) / texSize.y;
                bg.setScale(sf::Vector2f(scaleX, scaleY));
            }
            m_window.draw(bg);
        }

        // Let current scene render
        m_sceneManager->render(m_window);

        m_window.display();
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
        // Delegate events to current scene if present
        if (m_sceneManager && m_sceneManager->current()) {
            // dispatch a non-const event copy because our Scene API expects a non-const reference
            sf::Event ev = event;
            m_sceneManager->handleEvent(ev);
        }
        // No player/AI/networking logic here per requirements
    }
}

void Game::update(float deltaTime)
{
    // Update FPS counter and refresh title once per second
    m_fpsAccumulator += deltaTime;
    m_fpsFrames += 1;

    if (m_fpsAccumulator >= 1.0f) {
        m_fps = static_cast<float>(m_fpsFrames) / m_fpsAccumulator;
        m_fpsAccumulator = 0.0f;
        m_fpsFrames = 0;

        // update title with FPS
        try {
            if (!m_title.empty())
                m_window.setTitle(m_title + " - FPS: " + std::to_string(static_cast<int>(m_fps + 0.5f)));
        } catch (...) {
            // Ignore any window title update errors
        }
    }

    // Ensure the SFX keeps playing: some backends might stop instead of looping reliably,
    // so replay if it's stopped (this is a safe fallback even if setLooping(true) is used).
    if (m_sound) {
        try {
            auto status = m_sound->getStatus();
            if (status == sf::SoundSource::Status::Stopped) {
                m_sound->play();
            }
        } catch (...) {
            // don't let audio errors break the update loop
        }
    }
}

void Game::render()
{
    // This method is now intentionally empty: rendering is handled inside the main loop
}
