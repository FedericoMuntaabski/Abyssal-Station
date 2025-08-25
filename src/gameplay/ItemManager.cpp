#include "ItemManager.h"
#include "AchievementManager.h"
#include "../collisions/CollisionManager.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include "PuzzleManager.h"
#include <unordered_map>
#include "../ui/UIManager.h"
#include <fstream>
#include <algorithm>

using namespace gameplay;

ItemManager::ItemManager(collisions::CollisionManager* collisionManager, ui::UIManager* uiManager)
    : collisionManager_(collisionManager)
    , uiManager_(uiManager)
    , gameStartTime_(std::chrono::steady_clock::now())
{
}

ItemManager::~ItemManager() = default;

void ItemManager::addItem(std::unique_ptr<Item> item)
{
    if (!item) return;
    // Ensure item's collision manager is set
    if (collisionManager_) item->setCollisionManager(collisionManager_);
    
    // Update statistics
    stats_.totalItemsAdded++;
    stats_.itemsByType[static_cast<int>(item->type())]++;
    
    items_.push_back(std::move(item));
}

bool ItemManager::removeItem(entities::Entity::Id id)
{
    auto it = std::find_if(items_.begin(), items_.end(), [id](const std::unique_ptr<Item>& itp){ return itp && itp->id() == id; });
    if (it == items_.end()) return false;
    items_.erase(it);
    core::Logger::instance().info(std::string("Item removed: id=") + std::to_string(id));
    return true;
}

void ItemManager::updateAll(float deltaTime)
{
    // Update each item
    for (auto& item : items_) {
        if (!item) continue;
        bool wasCollected = item->isCollected();
        item->update(deltaTime);

        // Skip interaction detection for collected or disabled items
        if (item->isCollected() || item->isDisabled()) continue;

        // If we have a collision manager, check for colliders overlapping this item
        if (collisionManager_) {
            // Use the collision manager to find first collider for the item's bounds
            sf::FloatRect bounds(item->position(), item->size());
            // Only consider Player colliders for item pickup
            auto collider = collisionManager_->firstColliderForBounds(bounds, item.get(), entities::kLayerMaskPlayer);
            if (collider) {
                // If collider is a player, trigger interaction
                entities::Player* player = dynamic_cast<entities::Player*>(collider);
                if (player) {
                    item->interact(*player);
                    // Notify UI manager if available and item was not previously collected
                    if (uiManager_ && !wasCollected && item->isCollected()) {
                        uiManager_->notifyItemCollected(item->id());
                        // Update statistics
                        stats_.totalItemsCollected++;
                        auto elapsed = std::chrono::steady_clock::now() - gameStartTime_;
                        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                        stats_.averageCollectionTime = static_cast<float>(seconds) / stats_.totalItemsCollected;
                    }
                        // If bound to a puzzle step, mark the step completed
                        if (puzzleManager_ && !wasCollected && item->isCollected()) {
                            auto mit = itemToPuzzleMap_.find(item->id());
                            if (mit != itemToPuzzleMap_.end()) {
                                auto [puzzleId, stepIdx] = mit->second;
                                if (auto p = puzzleManager_->getPuzzleById(puzzleId)) {
                                    p->markStepCompleted(stepIdx);
                                }
                            }
                        }
                }
            }
        }
    }
}

void ItemManager::renderAll(sf::RenderWindow& window)
{
    for (auto& item : items_) {
        if (!item) continue;
        item->render(window);
    }
}

void ItemManager::setCollisionManager(collisions::CollisionManager* cm)
{
    collisionManager_ = cm;
    for (auto& item : items_) {
        if (item) item->setCollisionManager(cm);
    }
}

void ItemManager::setUIManager(ui::UIManager* uiManager)
{
    uiManager_ = uiManager;
}

void ItemManager::setPuzzleManager(PuzzleManager* pm)
{
    puzzleManager_ = pm;
}

