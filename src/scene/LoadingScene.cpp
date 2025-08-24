#include "LoadingScene.h"
#include "SceneManager.h"
#include <iostream>

namespace scene {

LoadingScene::LoadingScene(std::unique_ptr<Scene> nextScene, SceneManager* sceneManager)
    : m_nextScene(std::move(nextScene)), m_sceneManager(sceneManager) {
    loadAssets();
}

void LoadingScene::loadAssets() {
    // Load Loading Screen.png background
    m_backgroundTexture = AssetManager::instance().getTexture("Loading Screen");
    if (m_backgroundTexture) {
        m_backgroundSprite = std::make_unique<sf::Sprite>(*m_backgroundTexture);
        core::Logger::instance().info("LoadingScene: Background texture loaded successfully");
    } else {
        core::Logger::instance().warning("LoadingScene: Failed to load Loading Screen.png");
    }
}

void LoadingScene::onEnter() {
    m_elapsedTime = 0.0f;
    m_transitionReady = false;
    core::Logger::instance().info("LoadingScene: Started loading screen");
}

void LoadingScene::onExit() {
    core::Logger::instance().info("LoadingScene: Loading completed, transitioning to next scene");
}

void LoadingScene::handleEvent(sf::Event& event) {
    // Loading screen doesn't respond to input events
    // Users must wait the full 5 seconds
    (void)event; // Suppress unused parameter warning
}

void LoadingScene::update(float deltaTime) {
    m_elapsedTime += deltaTime;
    
    // Check if loading duration has passed
    if (m_elapsedTime >= LOADING_DURATION && !m_transitionReady) {
        m_transitionReady = true;
        checkTransition();
    }
}

void LoadingScene::render(sf::RenderWindow& window) {
    // Setup background if we have the texture
    if (m_backgroundSprite) {
        setupBackground(window);
        window.draw(*m_backgroundSprite);
    } else {
        // Fallback: clear with dark color
        window.clear(sf::Color(20, 20, 40));
        
        // Draw simple loading text as fallback
        static sf::Font font;
        static bool fontLoaded = false;
        if (!fontLoaded) {
            fontLoaded = font.openFromFile("assets/fonts/Main_font.ttf");
        }
        
        if (fontLoaded) {
            sf::Text loadingText(font, "LOADING...", 48);
            loadingText.setFillColor(sf::Color::White);
            sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
            sf::FloatRect textBounds = loadingText.getLocalBounds();
            loadingText.setPosition(sf::Vector2f(
                (windowSize.x - textBounds.size.x) / 2.0f,
                (windowSize.y - textBounds.size.y) / 2.0f
            ));
            window.draw(loadingText);
        }
    }
    
    // Draw atmospheric loading text and progress bar
    if (m_elapsedTime > 0.0f) {
        sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
        float progress = std::min(m_elapsedTime / LOADING_DURATION, 1.0f);
        
        // Load font for atmospheric text
        static sf::Font font;
        static bool fontLoaded = false;
        if (!fontLoaded) {
            fontLoaded = font.openFromFile("assets/fonts/Secundary_font.ttf");
        }
        
        // Draw atmospheric loading text
        if (fontLoaded) {
            std::string atmosphericText = "La estación ha quedado en silencio. Algo se mueve entre las sombras.\n"
                                        "Antes de adentrarte, asegúrate de estar listo:\n"
                                        "cada decisión puede ser la última. Cargando…";
            
            sf::Text loadingText(font, atmosphericText, 18);
            loadingText.setFillColor(sf::Color(200, 200, 200)); // Light gray
            loadingText.setLineSpacing(1.2f); // Better line spacing
            
            // Center the text above the progress bar
            sf::FloatRect textBounds = loadingText.getLocalBounds();
            loadingText.setPosition(sf::Vector2f(
                (windowSize.x - textBounds.size.x) / 2.0f,
                windowSize.y - 200.0f // Position above progress bar
            ));
            
            // Add subtle glow effect to the text
            sf::Text textGlow = loadingText;
            textGlow.setFillColor(sf::Color(255, 255, 255, 80)); // White glow
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    textGlow.setPosition(loadingText.getPosition() + sf::Vector2f(static_cast<float>(dx), static_cast<float>(dy)));
                    window.draw(textGlow);
                }
            }
            
            window.draw(loadingText);
        }
        
