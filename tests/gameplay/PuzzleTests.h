#ifndef ABYSSAL_STATION_TESTS_GAMEPLAY_PUZZLETESTS_H
#define ABYSSAL_STATION_TESTS_GAMEPLAY_PUZZLETESTS_H

#include "../../src/gameplay/Puzzle.h"
#include "../../src/gameplay/PuzzleManager.h"
#include <memory>
#include <cassert>
#include <vector>
#include <string>

namespace tests {

class PuzzleTests {
public:
    static void runAll();

private:
    static void testPuzzleCreation();
    static void testPuzzleStepCompletion();
    static void testPuzzleCompletion();
    static void testPuzzleStateTransitions();
    static void testPuzzleManagerBasics();
    static void testPuzzleManagerSearch();
    static void testPuzzleManagerStatistics();
    static void testInvalidStepIndex();
};

// Test implementation
void PuzzleTests::runAll()
{
    testPuzzleCreation();
    testPuzzleStepCompletion();
    testPuzzleCompletion();
    testPuzzleStateTransitions();
    testPuzzleManagerBasics();
    testPuzzleManagerSearch();
    testPuzzleManagerStatistics();
    testInvalidStepIndex();
}

void PuzzleTests::testPuzzleCreation()
{
    std::vector<std::string> steps = {"Collect key", "Open door", "Press button"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    
    assert(puzzle->id() == 1);
    assert(puzzle->position().x == 100.0f);
    assert(puzzle->position().y == 100.0f);
    assert(puzzle->state() == gameplay::PuzzleState::Locked);
    
    // Empty puzzle should be completed
    auto emptyPuzzle = std::make_unique<gameplay::Puzzle>(2, sf::Vector2f(200, 200));
    assert(emptyPuzzle->state() == gameplay::PuzzleState::Completed);
}

void PuzzleTests::testPuzzleStepCompletion()
{
    std::vector<std::string> steps = {"Step1", "Step2", "Step3"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    
    // Complete first step
    bool wasCompleted = puzzle->markStepCompleted(0);
    assert(!wasCompleted); // Puzzle not yet complete
    
    // Complete second step
    wasCompleted = puzzle->markStepCompleted(1);
    assert(!wasCompleted); // Still not complete
    
    // Complete third step
    wasCompleted = puzzle->markStepCompleted(2);
    assert(wasCompleted); // Now complete
}

void PuzzleTests::testPuzzleCompletion()
{
    std::vector<std::string> steps = {"Step1", "Step2"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    
    assert(!puzzle->checkCompletion());
    assert(puzzle->state() == gameplay::PuzzleState::Locked);
    
    puzzle->markStepCompleted(0);
    assert(!puzzle->checkCompletion());
    
    puzzle->markStepCompleted(1);
    assert(puzzle->checkCompletion());
}

void PuzzleTests::testPuzzleStateTransitions()
{
    std::vector<std::string> steps = {"Step1"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    
    assert(puzzle->state() == gameplay::PuzzleState::Locked);
    
    // Update should not change state without player interaction
    puzzle->update(0.016f);
    assert(puzzle->state() == gameplay::PuzzleState::Locked);
    
    // Complete the step
    puzzle->markStepCompleted(0);
    puzzle->update(0.016f);
    // State should remain Locked until player interaction activates it
    assert(puzzle->state() == gameplay::PuzzleState::Locked);
}

void PuzzleTests::testPuzzleManagerBasics()
{
    gameplay::PuzzleManager manager;
    
    std::vector<std::string> steps1 = {"Step1"};
    std::vector<std::string> steps2 = {"Step1", "Step2"};
    
    auto puzzle1 = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps1);
    auto puzzle2 = std::make_unique<gameplay::Puzzle>(2, sf::Vector2f(200, 200), sf::Vector2f(64, 64), steps2);
    
    manager.registerPuzzle(std::move(puzzle1));
    manager.registerPuzzle(std::move(puzzle2));
    
    auto allPuzzles = manager.allPuzzles();
    assert(allPuzzles.size() == 2);
    
    assert(manager.removePuzzle(1));
    assert(!manager.removePuzzle(999)); // Non-existent puzzle
    
    allPuzzles = manager.allPuzzles();
    assert(allPuzzles.size() == 1);
    assert(allPuzzles[0]->id() == 2);
}

void PuzzleTests::testPuzzleManagerSearch()
{
    gameplay::PuzzleManager manager;
    
    std::vector<std::string> steps = {"Step1"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(42, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    manager.registerPuzzle(std::move(puzzle));
    
    auto* found = manager.getPuzzleById(42);
    assert(found != nullptr);
    assert(found->id() == 42);
    
    auto* notFound = manager.getPuzzleById(999);
    assert(notFound == nullptr);
}

void PuzzleTests::testPuzzleManagerStatistics()
{
    gameplay::PuzzleManager manager;
    
    auto stats = manager.getStatistics();
    assert(stats.totalPuzzlesAdded == 0);
    assert(stats.totalPuzzlesCompleted == 0);
    
    std::vector<std::string> steps1 = {"Step1"};
    std::vector<std::string> steps2 = {"Step1", "Step2"};
    
    auto puzzle1 = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps1);
    auto puzzle2 = std::make_unique<gameplay::Puzzle>(2, sf::Vector2f(200, 200), sf::Vector2f(64, 64), steps2);
    
    manager.registerPuzzle(std::move(puzzle1));
    manager.registerPuzzle(std::move(puzzle2));
    
    stats = manager.getStatistics();
    assert(stats.totalPuzzlesAdded == 2);
    assert(stats.totalPuzzlesCompleted == 0);
}

void PuzzleTests::testInvalidStepIndex()
{
    std::vector<std::string> steps = {"Step1", "Step2"};
    auto puzzle = std::make_unique<gameplay::Puzzle>(1, sf::Vector2f(100, 100), sf::Vector2f(64, 64), steps);
    
    // Test invalid step index
    bool result = puzzle->markStepCompleted(999);
    assert(!result);
    
    // Test duplicate completion
    puzzle->markStepCompleted(0);
    bool duplicate = puzzle->markStepCompleted(0);
    assert(!duplicate);
}

} // namespace tests

#endif // ABYSSAL_STATION_TESTS_GAMEPLAY_PUZZLETESTS_H
