#include "PlayScene.h"
#include "SceneManager.h"
#include "MenuScene.h"
#include "../core/Logger.h"
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"
#include "../entities/Wall.h"
#include "../ai/Enemy.h"
#include "../collisions/CollisionManager.h"
#include "../collisions/CollisionSystem.h"

#include "../ui/UIManager.h"
#include "../ui/PauseMenu.h"
#include "../ui/OptionsMenu.h"

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

    // create UI manager for pause/options menus
    m_uiManager = std::make_unique<ui::UIManager>();

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

    // Add an enemy to the right of the wall on the X axis and offset away from the wall (~60 px)
    float enemyOffsetFromWall = 60.f;
    float enemyWidth = 32.f;
    // wall's right X = wall.position.x + wall.size.x = 480 + 50 = 530
    float wallRightX = 480.f + 50.f;
    sf::Vector2f enemyPos(wallRightX + enemyOffsetFromWall, 200.f);
    // provide simple patrol points so the enemy will patrol visibly to the right
    std::vector<sf::Vector2f> patrolPoints;
    patrolPoints.push_back(enemyPos);
    patrolPoints.push_back(sf::Vector2f(enemyPos.x + 120.f, enemyPos.y));
    auto enemy = std::make_unique<ai::Enemy>(3u, enemyPos, sf::Vector2f(enemyWidth, 32.f), 100.f, 200.f, 24.f, patrolPoints);
    // Attach player target so enemy can chase
    enemy->setTargetPlayer(m_player);
    m_entityManager->addEntity(std::move(enemy));
    // Create EnemyManager and register enemies for centralized planning
    m_enemyManager = std::make_unique<ai::EnemyManager>();
    // Register the previously added enemy (id=3) - find it in entity manager
    if (auto e3 = dynamic_cast<ai::Enemy*>(m_entityManager->getEntity(3u))) {
        m_enemyManager->addEnemyPointer(e3);
    }

    // Spawn a second enemy to test multiple AI interactions (id=4) positioned further right
    sf::Vector2f enemy2Pos(enemyPos.x + 200.f, enemyPos.y + 20.f);
    std::vector<sf::Vector2f> patrol2;
    patrol2.push_back(enemy2Pos);
    patrol2.push_back(sf::Vector2f(enemy2Pos.x + 100.f, enemy2Pos.y));
    auto enemy2 = std::make_unique<ai::Enemy>(4u, enemy2Pos, sf::Vector2f(enemyWidth, 32.f), 80.f, 180.f, 24.f, patrol2);
    enemy2->setTargetPlayer(m_player);
    // Keep ownership in EntityManager, but register pointer in EnemyManager
    ai::Enemy* enemy2Ptr = enemy2.get();
    m_entityManager->addEntity(std::move(enemy2));
    m_enemyManager->addEnemyPointer(enemy2Ptr);
}

void PlayScene::onExit() {
    Logger::instance().info("PlayScene: onExit");
}

void PlayScene::handleEvent(sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Escape) {
                // Toggle pause menu via UIManager
                if (m_uiManager && m_uiManager->currentMenu() && m_uiManager->currentMenu()->name() == "PauseMenu") {
                    m_uiManager->popMenu();
                } else if (m_uiManager) {
                    m_uiManager->pushMenu(new ui::PauseMenu(m_manager, m_uiManager.get()));
                }
            }
        }
    }
}

void PlayScene::update(float dt) {
    // Update velocity based on mapped actions via InputManager
    using input::InputManager;
    using input::Action;

    // First, let UI manager handle menus and input for them
    if (m_uiManager) m_uiManager->update(dt);

    // If PauseMenu is active, freeze game logic (don't process movement/AI)
    if (m_uiManager && m_uiManager->isMenuActive("PauseMenu")) {
        return; // game frozen while paused
    }

    auto& im = InputManager::getInstance();
    m_velocity = {0.f, 0.f};

    // If any menu active, do not propagate movement inputs to the game
    if (!(m_uiManager && m_uiManager->isAnyMenuActive())) {
        if (im.isActionPressed(Action::MoveLeft))  m_velocity.x -= 1.f;
        if (im.isActionPressed(Action::MoveRight)) m_velocity.x += 1.f;
        if (im.isActionPressed(Action::MoveUp))    m_velocity.y -= 1.f;
        if (im.isActionPressed(Action::MoveDown))  m_velocity.y += 1.f;
    }

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

    // Centralized enemy planning & commit via EnemyManager
    if (m_enemyManager) {
        // Run FSM updates for all enemies
        if (m_player) m_enemyManager->updateAll(dt, m_player->position());
        // Plan movement (collision-aware)
        m_enemyManager->planAllMovement(dt, m_collisionManager.get());
        // Commit moves after collision checks
        m_enemyManager->commitAllMoves(m_collisionManager.get());
        // Resolve residual collisions for each enemy
        if (m_collisionSystem) {
            for (auto& ep : m_enemyManager->enemies()) {
                if (ep) m_collisionSystem->resolve(ep, dt);
            }
        }
    }

    // Sync debug rectangle to player position so visible cube follows authoritative player
    if (m_player) m_rect.setPosition(m_player->position());

    // Update UI manager (menus input & logic)
    if (m_uiManager) m_uiManager->update(dt);
}

void PlayScene::render(sf::RenderWindow& window) {
    // Scene-local debug rectangle
    window.draw(m_rect);

    if (m_entityManager) m_entityManager->renderAll(window);

    // Render menus on top
    if (m_uiManager) m_uiManager->render(window);
}

} // namespace scene
