#include "PuzzleManager.h"
#include "../core/Logger.h"
#include "../ui/UIManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <fstream>

using namespace gameplay;

PuzzleManager::PuzzleManager() 
    : gameStartTime_(std::chrono::steady_clock::now())
{
}
PuzzleManager::~PuzzleManager() = default;

void PuzzleManager::registerPuzzle(std::unique_ptr<Puzzle> puzzle)
{
    if (!puzzle) return;
    stats_.totalPuzzlesAdded++;
    puzzles_.push_back(std::move(puzzle));
}

bool PuzzleManager::removePuzzle(entities::Entity::Id id)
{
    auto it = std::find_if(puzzles_.begin(), puzzles_.end(), [id](const std::unique_ptr<Puzzle>& p){ return p && p->id() == id; });
    if (it == puzzles_.end()) return false;
    puzzles_.erase(it);
    loggedCompleted_.erase(id);
    core::Logger::instance().info(std::string("Puzzle removed: id=") + std::to_string(id));
    return true;
}

void PuzzleManager::updateAll(float deltaTime)
{
    for (auto& p : puzzles_) {
        if (!p) continue;
        p->update(deltaTime);
    }
    checkAllCompletion();
}

void PuzzleManager::checkAllCompletion()
{
    for (auto& p : puzzles_) {
        if (!p) continue;
        if (p->state() == PuzzleState::Completed) {
            if (loggedCompleted_.find(p->id()) == loggedCompleted_.end()) {
                loggedCompleted_.insert(p->id());
                stats_.totalPuzzlesCompleted++;
                auto elapsed = std::chrono::steady_clock::now() - gameStartTime_;
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                stats_.averageCompletionTime = static_cast<float>(seconds) / stats_.totalPuzzlesCompleted;
                
                core::Logger::instance().info(std::string("PuzzleManager: puzzle completed id=") + std::to_string(p->id()));
                if (uiManager_) uiManager_->notifyPuzzleCompleted(p->id());
            }
        }
    }
}

void PuzzleManager::setUIManager(ui::UIManager* uiManager)
{
    uiManager_ = uiManager;
}

std::vector<Puzzle*> PuzzleManager::allPuzzles() const
{
    std::vector<Puzzle*> out;
    out.reserve(puzzles_.size());
    for (const auto& up : puzzles_) if (up) out.push_back(up.get());
    return out;
}

Puzzle* PuzzleManager::getPuzzleById(entities::Entity::Id id)
{
    auto it = std::find_if(puzzles_.begin(), puzzles_.end(), [id](const std::unique_ptr<Puzzle>& p){ return p && p->id() == id; });
    return (it != puzzles_.end()) ? it->get() : nullptr;
}

void PuzzleManager::renderAll(sf::RenderWindow& window)
{
    for (auto& p : puzzles_) {
        if (p) p->render(window);
    }
}

void PuzzleManager::saveToJson(const std::string& filename) const
{
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            core::Logger::instance().error("Failed to open file for writing: " + filename);
            return;
        }

        file << "{\n";
        file << "  \"version\": 1,\n";
        file << "  \"puzzles\": [\n";
        
        bool first = true;
        for (const auto& puzzle : puzzles_) {
            if (!puzzle) continue;
            if (!first) file << ",\n";
            first = false;
            
            file << "    {\n";
            file << "      \"id\": " << puzzle->id() << ",\n";
            file << "      \"position\": [" << puzzle->position().x << ", " << puzzle->position().y << "],\n";
            file << "      \"size\": [" << puzzle->size().x << ", " << puzzle->size().y << "],\n";
            file << "      \"state\": " << static_cast<int>(puzzle->state()) << ",\n";
            file << "      \"completedSteps\": [";
            
            // Note: We would need access to completedSteps_ to serialize properly
            // For now, just indicate if completed based on state
            file << (puzzle->state() == PuzzleState::Completed ? "true" : "false");
            
            file << "]\n";
            file << "    }";
        }
        
        file << "\n  ],\n";
        file << "  \"statistics\": {\n";
        file << "    \"totalPuzzlesAdded\": " << stats_.totalPuzzlesAdded << ",\n";
        file << "    \"totalPuzzlesCompleted\": " << stats_.totalPuzzlesCompleted << ",\n";
        file << "    \"averageCompletionTime\": " << stats_.averageCompletionTime << "\n";
        file << "  }\n";
        file << "}\n";
        
        core::Logger::instance().info("Puzzles saved to: " + filename);
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error saving puzzles: " + std::string(e.what()));
    }
}

bool PuzzleManager::loadFromJson(const std::string& filename)
{
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            core::Logger::instance().warning("Save file not found: " + filename);
            return false;
        }

        core::Logger::instance().info("Loading puzzles from: " + filename);
        
        // Clear existing puzzles first
        puzzles_.clear();
        loggedCompleted_.clear();
        
        // Reset statistics
        stats_ = Statistics{};
        gameStartTime_ = std::chrono::steady_clock::now();
        
        // TODO: Implement full JSON parsing when nlohmann/json is available
        core::Logger::instance().warning("JSON parsing not yet implemented - would need nlohmann/json library");
        
        return true;
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error loading puzzles: " + std::string(e.what()));
        return false;
    }
}
