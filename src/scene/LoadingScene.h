#ifndef ABYSSAL_STATION_SRC_SCENE_LOADINGSCENE_H
#define ABYSSAL_STATION_SRC_SCENE_LOADINGSCENE_H

#include "Scene.h"
#include "../core/Logger.h"
#include "../core/AssetManager.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace scene {

// Forward declaration
class SceneManager;

/**
 * @brief LoadingScene - Shows loading screen with background image for 5 seconds
 * 
 * Displays Loading Screen.png background for exactly 5 seconds before transitioning
 * to the target scene (typically PlayScene).
 */
class LoadingScene : public Scene {
public:
    /**
     * @brief Constructor
     * @param nextScene The scene to transition to after loading completes
     * @param sceneManager Pointer to scene manager for transitions
     */
    LoadingScene(std::unique_ptr<Scene> nextScene, SceneManager* sceneManager);
    
    virtual ~LoadingScene() = default;

    // Scene interface implementation
    void handleEvent(sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onEnter() override;
    void onExit() override;

private:
    std::unique_ptr<Scene> m_nextScene;
    SceneManager* m_sceneManager{nullptr};
    
    // Visual elements
    std::shared_ptr<sf::Texture> m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundSprite;
    
    // Timing
    static constexpr float LOADING_DURATION = 5.0f; // 5 seconds
    float m_elapsedTime{0.0f};
    bool m_transitionReady{false};
    
    // Helper methods
    void loadAssets();
    void setupBackground(sf::RenderWindow& window);
    void checkTransition();
};

} // namespace scene

#endif // ABYSSAL_STATION_SRC_SCENE_LOADINGSCENE_H