void ItemManager::bindItemToPuzzleStep(entities::Entity::Id itemId, entities::Entity::Id puzzleId, std::size_t stepIndex)
{
    itemToPuzzleMap_[itemId] = std::make_pair(puzzleId, stepIndex);
}

void ItemManager::interactWithItem(entities::Entity::Id itemId, entities::Player* player)
{
    if (!player) return;
    auto it = std::find_if(items_.begin(), items_.end(), [itemId](const std::unique_ptr<Item>& ip){ return ip && ip->id() == itemId; });
    if (it == items_.end()) return;
    Item* item = it->get();
    if (!item || item->isCollected()) return;

    // Trigger item interaction
    item->interact(*player);

    // UI notify
    if (uiManager_ ) uiManager_->notifyItemCollected(item->id());

    // Puzzle binding
    auto mit = itemToPuzzleMap_.find(item->id());
    if (mit != itemToPuzzleMap_.end() && puzzleManager_) {
        auto [pid, step] = mit->second;
        if (auto p = puzzleManager_->getPuzzleById(pid)) p->markStepCompleted(step);
    }
}

std::vector<Item*> ItemManager::allItems() const
{
    std::vector<Item*> out;
    out.reserve(items_.size());
    for (const auto& up : items_) if (up) out.push_back(up.get());
    return out;
}

Item* ItemManager::getItemById(entities::Entity::Id id)
{
    auto it = std::find_if(items_.begin(), items_.end(), 
        [id](const std::unique_ptr<Item>& item) { 
            return item && item->id() == id; 
        });
    return (it != items_.end()) ? it->get() : nullptr;
}

void ItemManager::saveToJson(const std::string& filename) const
{
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            core::Logger::instance().error("Failed to open file for writing: " + filename);
            return;
        }

        file << "{\n";
        file << "  \"version\": 1,\n";
        file << "  \"items\": [\n";
        
        bool first = true;
        for (const auto& item : items_) {
            if (!item) continue;
            if (!first) file << ",\n";
            first = false;
            
            file << "    {\n";
            file << "      \"id\": " << item->id() << ",\n";
            file << "      \"type\": " << static_cast<int>(item->type()) << ",\n";
            file << "      \"position\": [" << item->position().x << ", " << item->position().y << "],\n";
            file << "      \"size\": [" << item->size().x << ", " << item->size().y << "],\n";
            file << "      \"collected\": " << (item->isCollected() ? "true" : "false") << "\n";
            file << "    }";
        }
        
        file << "\n  ],\n";
        file << "  \"statistics\": {\n";
        file << "    \"totalItemsAdded\": " << stats_.totalItemsAdded << ",\n";
        file << "    \"totalItemsCollected\": " << stats_.totalItemsCollected << ",\n";
        file << "    \"itemsByType\": [" << stats_.itemsByType[0] << ", " << stats_.itemsByType[1] << ", " << stats_.itemsByType[2] << "],\n";
        file << "    \"averageCollectionTime\": " << stats_.averageCollectionTime << "\n";
        file << "  }\n";
        file << "}\n";
        
        core::Logger::instance().info("Items saved to: " + filename);
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error saving items: " + std::string(e.what()));
    }
}

bool ItemManager::loadFromJson(const std::string& filename)
{
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            core::Logger::instance().warning("Save file not found: " + filename);
            return false;
        }

        // For now, just log that we would load - full JSON parsing would require nlohmann/json
        core::Logger::instance().info("Loading items from: " + filename);
        
        // Clear existing items first
        items_.clear();
        
        // Reset statistics
        stats_ = Statistics{};
        gameStartTime_ = std::chrono::steady_clock::now();
        
        // TODO: Implement full JSON parsing when nlohmann/json is available
        core::Logger::instance().warning("JSON parsing not yet implemented - would need nlohmann/json library");
        
        return true;
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error loading items: " + std::string(e.what()));
        return false;
    }
}
