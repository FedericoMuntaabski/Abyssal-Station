#ifndef A6C1D2_SCENEMANAGER_H
#define A6C1D2_SCENEMANAGER_H

#include "Scene.h"
#include <stack>
#include <memory>

namespace core { class Logger; }

namespace scene {

class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;

    // Push a new scene on top. The new scene's onEnter() will be called.
    void push(std::unique_ptr<Scene> scene);

    // Pop the current scene. The popped scene's onExit() will be called.
    void pop();

    // Replace the current scene with a new one.
    void replace(std::unique_ptr<Scene> scene);

    // Return pointer to the current active scene or nullptr if none.
    Scene* current();

    // Delegation helpers
    void handleEvent(sf::Event& event);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    std::stack<std::unique_ptr<Scene>> m_scenes;
};

} // namespace scene

#endif // A6C1D2_SCENEMANAGER_H
