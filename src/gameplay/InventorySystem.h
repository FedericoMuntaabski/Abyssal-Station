#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_INVENTORYSYSTEM_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_INVENTORYSYSTEM_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "../core/Logger.h"

namespace gameplay {

/**
 * @brief Represents an inventory item with weight and properties
 */
struct InventoryItem {
    enum class Type {
        Tool,           // Herramientas (tubería, cuchillo)
        Key,            // Llaves
        Collectible,    // Objetos coleccionables
        Weapon,         // Armas improvisadas
        Consumable      // Consumibles (futuro)
    };
    
    size_t id{0};
    std::string name;
    Type type;
    float weight{0.0f};         // Peso en kg
    int durability{100};        // Durabilidad (0-100)
    int damage{0};              // Daño base (para armas)
    bool isStackable{false};    // Si se puede apilar
    int stackCount{1};          // Cantidad en stack
    
    InventoryItem() = default;
    InventoryItem(size_t itemId, const std::string& itemName, Type itemType, float itemWeight, int itemDurability = 100);
    
    // Factory methods for common items
    static std::unique_ptr<InventoryItem> createPipe();
    static std::unique_ptr<InventoryItem> createKnife();
    static std::unique_ptr<InventoryItem> createKey(const std::string& keyName);
};

/**
 * @brief Slot-based inventory system with weight restrictions
 * Implements the 4-slot hotbar system with overflow to backpack
 */
class InventorySystem {
public:
    static constexpr size_t HOTBAR_SLOTS = 4;
    static constexpr size_t MAX_BACKPACK_SLOTS = 16;
    static constexpr float MAX_WEIGHT = 20.0f;
    
    InventorySystem();
    ~InventorySystem() = default;
    
    // Core inventory operations
    bool addItem(std::unique_ptr<InventoryItem> item, int slot = -1);
    std::unique_ptr<InventoryItem> removeItem(size_t slot);
    std::unique_ptr<InventoryItem> removeItemById(size_t itemId);
    
    // Slot management
    bool swapSlots(size_t slot1, size_t slot2);
    bool moveToHotbar(size_t backpackSlot, size_t hotbarSlot);
    bool moveToBackpack(size_t hotbarSlot);
    
    // Weight and capacity queries
    float getCurrentWeight() const;
    float getRemainingWeight() const;
    bool canCarryWeight(float additionalWeight) const;
    bool isOverloaded() const;
    
    // Slot queries
    size_t getAvailableSlots() const;
    size_t getUsedSlots() const;
    bool isSlotEmpty(size_t slot) const;
    bool isHotbarSlot(size_t slot) const;
    
    // Item access
    InventoryItem* getItem(size_t slot) const;
    InventoryItem* getItemById(size_t itemId) const;
    InventoryItem* getHotbarItem(size_t hotbarSlot) const;
    
    // Equipment and usage
    InventoryItem* getCurrentWeapon() const;
    bool selectHotbarSlot(size_t slot);
    size_t getCurrentHotbarSlot() const { return currentHotbarSlot_; }
    
    // Item usage and durability
    bool useItem(size_t slot);
    void damageItem(size_t slot, int damage);
    void removeDestroyedItems();
    
    // Serialization support
    void serialize(std::vector<uint8_t>& buffer) const;
    bool deserialize(const std::vector<uint8_t>& buffer);
    
    // Debug and inspection
    void printInventory() const;
    std::vector<std::pair<size_t, const InventoryItem*>> getAllItems() const;
    
private:
    std::vector<std::unique_ptr<InventoryItem>> hotbar_;       // 4 slots principales
    std::vector<std::unique_ptr<InventoryItem>> backpack_;     // Mochila expandible
    size_t currentHotbarSlot_{0};                              // Slot activo del hotbar
    size_t nextItemId_{1};                                     // ID único para items
    
    // Internal helpers
    bool hasSpaceForItem(const InventoryItem& item) const;
    size_t findEmptySlot() const;
    size_t findEmptyHotbarSlot() const;
    size_t findEmptyBackpackSlot() const;
    void consolidateStacks();
    
    // Weight calculation
    float calculateTotalWeight() const;
};

/**
 * @brief Weapon-specific functionality for combat items
 */
class WeaponManager {
public:
    enum class WeaponType {
        Improvised,     // Armas improvisadas (tubería, etc.)
        Blade,          // Armas cortantes (cuchillo)
        Blunt,          // Armas contundentes
        Ranged          // Armas a distancia (futuro)
    };
    
    struct WeaponStats {
        int baseDamage{0};
        float attackSpeed{1.5f};        // Seconds between attacks
        float range{32.0f};             // Attack range in pixels
        int durabilityLoss{5};          // Durability lost per attack
        WeaponType type{WeaponType::Improvised};
    };
    
    static WeaponStats getWeaponStats(const InventoryItem& item);
    static bool isWeapon(const InventoryItem& item);
    static int calculateDamage(const InventoryItem& weapon, float criticalChance = 0.0f);
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_INVENTORYSYSTEM_H
