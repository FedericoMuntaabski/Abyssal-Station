#include "ItemTests.h"
#include "PuzzleTests.h"
#include "../../src/core/Logger.h"
#include <iostream>

int main()
{
    std::cout << "Running Gameplay Module Tests...\n\n";
    
    try {
        std::cout << "Running Item Tests...\n";
        tests::ItemTests::runAll();
        std::cout << "✓ All Item tests passed!\n\n";
        
        std::cout << "Running Puzzle Tests...\n";
        tests::PuzzleTests::runAll();
        std::cout << "✓ All Puzzle tests passed!\n\n";
        
        std::cout << "🎉 All Gameplay tests completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}
