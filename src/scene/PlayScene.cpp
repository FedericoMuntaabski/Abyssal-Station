#include "PlayScene.h"
#include "SceneManager.h"
#include "MenuScene.h"
#include "../core/Logger.h"
#include "../core/FontHelper.h"
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"
#include "../entities/Wall.h"
#include "../entities/EntityFactory.h"
#include "../ai/Enemy.h"
#include "../collisions/CollisionManager.h"
#include "../collisions/CollisionSystem.h"

#include "../ui/UIManager.h"
#include "../ui/PauseMenu.h"
#include "../ui/OptionsMenu.h"

#include "../gameplay/Item.h"
#include "../gameplay/ItemManager.h"
#include "../gameplay/Puzzle.h"
#include "../gameplay/PuzzleManager.h"

#include <cmath>
#include <filesystem>

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
    
    // ==================== FACTORY PATTERN DEMO ====================
    // Use EntityFactory to create entities with JSON configuration
    auto& factory = entities::EntityFactory::getInstance();
    
    // Create player using factory with custom configuration
    nlohmann::json playerConfig = {
        {"position", {m_rect.getPosition().x, m_rect.getPosition().y}},
        {"size", {20.0f, 20.0f}},
        {"speed", 220.0f},
        {"health", 120}
    };
    auto player = factory.createPlayer(1u, playerConfig);
    m_player = player.get();
    m_entityManager->addEntity(std::move(player));

    // Create wall using factory with position configuration
    nlohmann::json wallConfig = {
        {"position", {480.0f, 170.0f}},
        {"size", {50.0f, 150.0f}},
        {"color", {100, 100, 100, 255}}
    };
    auto wall = factory.createWall(2u, wallConfig);
    m_entityManager->addEntity(std::move(wall));

    // Create enemies using factory with different behavior profiles
    float enemyOffsetFromWall = 60.f;
    float wallRightX = 480.f + 50.f;
    sf::Vector2f enemyPos(wallRightX + enemyOffsetFromWall, 200.f);
    
    // First enemy: Aggressive behavior with patrol
    nlohmann::json enemy1Config = {
        {"position", {enemyPos.x, enemyPos.y}},
        {"size", {32.0f, 32.0f}},
        {"speed", 100.0f},
        {"visionRange", 200.0f},
        {"attackRange", 24.0f},
        {"behaviorProfile", "AGGRESSIVE"},
        {"patrolPoints", {{enemyPos.x, enemyPos.y}, {enemyPos.x + 120.0f, enemyPos.y}}}
    };
    auto enemy = factory.createEnemy(3u, enemy1Config);
    enemy->setTargetPlayer(m_player);
    m_entityManager->addEntity(std::move(enemy));
    
    // Second enemy: Defensive behavior
    sf::Vector2f enemy2Pos(enemyPos.x + 200.f, enemyPos.y + 20.f);
    nlohmann::json enemy2Config = {
        {"position", {enemy2Pos.x, enemy2Pos.y}},
        {"size", {32.0f, 32.0f}},
        {"speed", 80.0f},
        {"visionRange", 180.0f},
        {"attackRange", 24.0f},
        {"behaviorProfile", "DEFENSIVE"},
        {"patrolPoints", {{enemy2Pos.x, enemy2Pos.y}, {enemy2Pos.x + 100.0f, enemy2Pos.y}}}
    };
    auto enemy2 = factory.createEnemy(4u, enemy2Config);
    enemy2->setTargetPlayer(m_player);
    ai::Enemy* enemy2Ptr = enemy2.get();
    m_entityManager->addEntity(std::move(enemy2));
    
    // Create EnemyManager and register enemies for centralized planning
    m_enemyManager = std::make_unique<ai::EnemyManager>();
    if (auto e3 = dynamic_cast<ai::Enemy*>(m_entityManager->getEntity(3u))) {
        m_enemyManager->addEnemyPointer(e3);
    }
    m_enemyManager->addEnemyPointer(enemy2Ptr);
    
    Logger::instance().info("PlayScene: Created entities using Factory Pattern with configurations");

    // --- Gameplay: ItemManager & PuzzleManager example setup ---
    m_itemManager = std::make_unique<gameplay::ItemManager>(m_collisionManager.get(), m_uiManager.get());
    m_puzzleManager = std::make_unique<gameplay::PuzzleManager>();
    m_puzzleManager->setUIManager(m_uiManager.get());

    // Load resources for interaction hint: try an icon first, fallback to font+text via FontHelper
    bool loadedIcon = false;
    std::string iconPath = "assets/textures/interaction_icon.png";
    if (std::filesystem::exists(iconPath)) {
        if (m_hintTexture.loadFromFile(iconPath)) {
            m_hintSprite = std::make_unique<sf::Sprite>(m_hintTexture);
            m_hintSprite->setOrigin(sf::Vector2f((float)m_hintTexture.getSize().x * 0.5f, (float)m_hintTexture.getSize().y * 0.5f));
            loadedIcon = true;
        }
    }

    if (!loadedIcon) {
        if (core::loadBestFont(m_hintFont)) {
            m_hintText = std::make_unique<sf::Text>(m_hintFont, "Press E", 14u);
            m_hintText->setFillColor(sf::Color::White);
        } else {
            Logger::instance().warning("PlayScene: failed to load hint icon and font fallback");
        }
    }

    // ==================== ITEM CREATION USING FACTORY ====================
    // Create items using factory with different configurations
    sf::Vector2f playerItemPos = m_player ? (m_player->position() + sf::Vector2f(0.f, 50.f)) : sf::Vector2f(m_rect.getPosition().x, m_rect.getPosition().y + 50.f);
    
    // Key item using factory
    nlohmann::json keyConfig = {
        {"position", {playerItemPos.x, playerItemPos.y}},
        {"size", {16.0f, 16.0f}},
        {"itemType", "KEY"},
        {"value", 1}
    };
    auto playerKey = factory.createItem(10u, keyConfig);
    if (auto itemPtr = dynamic_cast<gameplay::Item*>(playerKey.get())) {
        itemPtr->setCollisionManager(m_collisionManager.get());
        m_itemManager->addItem(std::unique_ptr<gameplay::Item>(static_cast<gameplay::Item*>(playerKey.release())));
    }

    // Collectible item at enemy position
    nlohmann::json collectibleConfig = {
        {"position", {enemyPos.x, enemyPos.y}},
        {"size", {16.0f, 16.0f}},
        {"itemType", "COIN"},
        {"value", 25}
    };
    auto enemyKey = factory.createItem(11u, collectibleConfig);
    if (auto itemPtr = dynamic_cast<gameplay::Item*>(enemyKey.get())) {
        itemPtr->setCollisionManager(m_collisionManager.get());
        m_itemManager->addItem(std::unique_ptr<gameplay::Item>(static_cast<gameplay::Item*>(enemyKey.release())));
    }

    // Additional dispersed items using factory
    std::vector<sf::Vector2f> extraPositions = {
        sf::Vector2f(120.f, 200.f),
        sf::Vector2f(300.f, 80.f),
        sf::Vector2f(480.f, 240.f),
        sf::Vector2f(560.f, 160.f)
    };
    uint32_t extraId = 100;
    for (const auto& p : extraPositions) {
        nlohmann::json itemConfig = {
            {"position", {p.x, p.y}},
            {"size", {12.0f, 12.0f}},
            {"itemType", "HEALTH_POTION"},
            {"value", 10}
        };
        auto item = factory.createItem(extraId++, itemConfig);
        if (auto itemPtr = dynamic_cast<gameplay::Item*>(item.get())) {
            itemPtr->setCollisionManager(m_collisionManager.get());
            m_itemManager->addItem(std::unique_ptr<gameplay::Item>(static_cast<gameplay::Item*>(item.release())));
        }
    }

    // Wire item->puzzle binding for demo: picking player-item id=10 completes puzzle id=20 step 0
    m_itemManager->setPuzzleManager(m_puzzleManager.get());
    m_itemManager->bindItemToPuzzleStep(10u, 20u, 0u);

    // Add example puzzle that requires 1 step (collect key)
    std::vector<std::string> steps;
    steps.push_back("HasKey");
    auto puzzle = std::make_unique<gameplay::Puzzle>(20u, sf::Vector2f(m_rect.getPosition().x + 300.f, m_rect.getPosition().y), sf::Vector2f(48.f,48.f), steps);
    m_puzzleManager->registerPuzzle(std::move(puzzle));

    // --- Spawn an item for each enemy automatically ---
    uint32_t spawnItemId = 200;
    if (m_enemyManager) {
        for (auto enemyPtr : m_enemyManager->enemies()) {
            if (!enemyPtr) continue;
            sf::Vector2f spawnPos = enemyPtr->position();
            auto it = std::make_unique<gameplay::Item>(spawnItemId++, spawnPos, sf::Vector2f(16.f, 16.f), gameplay::ItemType::Collectible, m_collisionManager.get());
            m_itemManager->addItem(std::move(it));
        }
    }
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
    // Do not sync or draw debug rectangle on top of player; player entity is rendered separately

    // Update gameplay managers
    if (m_itemManager) m_itemManager->updateAll(dt);
    if (m_puzzleManager) m_puzzleManager->updateAll(dt);

    // Detect nearby item for explicit interaction (player-only)
    m_showInteractHint = false;
    m_nearbyItemId = 0u;
    if (m_collisionManager && m_player && m_itemManager) {
        // Expand player's bounds slightly to define interaction range
    sf::FloatRect playerBounds;
    playerBounds.position = m_player->position();
    playerBounds.size = m_player->size();
    const float interactPadding = 8.f;
    playerBounds.position.x -= interactPadding;
    playerBounds.position.y -= interactPadding;
    playerBounds.size.x += interactPadding * 2.f;
    playerBounds.size.y += interactPadding * 2.f;

        // Query collision manager for any item collider overlapping this area
        auto collider = m_collisionManager->firstColliderForBounds(playerBounds, m_player, entities::kLayerMaskItem);
        if (collider) {
            m_showInteractHint = true;
            m_nearbyItemId = collider->id();
            // If player pressed Interact, trigger centralized interaction
            using input::InputManager;
            using input::Action;
            if (InputManager::getInstance().isActionJustPressed(Action::Interact)) {
                m_itemManager->interactWithItem(m_nearbyItemId, m_player);
            }
        }
    }

    // Update UI manager (menus input & logic)
    if (m_uiManager) m_uiManager->update(dt);
}

