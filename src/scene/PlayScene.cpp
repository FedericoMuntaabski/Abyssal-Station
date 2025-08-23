#include "PlayScene.h"
#include "SceneManager.h"
#include "MenuScene.h"
#include "../core/Logger.h"
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"
#include "../entities/Wall.h"
#include "../collisions/CollisionManager.h"
#include "../collisions/CollisionSystem.h"

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
    // Create collision manager and system and wire to entity manager so colliders are registered
    m_collisionManager = std::make_unique<collisions::CollisionManager>();
    m_collisionSystem = std::make_unique<collisions::CollisionSystem>(*m_collisionManager);
    m_entityManager->setCollisionManager(m_collisionManager.get());
    // create player with id 1 and position matching the rectangle
    auto player = std::make_unique<entities::Player>(1u, m_rect.getPosition(), m_rect.getSize());
    m_player = player.get();
    m_entityManager->addEntity(std::move(player));

    // Add a static wall to test collisions (id=2)
    // place wall a bit further on X so it's not touching the player at spawn
    // Move wall a bit further to the right (X) and slightly lower (Y) from previous placement
    auto wall = std::make_unique<entities::Wall>(2u, sf::Vector2f(480.f, 170.f), sf::Vector2f(50.f, 150.f));
    m_entityManager->addEntity(std::move(wall));
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
    }

    // Let the player process input first so its velocity is updated
    if (m_player) {
        m_player->handleInput(InputManager::getInstance());
    }

    // Update all entities (they will update internal state but not commit player movement)
    if (m_entityManager) m_entityManager->updateAll(dt);

    // Pre-move collision test: compute player's intended position and check against CollisionManager
    if (m_collisionManager && m_player) {
        sf::Vector2f intended = m_player->computeIntendedMove(dt);
        sf::FloatRect testBounds;
        testBounds.position.x = intended.x; testBounds.position.y = intended.y;
        testBounds.size.x = m_player->size().x; testBounds.size.y = m_player->size().y;

        auto blocker = m_collisionManager->firstColliderForBounds(testBounds, m_player);
        if (!blocker) {
            // No collision would occur — commit the move
            m_player->commitMove(intended);
        } else {
            // Blocked: optionally log and keep player in place
            core::Logger::instance().info("[PlayScene] Movement blocked for player id=" + std::to_string(m_player->id()) +
                " by entity id=" + std::to_string(blocker->id()));
        }
    }

    // Resolve any residual collisions as a fallback
    if (m_collisionSystem && m_player) {
        m_collisionSystem->resolve(m_player, dt);
    }

    // Sync debug rectangle to player position so visible cube follows authoritative player
    if (m_player) m_rect.setPosition(m_player->position());
}

void PlayScene::render(sf::RenderWindow& window) {
    // Scene-local debug rectangle
    window.draw(m_rect);

    if (m_entityManager) m_entityManager->renderAll(window);
}

} // namespace scene
