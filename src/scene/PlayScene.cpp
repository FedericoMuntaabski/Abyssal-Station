#include "PlayScene.h"
#include "SceneManager.h"
#include "MenuScene.h"
#include "../core/Logger.h"
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"

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

    // Initialize entity manager and a player
    m_entityManager = std::make_unique<entities::EntityManager>();
    // create player with id 1 and position matching the rectangle
    auto player = std::make_unique<entities::Player>(1u, m_rect.getPosition(), m_rect.getSize());
    m_player = player.get();
    m_entityManager->addEntity(std::move(player));
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
    // Update velocity based on mapped actions via InputManager
    using input::InputManager;
    using input::Action;

    auto& im = InputManager::getInstance();
    m_velocity = {0.f, 0.f};
    if (im.isActionPressed(Action::MoveLeft))  m_velocity.x -= 1.f;
    if (im.isActionPressed(Action::MoveRight)) m_velocity.x += 1.f;
    if (im.isActionPressed(Action::MoveUp))    m_velocity.y -= 1.f;
    if (im.isActionPressed(Action::MoveDown))  m_velocity.y += 1.f;

    if (m_velocity.x != 0.f || m_velocity.y != 0.f) {
        // normalize
        float len = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        m_velocity /= len;
        m_rect.move(m_velocity * m_speed * dt);
    }

    // Let the player process input first so its velocity is updated
    if (m_player) {
        m_player->handleInput(InputManager::getInstance());
    }

    // Update all entities
    if (m_entityManager) m_entityManager->updateAll(dt);

    // Optional: sync debug rectangle to player position so only one visible cube is moved
    if (m_player) {
        m_rect.setPosition(m_player->position());
    }
}

void PlayScene::render(sf::RenderWindow& window) {
    // Scene-local debug rectangle
    window.draw(m_rect);

    if (m_entityManager) m_entityManager->renderAll(window);
}

} // namespace scene