        // Enhanced progress bar with better visual effects
        float barWidth = 500.0f; // Wider bar
        float barHeight = 12.0f; // Taller bar
        float barX = (windowSize.x - barWidth) / 2.0f;
        float barY = windowSize.y - 80.0f; // Moved up a bit
        
        // Background bar with border effect
        sf::RectangleShape barBorder(sf::Vector2f(barWidth + 4.0f, barHeight + 4.0f));
        barBorder.setPosition(sf::Vector2f(barX - 2.0f, barY - 2.0f));
        barBorder.setFillColor(sf::Color(100, 100, 100, 150)); // Dark border
        window.draw(barBorder);
        
        sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
        barBg.setPosition(sf::Vector2f(barX, barY));
        barBg.setFillColor(sf::Color(30, 30, 30)); // Darker background
        window.draw(barBg);
        
        // Animated progress fill with gradient-like effect
        if (progress > 0.0f) {
            float fillWidth = barWidth * progress;
            
            // Main progress fill
            sf::RectangleShape barFill(sf::Vector2f(fillWidth, barHeight));
            barFill.setPosition(sf::Vector2f(barX, barY));
            
            // Color changes based on progress
            sf::Color fillColor;
            if (progress < 0.3f) {
                fillColor = sf::Color(255, 100, 100); // Red start
            } else if (progress < 0.7f) {
                fillColor = sf::Color(255, 200, 100); // Orange middle
            } else {
                fillColor = sf::Color(100, 255, 100); // Green end
            }
            
            barFill.setFillColor(fillColor);
            window.draw(barFill);
            
            // Add pulsing glow effect
            float pulseIntensity = 0.7f + 0.3f * std::sin(m_elapsedTime * 6.0f); // Fast pulse
            sf::RectangleShape barGlow(sf::Vector2f(fillWidth, barHeight + 4.0f));
            barGlow.setPosition(sf::Vector2f(barX, barY - 2.0f));
            
            sf::Color glowColor = fillColor;
            glowColor.a = static_cast<std::uint8_t>(100 * pulseIntensity);
            barGlow.setFillColor(glowColor);
            window.draw(barGlow);
        }
        
        // Progress percentage text
        if (fontLoaded) {
            int percentage = static_cast<int>(progress * 100);
            sf::Text percentText(font, std::to_string(percentage) + "%", 16);
            percentText.setFillColor(sf::Color::White);
            
            sf::FloatRect percentBounds = percentText.getLocalBounds();
            percentText.setPosition(sf::Vector2f(
                barX + barWidth + 10.0f,
                barY - 2.0f
            ));
            window.draw(percentText);
        }
    }
}

void LoadingScene::setupBackground(sf::RenderWindow& window) {
    if (!m_backgroundSprite) return;
    
    // Scale background to fit window
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    sf::Vector2f textureSize = static_cast<sf::Vector2f>(m_backgroundTexture->getSize());
    
    float scaleX = windowSize.x / textureSize.x;
    float scaleY = windowSize.y / textureSize.y;
    
    m_backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
    m_backgroundSprite->setPosition(sf::Vector2f(0.0f, 0.0f));
}

void LoadingScene::checkTransition() {
    if (m_transitionReady && m_nextScene && m_sceneManager) {
        core::Logger::instance().info("LoadingScene: Transitioning to next scene after 5 seconds");
        
        // Replace current loading scene with next scene
        m_sceneManager->replace(std::move(m_nextScene));
    }
}

} // namespace scene
