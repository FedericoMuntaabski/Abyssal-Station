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
#include <sstream>
#include <iomanip>

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

    // Initialize pause menu with simplified interface
    m_pauseMenu = std::make_unique<ui::PauseMenu>(m_manager);

    // create UI manager for options and other menus
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
        {"position", {100.0f, 100.0f}}, // Better starting position
        {"size", {24.0f, 24.0f}}, // Slightly larger player
        {"speed", 180.0f}, // Balanced speed
        {"health", 100}
    };
    auto player = factory.createPlayer(1u, playerConfig);
    m_player = player.get();
    m_entityManager->addEntity(std::move(player));

    // ==================== CREATE COMPLEX ROOM LAYOUT ====================
    // Create walls to form a more complex room structure
    
    // Outer perimeter walls
    std::vector<nlohmann::json> wallConfigs = {
        // Top wall
        {{"position", {0.0f, 0.0f}}, {"size", {800.0f, 20.0f}}, {"color", {80, 80, 80, 255}}},
        // Bottom wall  
        {{"position", {0.0f, 580.0f}}, {"size", {800.0f, 20.0f}}, {"color", {80, 80, 80, 255}}},
        // Left wall
        {{"position", {0.0f, 0.0f}}, {"size", {20.0f, 600.0f}}, {"color", {80, 80, 80, 255}}},
        // Right wall
        {{"position", {780.0f, 0.0f}}, {"size", {20.0f, 600.0f}}, {"color", {80, 80, 80, 255}}},
        
        // Interior walls creating rooms and corridors
        // Central room structure
        {{"position", {200.0f, 150.0f}}, {"size", {20.0f, 200.0f}}, {"color", {90, 90, 90, 255}}},
        {{"position", {220.0f, 150.0f}}, {"size", {180.0f, 20.0f}}, {"color", {90, 90, 90, 255}}},
        {{"position", {380.0f, 170.0f}}, {"size", {20.0f, 160.0f}}, {"color", {90, 90, 90, 255}}},
        {{"position", {220.0f, 330.0f}}, {"size", {160.0f, 20.0f}}, {"color", {90, 90, 90, 255}}},
        
        // Side corridors
        {{"position", {500.0f, 100.0f}}, {"size", {20.0f, 150.0f}}, {"color", {85, 85, 85, 255}}},
        {{"position", {520.0f, 100.0f}}, {"size", {150.0f, 20.0f}}, {"color", {85, 85, 85, 255}}},
        {{"position", {500.0f, 350.0f}}, {"size", {200.0f, 20.0f}}, {"color", {85, 85, 85, 255}}},
        
        // Small rooms and obstacles
        {{"position", {600.0f, 200.0f}}, {"size", {80.0f, 20.0f}}, {"color", {95, 95, 95, 255}}},
        {{"position", {600.0f, 220.0f}}, {"size", {20.0f, 80.0f}}, {"color", {95, 95, 95, 255}}},
        {{"position", {660.0f, 280.0f}}, {"size", {20.0f, 70.0f}}, {"color", {95, 95, 95, 255}}},
        
        // Additional pillars and obstacles
        {{"position", {120.0f, 280.0f}}, {"size", {40.0f, 40.0f}}, {"color", {100, 70, 70, 255}}},
        {{"position", {450.0f, 450.0f}}, {"size", {30.0f, 30.0f}}, {"color", {70, 100, 70, 255}}},
        {{"position", {300.0f, 450.0f}}, {"size", {25.0f, 50.0f}}, {"color", {70, 70, 100, 255}}}
    };
    
    uint32_t wallId = 2;
    for (const auto& wallConfig : wallConfigs) {
        auto wall = factory.createWall(wallId++, wallConfig);
        m_entityManager->addEntity(std::move(wall));
    }

    // ==================== CREATE COMPLEX ENEMY LAYOUT ====================
    // Create multiple enemies with different behaviors and positions
    m_enemyManager = std::make_unique<ai::EnemyManager>();
    
    // Patrol Guard - guards the central room entrance
    nlohmann::json guardConfig = {
        {"position", {240.0f, 200.0f}},
        {"size", {28.0f, 28.0f}},
        {"speed", 90.0f},
        {"visionRange", 150.0f},
        {"attackRange", 32.0f},
        {"behaviorProfile", "DEFENSIVE"},
        {"patrolPoints", {{240.0f, 200.0f}, {320.0f, 200.0f}, {320.0f, 280.0f}, {240.0f, 280.0f}}}
    };
    auto guard = factory.createEnemy(50u, guardConfig);
    guard->setTargetPlayer(m_player);
    ai::Enemy* guardPtr = guard.get();
    m_entityManager->addEntity(std::move(guard));
    m_enemyManager->addEnemyPointer(guardPtr);
    
    // Aggressive Hunter - roams the upper corridor
    nlohmann::json hunterConfig = {
        {"position", {550.0f, 140.0f}},
        {"size", {30.0f, 30.0f}},
        {"speed", 120.0f},
        {"visionRange", 200.0f},
        {"attackRange", 24.0f},
        {"behaviorProfile", "AGGRESSIVE"},
        {"patrolPoints", {{550.0f, 140.0f}, {650.0f, 140.0f}}}
    };
    auto hunter = factory.createEnemy(51u, hunterConfig);
    hunter->setTargetPlayer(m_player);
    ai::Enemy* hunterPtr = hunter.get();
    m_entityManager->addEntity(std::move(hunter));
    m_enemyManager->addEnemyPointer(hunterPtr);
    
    // Wandering Scout - moves around the lower area
    nlohmann::json scoutConfig = {
        {"position", {400.0f, 480.0f}},
        {"size", {26.0f, 26.0f}},
        {"speed", 70.0f},
        {"visionRange", 120.0f},
        {"attackRange", 28.0f},
        {"behaviorProfile", "PASSIVE"},
        {"patrolPoints", {{400.0f, 480.0f}, {500.0f, 480.0f}, {500.0f, 520.0f}, {400.0f, 520.0f}}}
    };
    auto scout = factory.createEnemy(52u, scoutConfig);
    scout->setTargetPlayer(m_player);
    ai::Enemy* scoutPtr = scout.get();
    m_entityManager->addEntity(std::move(scout));
    m_enemyManager->addEnemyPointer(scoutPtr);
    
    // Corner Sentry - defends the upper right corner
    nlohmann::json sentryConfig = {
        {"position", {620.0f, 250.0f}},
        {"size", {32.0f, 32.0f}},
        {"speed", 80.0f},
        {"visionRange", 180.0f},
        {"attackRange", 36.0f},
        {"behaviorProfile", "DEFENSIVE"},
        {"patrolPoints", {{620.0f, 250.0f}, {650.0f, 250.0f}}}
    };
    auto sentry = factory.createEnemy(53u, sentryConfig);
    sentry->setTargetPlayer(m_player);
    ai::Enemy* sentryPtr = sentry.get();
    m_entityManager->addEntity(std::move(sentry));
    m_enemyManager->addEnemyPointer(sentryPtr);
    
    Logger::instance().info("PlayScene: Created entities using Factory Pattern with configurations");

    // --- Gameplay: ItemManager & PuzzleManager example setup ---
    m_itemManager = std::make_unique<gameplay::ItemManager>(m_collisionManager.get(), m_uiManager.get());
    m_puzzleManager = std::make_unique<gameplay::PuzzleManager>();
    m_puzzleManager->setUIManager(m_uiManager.get());
    
    // --- NEW: Initialize advanced gameplay systems ---
    m_noiseSystem = std::make_unique<gameplay::NoiseSystem>();
    m_visionSystem = std::make_unique<gameplay::VisionSystem>();
    // Initialize vision system with window dimensions (standard 800x600 for now)
    m_visionSystem->initialize(800, 600);

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

    // ==================== ENHANCED ITEM PLACEMENT ====================
    // Strategic item placement throughout the complex level
    std::vector<std::pair<sf::Vector2f, std::string>> itemPlacements = {
        // Key items in strategic locations
        {{130.0f, 140.0f}, "KEY"},           // Near starting area
        {{280.0f, 200.0f}, "KEY"},           // Central room
        {{580.0f, 160.0f}, "KEY"},           // Upper corridor
        
        // Health potions in dangerous areas
        {{350.0f, 250.0f}, "HEALTH_POTION"}, // Central room
        {{650.0f, 120.0f}, "HEALTH_POTION"}, // Near aggressive enemy
        {{450.0f, 500.0f}, "HEALTH_POTION"}, // Lower area
        {{140.0f, 320.0f}, "HEALTH_POTION"}, // Near pillar
        
        // Collectible coins for score/progression
        {{60.0f, 60.0f}, "COIN"},            // Corner reward
        {{740.0f, 60.0f}, "COIN"},           // Upper right corner
        {{60.0f, 540.0f}, "COIN"},           // Lower left corner
        {{740.0f, 540.0f}, "COIN"},          // Lower right corner
        {{280.0f, 300.0f}, "COIN"},          // Central room reward
        {{480.0f, 200.0f}, "COIN"},          // Corridor junction
        {{600.0f, 320.0f}, "COIN"},          // Side room
        
        // Special items requiring exploration
        {{320.0f, 480.0f}, "ENERGY_CRYSTAL"}, // Hidden in lower area
        {{680.0f, 280.0f}, "ENERGY_CRYSTAL"}, // Behind enemy position
        {{80.0f, 300.0f}, "ENERGY_CRYSTAL"},   // Near pillar obstacle
    };
    
    uint32_t itemId = 100;
    for (const auto& placement : itemPlacements) {
        nlohmann::json itemConfig = {
            {"position", {placement.first.x, placement.first.y}},
            {"size", {14.0f, 14.0f}},
            {"itemType", placement.second},
            {"value", placement.second == "COIN" ? 25 : 
                     placement.second == "HEALTH_POTION" ? 20 :
                     placement.second == "KEY" ? 1 :
                     placement.second == "ENERGY_CRYSTAL" ? 50 : 10}
        };
        auto item = factory.createItem(itemId++, itemConfig);
        if (auto itemPtr = dynamic_cast<gameplay::Item*>(item.get())) {
            itemPtr->setCollisionManager(m_collisionManager.get());
            m_itemManager->addItem(std::unique_ptr<gameplay::Item>(static_cast<gameplay::Item*>(item.release())));
        }
    }

    // ==================== ENHANCED PUZZLE SYSTEM ====================
    // Wire item->puzzle binding for demo: collecting multiple keys unlocks areas
    m_itemManager->setPuzzleManager(m_puzzleManager.get());
    
    // Puzzle 1: Main door - requires 2 keys
    std::vector<std::string> mainDoorSteps = {"HasKey", "HasKey"};
    auto mainDoorPuzzle = std::make_unique<gameplay::Puzzle>(200u, sf::Vector2f(200.0f, 120.0f), sf::Vector2f(60.f, 30.f), mainDoorSteps);
    m_puzzleManager->registerPuzzle(std::move(mainDoorPuzzle));
    
    // Puzzle 2: Side room access - requires 1 key + energy crystal
    std::vector<std::string> sideRoomSteps = {"HasKey", "HasCrystal"};
    auto sideRoomPuzzle = std::make_unique<gameplay::Puzzle>(201u, sf::Vector2f(520.0f, 120.0f), sf::Vector2f(50.f, 20.f), sideRoomSteps);
    m_puzzleManager->registerPuzzle(std::move(sideRoomPuzzle));
    
    // Puzzle 3: Emergency exit - requires health potion (safety check)
    std::vector<std::string> emergencySteps = {"HasHealth"};
    auto emergencyPuzzle = std::make_unique<gameplay::Puzzle>(202u, sf::Vector2f(380.0f, 350.0f), sf::Vector2f(20.f, 40.f), emergencySteps);
    m_puzzleManager->registerPuzzle(std::move(emergencyPuzzle));
    
    // Bind specific items to puzzle steps
    m_itemManager->bindItemToPuzzleStep(100u, 200u, 0u); // First key to main door step 0
    m_itemManager->bindItemToPuzzleStep(101u, 200u, 1u); // Second key to main door step 1
    m_itemManager->bindItemToPuzzleStep(102u, 201u, 0u); // Third key to side room step 0
    m_itemManager->bindItemToPuzzleStep(117u, 201u, 1u); // Energy crystal to side room step 1
    m_itemManager->bindItemToPuzzleStep(103u, 202u, 0u); // Health potion to emergency exit

    // ==================== ENHANCED ENEMY SPAWNING ====================
    // Spawn items near each enemy for risk/reward gameplay
    uint32_t spawnItemId = 300;
    if (m_enemyManager) {
        for (auto enemyPtr : m_enemyManager->enemies()) {
            if (!enemyPtr) continue;
            sf::Vector2f spawnPos = enemyPtr->position() + sf::Vector2f(40.0f, 0.0f); // Offset from enemy
            auto riskRewardItem = std::make_unique<gameplay::Item>(spawnItemId++, spawnPos, sf::Vector2f(16.f, 16.f), gameplay::ItemType::Collectible, m_collisionManager.get());
            m_itemManager->addItem(std::move(riskRewardItem));
        }
    }
    
    Logger::instance().info("PlayScene: Enhanced complex level initialized with " + 
                           std::to_string(m_enemyManager->enemies().size()) + " enemies, " +
                           std::to_string(itemPlacements.size()) + " strategic items, and " +
                           "3 multi-step puzzles");
    
    // ==================== INITIALIZE AUTO-SAVE SYSTEM ====================
    m_saveManager = std::make_unique<core::SaveManager>();
    m_saveManager->enableAutoSave(true);
    m_saveManager->setAutoSaveInterval(120.0f); // Auto-save every 2 minutes
    Logger::instance().info("PlayScene: Auto-save enabled with 2-minute interval");
    
    // ==================== INITIALIZE DEBUG SYSTEMS ====================
    m_debugConsole = std::make_unique<debug::DebugConsole>();
    m_debugConsole->setPlayerReference(m_player);
    
    m_debugOverlay = std::make_unique<debug::DebugOverlay>();
    m_debugOverlay->setEntityManager(m_entityManager.get());
    m_debugOverlay->setEnemyManager(m_enemyManager.get());
    m_debugOverlay->setCollisionManager(m_collisionManager.get());
    
    Logger::instance().info("PlayScene: Debug systems initialized (F1 for console, F3 for overlay)");
    
    // ==================== INITIALIZE SURVIVAL HUD ====================
    m_survivalHUD = std::make_unique<ui::SurvivalHUD>();
    m_survivalHUD->setPlayer(m_player);
    // TODO: Set font when AssetManager is available
    // m_survivalHUD->setAssetManager(assetManager);
    
    Logger::instance().info("PlayScene: Survival HUD initialized with vital signs display");
}

