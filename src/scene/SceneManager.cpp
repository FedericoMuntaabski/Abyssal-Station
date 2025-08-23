#include "SceneManager.h"
#include "../core/Logger.h"

#include <utility>

namespace scene {

using core::Logger;

void SceneManager::push(std::unique_ptr<Scene> scene) {
    if (!scene) return;

    if (!m_scenes.empty()) {
        // call onExit on previous top
        m_scenes.top()->onExit();
        Logger::instance().info("SceneManager: pushing new scene, calling onExit on previous scene");
    }

    Logger::instance().info("SceneManager: pushing scene");
    scene->onEnter();
    m_scenes.push(std::move(scene));
}

void SceneManager::pop() {
    if (m_scenes.empty()) {
        Logger::instance().warning("SceneManager: pop() called but stack is empty");
        return;
    }

    Logger::instance().info("SceneManager: popping current scene");
    m_scenes.top()->onExit();
    m_scenes.pop();

    if (!m_scenes.empty()) {
        m_scenes.top()->onEnter();
        Logger::instance().info("SceneManager: resumed previous scene and called onEnter");
    }
}

void SceneManager::replace(std::unique_ptr<Scene> scene) {
    if (!scene) return;

    Logger::instance().info("SceneManager: replacing current scene");
    if (!m_scenes.empty()) {
        m_scenes.top()->onExit();
        m_scenes.pop();
    }

    scene->onEnter();
    m_scenes.push(std::move(scene));
}

Scene* SceneManager::current() {
    if (m_scenes.empty()) return nullptr;
    return m_scenes.top().get();
}

std::size_t SceneManager::size() const {
    return m_scenes.size();
}

void SceneManager::handleEvent(sf::Event& event) {
    Scene* cur = current();
    if (cur) cur->handleEvent(event);
}

void SceneManager::update(float dt) {
    Scene* cur = current();
    if (cur) cur->update(dt);
}

void SceneManager::render(sf::RenderWindow& window) {
    Scene* cur = current();
    if (cur) cur->render(window);
}

} // namespace scene
