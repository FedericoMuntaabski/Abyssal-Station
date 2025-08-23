#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLEMANAGER_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLEMANAGER_H

#include "Puzzle.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <chrono>
#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
namespace ui { class UIManager; }

namespace gameplay {

class PuzzleManager {
public:
    PuzzleManager();
    ~PuzzleManager();

    // Register a puzzle (takes ownership)
    void registerPuzzle(std::unique_ptr<Puzzle> puzzle);

    // Remove a puzzle by id
    bool removePuzzle(entities::Entity::Id id);

    // Update all puzzles
    void updateAll(float deltaTime);

    // Explicitly check completions and notify logger for newly completed puzzles
    void checkAllCompletion();

    // Render all puzzles
    void renderAll(sf::RenderWindow& window);

    // Accessor
    Puzzle* getPuzzleById(entities::Entity::Id id);
    
        void setUIManager(ui::UIManager* uiManager);

    // Access to puzzles for serialization/integration
    std::vector<Puzzle*> allPuzzles() const;

    // Statistics tracking
    struct Statistics {
        std::size_t totalPuzzlesAdded = 0;
        std::size_t totalPuzzlesCompleted = 0;
        float averageCompletionTime = 0.0f;
    };
    const Statistics& getStatistics() const { return stats_; }

    // Serialization support
    void saveToJson(const std::string& filename) const;
    bool loadFromJson(const std::string& filename);

private:
    std::vector<std::unique_ptr<Puzzle>> puzzles_;
    // Track which puzzles we've already logged as completed
    std::unordered_set<entities::Entity::Id> loggedCompleted_;
        ui::UIManager* uiManager_{nullptr};
    Statistics stats_;
    std::chrono::steady_clock::time_point gameStartTime_;
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_PUZZLEMANAGER_H