void PlayScene::onExit() {
    Logger::instance().info("PlayScene: onExit");
}

void PlayScene::handleEvent(sf::Event& event) {
    // Debug console input handling (highest priority)
    if (m_debugConsole) {
        if (event.is<sf::Event::KeyPressed>()) {
            if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F1) {
                    m_debugConsole->toggle();
                    return; // Consume the event
                }
                if (kp->code == sf::Keyboard::Key::F3) {
                    if (m_debugOverlay) {
                        m_debugOverlay->toggle();
                    }
                    return; // Consume the event
                }
            }
        }
        
        // If console is open, let it handle all input
        if (m_debugConsole->isOpen()) {
            m_debugConsole->handleEvent(event);
            return;
        }
    }
    
    // Regular game input handling
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Escape) {
                // Toggle pause menu directly
                if (m_pauseMenu) {
                    m_pauseMenu->toggle();
                }
            }
        }
    }
}

void PlayScene::update(float dt) {
    // Update velocity based on mapped actions via InputManager
    using input::InputManager;
    using input::Action;

    // Update pause menu
    if (m_pauseMenu) {
        m_pauseMenu->update(dt);
    }

    // Handle pause menu input
    if (m_pauseMenu && m_pauseMenu->isVisible()) {
        m_pauseMenu->handleInput(input::InputManager::getInstance());
        return; // Don't process other inputs when pause menu is active
    }

    // First, let UI manager handle menus and input for them
    if (m_uiManager) m_uiManager->update(dt);

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
    
    // --- NEW: Update advanced gameplay systems ---
    if (m_noiseSystem && m_player) {
        // Generate player movement noise using specific methods
        if (m_player->isRunning()) {
            m_noiseSystem->generatePlayerRunningNoise(m_player);
        } else {
            // Check if player is moving
            sf::Vector2f oldPos = m_player->position(); // This would need to be stored from previous frame
            // For now, just generate walking noise if not stationary
            m_noiseSystem->generatePlayerWalkingNoise(m_player);
        }
        
        // Update noise system (decay old events)
        m_noiseSystem->update(dt);
        
        // Make enemies respond to noise events
        if (m_enemyManager) {
            const auto& noiseEvents = m_noiseSystem->getNoiseEvents();
            for (auto& enemy : m_enemyManager->enemies()) {
                if (!enemy) continue;
                
                // Check each noise event for this enemy
                for (const auto& noise : noiseEvents) {
                    sf::Vector2f enemyPos = enemy->position();
                    sf::Vector2f noisePos = noise.position;
                    
                    // Calculate distance to noise
                    float dx = noisePos.x - enemyPos.x;
                    float dy = noisePos.y - enemyPos.y;
                    float distance = std::sqrt(dx * dx + dy * dy);
                    
                    // Enemy hearing range (configurable)
                    float hearingRange = 200.0f;
                    
                    // If noise is within hearing range, notify enemy
                    if (distance <= hearingRange) {
                        // Intensity decreases with distance
                        float adjustedIntensity = noise.intensity * (1.0f - (distance / hearingRange));
                        enemy->onSoundHeard(noisePos, adjustedIntensity);
                    }
                }
            }
        }
    }
    
    if (m_visionSystem && m_player) {
        // Update vision system with player reference
        m_visionSystem->update(dt, m_player);
    }

    // Update auto-save system
    if (m_saveManager && m_player) {
        // Create current game state for auto-save
        core::GameState currentState;
        core::GameState::PlayerState playerState;
        playerState.id = m_player->id();
        sf::Vector2f pos = m_player->position();
        playerState.x = pos.x;
        playerState.y = pos.y;
        playerState.health = 100; // Default value, could be enhanced to read from player
        currentState.players.push_back(playerState);
        
        // Add collected items and completed puzzles if available
        // These would need to be tracked by the game systems
        
        m_saveManager->update(dt, currentState);
        
        if (m_saveManager->shouldAutoSave()) {
            bool success = m_saveManager->performAutoSave(currentState);
            if (success && m_uiManager) {
                m_uiManager->showToast("Juego guardado automaticamente", 3.0f, sf::Color::Green);
                Logger::instance().info("PlayScene: Auto-save completed successfully");
            }
        }
    }
    
    // Track survival time for achievements (placeholder for future achievement system)
    m_survivalTime += dt;
    
    // Update debug systems
    if (m_debugConsole) {
        m_debugConsole->update(dt);
    }
    if (m_debugOverlay) {
        m_debugOverlay->update(dt);
        m_debugOverlay->updateFrameTime(dt);
        
        // Add custom metrics
        m_debugOverlay->setCustomMetric("Survival Time", formatFloat(m_survivalTime, 1) + "s");
        if (m_player) {
            sf::Vector2f pos = m_player->position();
            m_debugOverlay->setCustomMetric("Player Pos", "(" + formatFloat(pos.x, 0) + ", " + formatFloat(pos.y, 0) + ")");
        }
    }
    
    // Update survival HUD
    if (m_survivalHUD && m_player) {
        m_survivalHUD->update(dt);
    }

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

std::string PlayScene::formatFloat(float value, int precision) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

void PlayScene::render(sf::RenderWindow& window) {
    // Scene-local debug rectangle (hidden)
    // ...existing code... (m_rect intentionally not drawn so player entity is visible)

    if (m_entityManager) m_entityManager->renderAll(window);

    // Render items and puzzles
    if (m_itemManager) m_itemManager->renderAll(window);
    if (m_puzzleManager) m_puzzleManager->renderAll(window);
    
    // --- NEW: Render vision system (darkness and vision cone) ---
    if (m_visionSystem && m_player) {
        m_visionSystem->render(window, m_player);
    }
    
    // --- NEW: Render noise debug info (for development) ---
    if (m_noiseSystem && m_noiseSystem->isDebugMode()) {
        m_noiseSystem->renderDebug(window);
    }

    // Render menus on top
    if (m_uiManager) m_uiManager->render(window);
    
    // Render pause menu (above everything else)
    if (m_pauseMenu) {
        m_pauseMenu->render(window);
    }

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
    
    // Render survival HUD (UI layer, above game elements)
    if (m_survivalHUD) {
        m_survivalHUD->render(window);
    }
    
    // Render debug systems (on top of everything)
    if (m_debugOverlay) {
        m_debugOverlay->render(window);
    }
    if (m_debugConsole) {
        m_debugConsole->render(window);
    }
}

} // namespace scene