void PlayScene::render(sf::RenderWindow& window) {
    // Scene-local debug rectangle (hidden)
    // ...existing code... (m_rect intentionally not drawn so player entity is visible)

    if (m_entityManager) m_entityManager->renderAll(window);

    // Render items and puzzles
    if (m_itemManager) m_itemManager->renderAll(window);
    if (m_puzzleManager) m_puzzleManager->renderAll(window);

    // Render menus on top
    if (m_uiManager) m_uiManager->render(window);

    // Render interaction hint above player if available (sprite preferred)
    if (m_showInteractHint && m_player) {
        sf::Vector2f hintPos = m_player->position();
        hintPos.y -= 18.f; // above player
        // Pulse timer advance
        m_hintPulseTimer += 0.016f;
        float pulse = 1.0f + 0.1f * std::sin(m_hintPulseTimer * 8.0f);
    if (m_hintSprite) {
            m_hintSprite->setPosition(hintPos + sf::Vector2f(m_player->size().x * 0.5f, 0.f));
            m_hintSprite->setScale(sf::Vector2f(pulse, pulse));
            window.draw(*m_hintSprite);
        } else if (m_hintText) {
            m_hintText->setPosition(hintPos + sf::Vector2f(m_player->size().x * 0.5f, 0.f));
            m_hintText->setScale(sf::Vector2f(pulse, pulse));
            window.draw(*m_hintText);
        } else {
            sf::CircleShape circ(6.f * pulse);
            circ.setFillColor(sf::Color::Yellow);
            circ.setPosition(hintPos + sf::Vector2f(m_player->size().x * 0.5f - 6.f, -6.f));
            window.draw(circ);
        }
    }
}

} // namespace scene
