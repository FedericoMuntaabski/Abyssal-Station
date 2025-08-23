#include "PlayScene.h"
#include "SceneManager.h"
#include "MenuScene.h"
#include "../core/Logger.h"

#include <cmath>

namespace scene {

using core::Logger;

PlayScene::PlayScene(SceneManager* manager)
    : m_manager(manager) {
}

void PlayScene::onEnter() {
    m_rect.setSize({200.f, 150.f});
    m_rect.setFillColor(sf::Color::Red);
    m_rect.setPosition(sf::Vector2f(200.f, 150.f));
    Logger::instance().info("PlayScene: onEnter");
}

void PlayScene::onExit() {
    Logger::instance().info("PlayScene: onExit");
}

void PlayScene::handleEvent(sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Escape) {
                Logger::instance().info("PlayScene: Escape pressed -> replacing with MenuScene");
                if (m_manager) m_manager->replace(std::make_unique<MenuScene>(m_manager));
            }
        }
    }
}

void PlayScene::update(float dt) {
    // Update velocity based on real-time keyboard state for smoother movement
    m_velocity = {0.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  m_velocity.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) m_velocity.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    m_velocity.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  m_velocity.y += 1.f;

    if (m_velocity.x != 0.f || m_velocity.y != 0.f) {
        // normalize
    float len = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        m_velocity /= len;
        m_rect.move(m_velocity * m_speed * dt);
    }
}

void PlayScene::render(sf::RenderWindow& window) {
    window.draw(m_rect);
}

} // namespace scene
