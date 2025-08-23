#include "ItemManager.h"
#include "../collisions/CollisionManager.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include "PuzzleManager.h"
#include <unordered_map>
#include "../ui/UIManager.h"

using namespace gameplay;

ItemManager::ItemManager(collisions::CollisionManager* collisionManager, ui::UIManager* uiManager)
    : collisionManager_(collisionManager)
    , uiManager_(uiManager)
{
}

ItemManager::~ItemManager() = default;

void ItemManager::addItem(std::unique_ptr<Item> item)
{
    if (!item) return;
    // Ensure item's collision manager is set
    if (collisionManager_) item->setCollisionManager(collisionManager_);
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

        // If we have a collision manager, check for colliders overlapping this item
        if (collisionManager_ && !item->isCollected()) {
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
