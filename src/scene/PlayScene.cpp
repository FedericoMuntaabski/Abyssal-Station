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
#include "../gameplay/AchievementManager.h"
#include "../ui/OptionsMenu.h"

#include "../gameplay/Item.h"
#include "../gameplay/ItemManager.h"
#include "../gameplay/Puzzle.h"
#include "../gameplay/PuzzleManager.h"

#include <cmath>
#include <filesystem>
#include <ctime>

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

    // Create wall using factory with position configuration - Main structural wall
    nlohmann::json wallConfig = {
        {"position", {480.0f, 170.0f}},
        {"size", {50.0f, 150.0f}},
        {"color", {100, 100, 100, 255}}
    };
    auto wall = factory.createWall(2u, wallConfig);
    m_entityManager->addEntity(std::move(wall));

    // Create additional walls to form a more complex station layout
    // Top horizontal corridor wall
    nlohmann::json topWallConfig = {
        {"position", {200.0f, 100.0f}},
        {"size", {400.0f, 30.0f}},
        {"color", {80, 80, 80, 255}}
    };
    auto topWall = factory.createWall(5u, topWallConfig);
    m_entityManager->addEntity(std::move(topWall));

    // Bottom horizontal corridor wall  
    nlohmann::json bottomWallConfig = {
        {"position", {200.0f, 380.0f}},
        {"size", {400.0f, 30.0f}},
        {"color", {80, 80, 80, 255}}
    };
    auto bottomWall = factory.createWall(6u, bottomWallConfig);
    m_entityManager->addEntity(std::move(bottomWall));

    // Left vertical corridor wall
    nlohmann::json leftWallConfig = {
        {"position", {150.0f, 130.0f}},
        {"size", {30.0f, 250.0f}},
        {"color", {80, 80, 80, 255}}
    };
    auto leftWall = factory.createWall(7u, leftWallConfig);
    m_entityManager->addEntity(std::move(leftWall));

    // Right vertical corridor wall
    nlohmann::json rightWallConfig = {
        {"position", {650.0f, 130.0f}},
        {"size", {30.0f, 250.0f}},
        {"color", {80, 80, 80, 255}}
    };
    auto rightWall = factory.createWall(8u, rightWallConfig);
    m_entityManager->addEntity(std::move(rightWall));

    // Interior room walls to create compartments
    nlohmann::json interiorWall1Config = {
        {"position", {300.0f, 200.0f}},
        {"size", {20.0f, 80.0f}},
        {"color", {90, 90, 90, 255}}
    };
    auto interiorWall1 = factory.createWall(9u, interiorWall1Config);
    m_entityManager->addEntity(std::move(interiorWall1));

    nlohmann::json interiorWall2Config = {
        {"position", {550.0f, 150.0f}},
        {"size", {20.0f, 120.0f}},
        {"color", {90, 90, 90, 255}}
    };
    auto interiorWall2 = factory.createWall(10u, interiorWall2Config);
    m_entityManager->addEntity(std::move(interiorWall2));

    // Create enemies using factory with different behavior profiles and strategic positions
    float enemyOffsetFromWall = 60.f;
    float wallRightX = 480.f + 50.f;
    
    // First enemy: Aggressive patroller in main corridor
    sf::Vector2f enemyPos(wallRightX + enemyOffsetFromWall, 200.f);
    nlohmann::json enemy1Config = {
        {"position", {enemyPos.x, enemyPos.y}},
        {"size", {32.0f, 32.0f}},
        {"speed", 100.0f},
        {"visionRange", 200.0f},
        {"attackRange", 24.0f},
        {"behaviorProfile", "AGGRESSIVE"},
        {"patrolPoints", {{enemyPos.x, enemyPos.y}, {enemyPos.x + 120.0f, enemyPos.y}}}
    };
    auto enemy1 = factory.createEnemy(3u, enemy1Config);
    enemy1->setTargetPlayer(m_player);
    m_entityManager->addEntity(std::move(enemy1));
    
    // Second enemy: Defensive guard in upper area
    sf::Vector2f enemy2Pos(320.f, 160.f);
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

    // Third enemy: Sentinel in bottom corridor
    sf::Vector2f enemy3Pos(450.f, 340.f);
    nlohmann::json enemy3Config = {
        {"position", {enemy3Pos.x, enemy3Pos.y}},
        {"size", {28.0f, 28.0f}},
        {"speed", 90.0f},
        {"visionRange", 160.0f},
        {"attackRange", 20.0f},
        {"behaviorProfile", "AGGRESSIVE"},
        {"patrolPoints", {{enemy3Pos.x, enemy3Pos.y}, {enemy3Pos.x - 80.0f, enemy3Pos.y}}}
    };
    auto enemy3 = factory.createEnemy(11u, enemy3Config);
    enemy3->setTargetPlayer(m_player);
    ai::Enemy* enemy3Ptr = enemy3.get();
    m_entityManager->addEntity(std::move(enemy3));

    // Fourth enemy: Roaming hunter in right area
    sf::Vector2f enemy4Pos(600.f, 250.f);
    nlohmann::json enemy4Config = {
        {"position", {enemy4Pos.x, enemy4Pos.y}},
        {"size", {30.0f, 30.0f}},
        {"speed", 110.0f},
        {"visionRange", 220.0f},
        {"attackRange", 26.0f},
        {"behaviorProfile", "AGGRESSIVE"},
        {"patrolPoints", {{enemy4Pos.x, enemy4Pos.y}, {enemy4Pos.x, enemy4Pos.y + 60.0f}, {enemy4Pos.x - 40.0f, enemy4Pos.y + 60.0f}, {enemy4Pos.x - 40.0f, enemy4Pos.y}}}
    };
    auto enemy4 = factory.createEnemy(12u, enemy4Config);
    enemy4->setTargetPlayer(m_player);
    ai::Enemy* enemy4Ptr = enemy4.get();
    m_entityManager->addEntity(std::move(enemy4));
    
    // Create EnemyManager and register all enemies for centralized planning
    m_enemyManager = std::make_unique<ai::EnemyManager>();
    if (auto e1 = dynamic_cast<ai::Enemy*>(m_entityManager->getEntity(3u))) {
        m_enemyManager->addEnemyPointer(e1);
    }
    m_enemyManager->addEnemyPointer(enemy2Ptr);
    m_enemyManager->addEnemyPointer(enemy3Ptr);
    m_enemyManager->addEnemyPointer(enemy4Ptr);
    
    Logger::instance().info("PlayScene: Created entities using Factory Pattern with configurations");

    // --- Gameplay: ItemManager & PuzzleManager example setup ---
    m_itemManager = std::make_unique<gameplay::ItemManager>(m_collisionManager.get(), m_uiManager.get());
    m_puzzleManager = std::make_unique<gameplay::PuzzleManager>();
    m_puzzleManager->setUIManager(m_uiManager.get());
    
    // --- Achievement System ---
    m_achievementManager = std::make_unique<gameplay::AchievementManager>();
    m_achievementManager->loadAchievements(); // Load existing progress
    
    // Set up achievement unlock callback for UI notifications
    m_achievementManager->setOnAchievementUnlocked([this](const gameplay::Achievement& achievement) {
        if (m_uiManager) {
            std::string message = "¡Logro desbloqueado! " + achievement.name + ": " + achievement.description;
            m_uiManager->showSuccessNotification(message, 5.0f);
        }
        core::Logger::instance().info("Achievement unlocked: " + achievement.name);
    });
    
    // Connect achievement manager to item manager for collection tracking
    m_itemManager->setAchievementManager(m_achievementManager.get());
    
    // Add strategic items throughout the station complex
    // Medical supplies in safe alcoves
    sf::Vector2f medkitPos1(250.f, 180.f); // Left corridor
    auto medkit1 = std::make_unique<gameplay::Item>(100u, medkitPos1, sf::Vector2f(16.f, 16.f), gameplay::ItemType::Tool, m_collisionManager.get());
    m_itemManager->addItem(std::move(medkit1));

    sf::Vector2f medkitPos2(580.f, 320.f); // Bottom right area
    auto medkit2 = std::make_unique<gameplay::Item>(101u, medkitPos2, sf::Vector2f(16.f, 16.f), gameplay::ItemType::Tool, m_collisionManager.get());
    m_itemManager->addItem(std::move(medkit2));

    // Energy cells in corners
    sf::Vector2f energyPos1(220.f, 140.f); // Top left corner
    auto energy1 = std::make_unique<gameplay::Item>(102u, energyPos1, sf::Vector2f(12.f, 12.f), gameplay::ItemType::Collectible, m_collisionManager.get());
    m_itemManager->addItem(std::move(energy1));

    sf::Vector2f energyPos2(620.f, 140.f); // Top right corner  
    auto energy2 = std::make_unique<gameplay::Item>(103u, energyPos2, sf::Vector2f(12.f, 12.f), gameplay::ItemType::Collectible, m_collisionManager.get());
    m_itemManager->addItem(std::move(energy2));

    sf::Vector2f energyPos3(220.f, 350.f); // Bottom left corner
    auto energy3 = std::make_unique<gameplay::Item>(104u, energyPos3, sf::Vector2f(12.f, 12.f), gameplay::ItemType::Collectible, m_collisionManager.get());
    m_itemManager->addItem(std::move(energy3));

    // Weapon/tools in central areas (high risk, high reward)
    sf::Vector2f weaponPos1(400.f, 240.f); // Central area
    auto weapon1 = std::make_unique<gameplay::Item>(105u, weaponPos1, sf::Vector2f(18.f, 18.f), gameplay::ItemType::Tool, m_collisionManager.get());
    m_itemManager->addItem(std::move(weapon1));

    sf::Vector2f weaponPos2(520.f, 190.f); // Near interior wall
    auto weapon2 = std::make_unique<gameplay::Item>(106u, weaponPos2, sf::Vector2f(18.f, 18.f), gameplay::ItemType::Tool, m_collisionManager.get());
    m_itemManager->addItem(std::move(weapon2));

    // Key cards for puzzle elements
    sf::Vector2f keycardPos1(350.f, 300.f); // Protected by enemy patrol
    auto keycard1 = std::make_unique<gameplay::Item>(107u, keycardPos1, sf::Vector2f(14.f, 14.f), gameplay::ItemType::Key, m_collisionManager.get());
    m_itemManager->addItem(std::move(keycard1));

    sf::Vector2f keycardPos2(570.f, 280.f); // In enemy territory
    auto keycard2 = std::make_unique<gameplay::Item>(108u, keycardPos2, sf::Vector2f(14.f, 14.f), gameplay::ItemType::Key, m_collisionManager.get());
    m_itemManager->addItem(std::move(keycard2));

    // Documents/lore items for worldbuilding
    sf::Vector2f documentPos1(280.f, 210.f);
    auto document1 = std::make_unique<gameplay::Item>(109u, documentPos1, sf::Vector2f(10.f, 10.f), gameplay::ItemType::Collectible, m_collisionManager.get());
    m_itemManager->addItem(std::move(document1));

    sf::Vector2f documentPos2(590.f, 170.f);
    auto document2 = std::make_unique<gameplay::Item>(110u, documentPos2, sf::Vector2f(10.f, 10.f), gameplay::ItemType::Collectible, m_collisionManager.get());
    m_itemManager->addItem(std::move(document2));

    Logger::instance().info("PlayScene: Added 11 strategic items throughout station complex");
    
    // --- Achievement System ---
    m_achievementManager = std::make_unique<gameplay::AchievementManager>();
    m_achievementManager->loadAchievements(); // Load existing progress
    
    // Set up achievement unlock callback for UI notifications
    m_achievementManager->setOnAchievementUnlocked([this](const gameplay::Achievement& achievement) {
        if (m_uiManager) {
            std::string message = "¡Logro desbloqueado! " + achievement.name + ": " + achievement.description;
            m_uiManager->showSuccessNotification(message, 5.0f);
        }
        core::Logger::instance().info("Achievement unlocked: " + achievement.name);
    });
    
    Logger::instance().info("PlayScene: Achievement system initialized");
    
    // Initialize auto-save system
    m_saveManager = std::make_unique<core::SaveManager>();
    m_saveManager->enableAutoSave(true);
    m_saveManager->setAutoSaveInterval(120.0f);  // Auto-save every 2 minutes
    Logger::instance().info("PlayScene: Auto-save system initialized (2 min interval)");

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
    // Save achievements before leaving the scene
    if (m_achievementManager) {
        m_achievementManager->saveAchievements();
    }
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

    // Track survival time for achievements (only when not paused)
    m_survivalTime += dt;
    if (m_achievementManager) {
        // Update survival achievements
        m_achievementManager->updateProgress("survivor", static_cast<int>(m_survivalTime));
        m_achievementManager->updateProgress("veteran_survivor", static_cast<int>(m_survivalTime));
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

    // Update auto-save system
    if (m_saveManager && m_player) {
        // Create a basic game state for auto-save
        core::GameState currentState;
        
        // Add player state
        core::GameState::PlayerState playerState;
        playerState.id = m_player->id();
        auto pos = m_player->position();
        playerState.x = pos.x;
        playerState.y = pos.y;
        playerState.health = 100; // Default health for now
        
        currentState.players.push_back(playerState);
        
        m_saveManager->update(dt, currentState);
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
