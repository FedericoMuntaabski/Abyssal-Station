#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_GAMEPLAYEXAMPLE_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_GAMEPLAYEXAMPLE_H

#include "ItemManager.h"
#include "PuzzleManager.h"
#include "../collisions/CollisionManager.h"
#include "../ui/UIManager.h"
#include "../entities/Player.h"
#include <memory>

namespace gameplay {

/**
 * Example showing how to use the improved gameplay system
 */
class GameplayExample {
public:
    GameplayExample();
    ~GameplayExample();

    void setupExample();
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    // Save/Load example
    void saveGameState(const std::string& filename);
    bool loadGameState(const std::string& filename);
    
    // Statistics example
    void printStatistics();

private:
    std::unique_ptr<collisions::CollisionManager> collisionManager_;
    std::unique_ptr<ui::UIManager> uiManager_;
    std::unique_ptr<ItemManager> itemManager_;
    std::unique_ptr<PuzzleManager> puzzleManager_;
    std::unique_ptr<entities::Player> player_;
};

// Implementation
GameplayExample::GameplayExample()
    : collisionManager_(std::make_unique<collisions::CollisionManager>())
    , uiManager_(std::make_unique<ui::UIManager>())
    , itemManager_(std::make_unique<ItemManager>(collisionManager_.get(), uiManager_.get()))
    , puzzleManager_(std::make_unique<PuzzleManager>())
    , player_(std::make_unique<entities::Player>(1, sf::Vector2f(100, 100)))
{
    // Connect managers
    itemManager_->setPuzzleManager(puzzleManager_.get());
    puzzleManager_->setUIManager(uiManager_.get());
}

GameplayExample::~GameplayExample() = default;

void GameplayExample::setupExample()
{
    // Create some example items
    auto key = std::make_unique<Item>(1, sf::Vector2f(200, 100), sf::Vector2f(16, 16), ItemType::Key);
    auto tool = std::make_unique<Item>(2, sf::Vector2f(300, 100), sf::Vector2f(16, 16), ItemType::Tool);
    auto collectible = std::make_unique<Item>(3, sf::Vector2f(400, 100), sf::Vector2f(16, 16), ItemType::Collectible);
    
    // Add items to manager
    itemManager_->addItem(std::move(key));
    itemManager_->addItem(std::move(tool));
    itemManager_->addItem(std::move(collectible));
    
    // Create a puzzle that requires the key
    std::vector<std::string> puzzleSteps = {
        "Find the key",
        "Unlock the door",
        "Enter the room"
    };
    auto puzzle = std::make_unique<Puzzle>(10, sf::Vector2f(500, 200), sf::Vector2f(64, 64), puzzleSteps);
    puzzleManager_->registerPuzzle(std::move(puzzle));
    
    // Bind key item to first puzzle step
    itemManager_->bindItemToPuzzleStep(1, 10, 0); // Item ID 1 completes step 0 of puzzle 10
    
    // Add player to collision system
    collisionManager_->addCollider(player_.get(), sf::FloatRect(player_->position(), player_->size()));
    
    core::Logger::instance().info("Gameplay example setup complete");
}

void GameplayExample::update(float deltaTime)
{
    // Update all systems
    player_->update(deltaTime);
    itemManager_->updateAll(deltaTime);
    puzzleManager_->updateAll(deltaTime);
    
    // Example: Check if player is near puzzle to activate it
    auto* puzzle = puzzleManager_->getPuzzleById(10);
    if (puzzle && puzzle->state() == PuzzleState::Locked) {
        sf::Vector2f playerPos = player_->position();
        sf::Vector2f puzzlePos = puzzle->position();
        float distance = std::sqrt(std::pow(playerPos.x - puzzlePos.x, 2) + std::pow(playerPos.y - puzzlePos.y, 2));
        
        if (distance < 100.0f) { // Player within 100 pixels
            puzzle->playerJoin(player_.get());
        }
    }
}

void GameplayExample::render(sf::RenderWindow& window)
{
    player_->render(window);
    itemManager_->renderAll(window);
    puzzleManager_->renderAll(window);
}

void GameplayExample::saveGameState(const std::string& filename)
{
    // Save items and puzzles separately for better organization
    itemManager_->saveToJson(filename + "_items.json");
    puzzleManager_->saveToJson(filename + "_puzzles.json");
    
    core::Logger::instance().info("Game state saved to " + filename + "_*.json");
}

bool GameplayExample::loadGameState(const std::string& filename)
{
    bool itemsLoaded = itemManager_->loadFromJson(filename + "_items.json");
    bool puzzlesLoaded = puzzleManager_->loadFromJson(filename + "_puzzles.json");
    
    if (itemsLoaded && puzzlesLoaded) {
        core::Logger::instance().info("Game state loaded from " + filename + "_*.json");
        return true;
    } else {
        core::Logger::instance().warn("Failed to load complete game state from " + filename + "_*.json");
        return false;
    }
}

void GameplayExample::printStatistics()
{
    auto itemStats = itemManager_->getStatistics();
    auto puzzleStats = puzzleManager_->getStatistics();
    
    core::Logger::instance().info("=== Gameplay Statistics ===");
    core::Logger::instance().info("Items Added: " + std::to_string(itemStats.totalItemsAdded));
    core::Logger::instance().info("Items Collected: " + std::to_string(itemStats.totalItemsCollected));
    core::Logger::instance().info("Keys: " + std::to_string(itemStats.itemsByType[0]));
    core::Logger::instance().info("Tools: " + std::to_string(itemStats.itemsByType[1]));
    core::Logger::instance().info("Collectibles: " + std::to_string(itemStats.itemsByType[2]));
    core::Logger::instance().info("Average Collection Time: " + std::to_string(itemStats.averageCollectionTime) + "s");
    
    core::Logger::instance().info("Puzzles Added: " + std::to_string(puzzleStats.totalPuzzlesAdded));
    core::Logger::instance().info("Puzzles Completed: " + std::to_string(puzzleStats.totalPuzzlesCompleted));
    core::Logger::instance().info("Average Completion Time: " + std::to_string(puzzleStats.averageCompletionTime) + "s");
    core::Logger::instance().info("===========================");
}

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_GAMEPLAYEXAMPLE_H
