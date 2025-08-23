#include "../gameplay/ItemTests.h"
#include "../gameplay/PuzzleTests.h"
#include "../../src/core/Logger.h"
#include <iostream>

// Simple test runner without external dependencies
void runBasicTests() {
    std::cout << "Running Basic Gameplay Tests...\n";
    
    // Test Item creation
    try {
        auto item = std::make_unique<gameplay::Item>(1, sf::Vector2f(100, 100), sf::Vector2f(16, 16), gameplay::ItemType::Key);
        assert(item->id() == 1);
        assert(item->position().x == 100.0f);
        assert(item->type() == gameplay::ItemType::Key);
        assert(!item->isCollected());
        std::cout << "✓ Item creation test passed\n";
    } catch (...) {
        std::cout << "❌ Item creation test failed\n";
        throw;
    }
    
    // Test Item disable/enable
    try {
        auto item = std::make_unique<gameplay::Item>(2, sf::Vector2f(200, 200), sf::Vector2f(16, 16), gameplay::ItemType::Tool);
        assert(!item->isDisabled());
        item->setDisabled(true);
        assert(item->isDisabled());
        item->setDisabled(false);
        assert(!item->isDisabled());
        std::cout << "✓ Item disable/enable test passed\n";
    } catch (...) {
        std::cout << "❌ Item disable/enable test failed\n";
        throw;
    }
    
    // Test ItemManager basic operations
    try {
        gameplay::ItemManager manager; // No dependencies
        
        auto item1 = std::make_unique<gameplay::Item>(1, sf::Vector2f(100, 100), sf::Vector2f(16, 16), gameplay::ItemType::Key);
        auto item2 = std::make_unique<gameplay::Item>(2, sf::Vector2f(200, 200), sf::Vector2f(16, 16), gameplay::ItemType::Tool);
        
        manager.addItem(std::move(item1));
        manager.addItem(std::move(item2));
        
        auto allItems = manager.allItems();
        assert(allItems.size() == 2);
        
        // Test search
        auto* found = manager.getItemById(1);
        assert(found != nullptr);
        assert(found->id() == 1);
        
        auto* notFound = manager.getItemById(999);
        assert(notFound == nullptr);
        
        // Test removal
        assert(manager.removeItem(1));
        assert(!manager.removeItem(999));
        
        allItems = manager.allItems();
        assert(allItems.size() == 1);
        
        std::cout << "✓ ItemManager operations test passed\n";
    } catch (...) {
        std::cout << "❌ ItemManager operations test failed\n";
        throw;
    }
    
    // Test Puzzle creation
    try {
        std::vector<std::string> steps = {"Step1", "Step2"};
        auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
        
        assert(puzzle->id() == 1);
        assert(puzzle->state() == gameplay::PuzzleState::Locked);
        
        // Test step completion
        assert(!puzzle->checkCompletion());
        puzzle->markStepCompleted(0);
        assert(!puzzle->checkCompletion()); // Still need step 1
        puzzle->markStepCompleted(1);
        assert(puzzle->checkCompletion()); // Now complete
        
        std::cout << "✓ Puzzle creation and completion test passed\n";
    } catch (...) {
        std::cout << "❌ Puzzle creation and completion test failed\n";
        throw;
    }
    
    // Test PuzzleManager basic operations
    try {
        gameplay::PuzzleManager manager;
        
        std::vector<std::string> steps1 = {"Step1"};
        std::vector<std::string> steps2 = {"Step1", "Step2"};
        
        auto puzzle1 = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps1);
        auto puzzle2 = std::make_unique<gameplay::Puzzle>(2, sf::Vector2f(200, 200), sf::Vector2f(64, 64), steps2);
        
        manager.registerPuzzle(std::move(puzzle1));
        manager.registerPuzzle(std::move(puzzle2));
        
        auto allPuzzles = manager.allPuzzles();
        assert(allPuzzles.size() == 2);
        
        // Test search
        auto* found = manager.getPuzzleById(1);
        assert(found != nullptr);
        assert(found->id() == 1);
        
        auto* notFound = manager.getPuzzleById(999);
        assert(notFound == nullptr);
        
        // Test removal
        assert(manager.removePuzzle(1));
        assert(!manager.removePuzzle(999));
        
        allPuzzles = manager.allPuzzles();
        assert(allPuzzles.size() == 1);
        
        std::cout << "✓ PuzzleManager operations test passed\n";
    } catch (...) {
        std::cout << "❌ PuzzleManager operations test failed\n";
        throw;
    }
    
    std::cout << "🎉 All basic gameplay tests completed successfully!\n";
}

int main()
{
    try {
        runBasicTests();
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}
