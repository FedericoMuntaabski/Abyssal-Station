#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_ITEMMANAGER_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_ITEMMANAGER_H

#include "Item.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstddef>
#include <chrono>
#include <string>

namespace collisions { class CollisionManager; }
namespace ui { class UIManager; }
namespace gameplay { class AchievementManager; }

namespace gameplay {

class ItemManager {
public:
    explicit ItemManager(collisions::CollisionManager* collisionManager = nullptr, ui::UIManager* uiManager = nullptr);
    ~ItemManager();

    // Add an existing unique_ptr<Item> to the manager
    void addItem(std::unique_ptr<Item> item);

    // Remove item by id. Returns true if removed.
    bool removeItem(entities::Entity::Id id);

    // Update all items and detect interactions via CollisionManager
    void updateAll(float deltaTime);

    // Render all active items
    void renderAll(sf::RenderWindow& window);

    void setCollisionManager(collisions::CollisionManager* cm);
    void setUIManager(ui::UIManager* uiManager);
    void setPuzzleManager(class PuzzleManager* pm);
    void setAchievementManager(class AchievementManager* am);

    // Bind an item id to a puzzle id and step index so picking the item completes that step
    void bindItemToPuzzleStep(entities::Entity::Id itemId, entities::Entity::Id puzzleId, std::size_t stepIndex);

    // Directly trigger interaction with an item by id (used by player 'Interact' action)
    void interactWithItem(entities::Entity::Id itemId, entities::Player* player);

    // Access to items for serialization/integration (non-owning raw pointers)
    std::vector<Item*> allItems() const;

    // Search by ID for quick access
    Item* getItemById(entities::Entity::Id id);

    // Statistics tracking
    struct Statistics {
        std::size_t totalItemsAdded = 0;
        std::size_t totalItemsCollected = 0;
        std::size_t itemsByType[3] = {0, 0, 0}; // Key, Tool, Collectible
        float averageCollectionTime = 0.0f;
    };
    const Statistics& getStatistics() const { return stats_; }

    // Serialization support
    void saveToJson(const std::string& filename) const;
    bool loadFromJson(const std::string& filename);

private:
    collisions::CollisionManager* collisionManager_;
    std::vector<std::unique_ptr<Item>> items_;
    ui::UIManager* uiManager_{nullptr};
    class PuzzleManager* puzzleManager_{nullptr};
    class AchievementManager* achievementManager_{nullptr};
    std::unordered_map<entities::Entity::Id, std::pair<entities::Entity::Id, std::size_t>> itemToPuzzleMap_;
    Statistics stats_;
    std::chrono::steady_clock::time_point gameStartTime_;
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_ITEMMANAGER_H
