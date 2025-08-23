#include "DebugPickupScene.h"
#include "SceneManager.h"
#include "../core/Logger.h"
#include "../entities/EntityManager.h"
#include "../entities/Player.h"
#include "../collisions/CollisionManager.h"
#include "../collisions/CollisionSystem.h"
#include "../gameplay/ItemManager.h"
#include "../gameplay/PuzzleManager.h"
#include "../gameplay/Item.h"
#include "../gameplay/Puzzle.h"

namespace scene {

using core::Logger;

DebugPickupScene::DebugPickupScene(SceneManager* manager)
    : m_manager(manager)
{
}

void DebugPickupScene::onEnter()
{
    Logger::instance().info("DebugPickupScene: onEnter");
    m_rect.setSize({32.f, 32.f});
    m_rect.setFillColor(sf::Color::Blue);
    m_rect.setPosition(sf::Vector2f(100.f, 100.f));

    // Setup managers locally like PlayScene minimal example
    auto entityManager = std::make_unique<entities::EntityManager>();
    auto collisionManager = std::make_unique<collisions::CollisionManager>();

    // create player
    auto player = std::make_unique<entities::Player>(1u, m_rect.getPosition(), m_rect.getSize());
    entities::Player* pptr = player.get();
    entityManager->addEntity(std::move(player));

    // ItemManager + PuzzleManager
    auto itemManager = std::make_unique<gameplay::ItemManager>(collisionManager.get(), nullptr);
    auto puzzleManager = std::make_unique<gameplay::PuzzleManager>();

        // Spawn two test items:
        // 1) 50 px below the player's spawn
        sf::Vector2f debugPlayerPos(100.f, 100.f);
        sf::Vector2f playerItemPos = debugPlayerPos + sf::Vector2f(0.f, 50.f);
        auto playerKey = std::make_unique<gameplay::Item>(10u, playerItemPos, sf::Vector2f(16.f,16.f), gameplay::ItemType::Key, collisionManager.get());
        itemManager->addItem(std::move(playerKey));

        // 2) at enemy position
        sf::Vector2f debugEnemyPos(240.f, 70.f);
        sf::Vector2f enemyItemPos = debugEnemyPos;
        auto enemyKey = std::make_unique<gameplay::Item>(11u, enemyItemPos, sf::Vector2f(16.f,16.f), gameplay::ItemType::Collectible, collisionManager.get());
        itemManager->addItem(std::move(enemyKey));

    std::vector<std::string> steps; steps.push_back("HasKey");
    auto puzzle = std::make_unique<gameplay::Puzzle>(20u, sf::Vector2f(360.f, 100.f), sf::Vector2f(48.f,48.f), steps);
    puzzleManager->registerPuzzle(std::move(puzzle));

    // Wire binding
    itemManager->setPuzzleManager(puzzleManager.get());
    itemManager->bindItemToPuzzleStep(10u, 20u, 0u);

    // Keep them alive in scene via heap storage (simple approach)
    // ...store pointers in static variables for debug run
    static auto s_entityManager = std::move(entityManager);
    static auto s_collisionManager = std::move(collisionManager);
    static auto s_itemManager = std::move(itemManager);
    static auto s_puzzleManager = std::move(puzzleManager);

    // simple: move player towards the key each update using global pointers above
}

void DebugPickupScene::update(float dt)
{
    m_elapsed += dt;
    // nothing here; logic lives in static managers for this simple debug helper
}

void DebugPickupScene::render(sf::RenderWindow& window)
{
    window.draw(m_rect);
}

void DebugPickupScene::onExit()
{
    Logger::instance().info("DebugPickupScene: onExit");
}

} // namespace scene
