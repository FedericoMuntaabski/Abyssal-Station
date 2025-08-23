#include "PuzzleManager.h"
#include "../core/Logger.h"
#include "../ui/UIManager.h"
#include <SFML/Graphics/RenderWindow.hpp>

using namespace gameplay;

PuzzleManager::PuzzleManager() = default;
PuzzleManager::~PuzzleManager() = default;

void PuzzleManager::registerPuzzle(std::unique_ptr<Puzzle> puzzle)
{
    if (!puzzle) return;
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
