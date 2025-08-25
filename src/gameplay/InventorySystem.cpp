#include "InventorySystem.h"
#include "../core/Logger.h"
#include <algorithm>
#include <sstream>

namespace gameplay {

// InventoryItem implementation
InventoryItem::InventoryItem(size_t itemId, const std::string& itemName, Type itemType, float itemWeight, int itemDurability)
    : id(itemId), name(itemName), type(itemType), weight(itemWeight), durability(itemDurability) {
}

std::unique_ptr<InventoryItem> InventoryItem::createPipe() {
    auto item = std::make_unique<InventoryItem>(0, "Tubería", Type::Weapon, 3.5f, 80);
    item->damage = 15;
    return item;
}

std::unique_ptr<InventoryItem> InventoryItem::createKnife() {
    auto item = std::make_unique<InventoryItem>(0, "Cuchillo", Type::Weapon, 0.5f, 60);
    item->damage = 25;
    return item;
}

std::unique_ptr<InventoryItem> InventoryItem::createKey(const std::string& keyName) {
    return std::make_unique<InventoryItem>(0, keyName, Type::Key, 0.1f, 100);
}

// InventorySystem implementation
InventorySystem::InventorySystem() {
    hotbar_.resize(HOTBAR_SLOTS);
    backpack_.resize(MAX_BACKPACK_SLOTS);
    
    core::Logger::instance().info("[InventorySystem] Initialized with " + 
        std::to_string(HOTBAR_SLOTS) + " hotbar slots and " + 
        std::to_string(MAX_BACKPACK_SLOTS) + " backpack slots");
}

bool InventorySystem::addItem(std::unique_ptr<InventoryItem> item, int slot) {
    if (!item) return false;
    
    // Check weight constraints
    if (!canCarryWeight(item->weight)) {
        core::Logger::instance().info("[InventorySystem] Cannot add item: would exceed weight limit");
        return false;
    }
    
    // Assign unique ID if not set
    if (item->id == 0) {
        item->id = nextItemId_++;
    }
    
    // Try to add to specific slot if requested
    if (slot >= 0) {
        if (slot < static_cast<int>(HOTBAR_SLOTS)) {
            if (!hotbar_[slot]) {
                hotbar_[slot] = std::move(item);
                core::Logger::instance().info("[InventorySystem] Added item to hotbar slot " + std::to_string(slot));
                return true;
            }
        } else {
            size_t backpackIndex = slot - HOTBAR_SLOTS;
            if (backpackIndex < backpack_.size() && !backpack_[backpackIndex]) {
                backpack_[backpackIndex] = std::move(item);
                core::Logger::instance().info("[InventorySystem] Added item to backpack slot " + std::to_string(backpackIndex));
                return true;
            }
        }
        return false; // Requested slot was occupied
    }
    
    // Try to add to hotbar first
    for (size_t i = 0; i < hotbar_.size(); ++i) {
        if (!hotbar_[i]) {
            hotbar_[i] = std::move(item);
            core::Logger::instance().info("[InventorySystem] Added item to hotbar slot " + std::to_string(i));
            return true;
        }
    }
    
    // If hotbar is full, try backpack
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (!backpack_[i]) {
            backpack_[i] = std::move(item);
            core::Logger::instance().info("[InventorySystem] Added item to backpack slot " + std::to_string(i));
            return true;
        }
    }
    
    core::Logger::instance().info("[InventorySystem] Could not add item: no free slots");
    return false; // No free slots
}

std::unique_ptr<InventoryItem> InventorySystem::removeItem(size_t slot) {
    if (slot < HOTBAR_SLOTS) {
        auto item = std::move(hotbar_[slot]);
        if (item) {
            core::Logger::instance().info("[InventorySystem] Removed item from hotbar slot " + std::to_string(slot));
        }
        return item;
    } else {
        size_t backpackIndex = slot - HOTBAR_SLOTS;
        if (backpackIndex < backpack_.size()) {
            auto item = std::move(backpack_[backpackIndex]);
            if (item) {
                core::Logger::instance().info("[InventorySystem] Removed item from backpack slot " + std::to_string(backpackIndex));
            }
            return item;
        }
    }
    return nullptr;
}

std::unique_ptr<InventoryItem> InventorySystem::removeItemById(size_t itemId) {
    // Search in hotbar
    for (size_t i = 0; i < hotbar_.size(); ++i) {
        if (hotbar_[i] && hotbar_[i]->id == itemId) {
            return removeItem(i);
        }
    }
    
    // Search in backpack
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (backpack_[i] && backpack_[i]->id == itemId) {
            return removeItem(i + HOTBAR_SLOTS);
        }
    }
    
    return nullptr;
}

bool InventorySystem::swapSlots(size_t slot1, size_t slot2) {
    auto item1 = removeItem(slot1);
    auto item2 = removeItem(slot2);
    
    bool success = true;
    if (item1) success &= addItem(std::move(item1), slot2);
    if (item2) success &= addItem(std::move(item2), slot1);
    
    return success;
}

bool InventorySystem::moveToHotbar(size_t backpackSlot, size_t hotbarSlot) {
    if (hotbarSlot >= HOTBAR_SLOTS) return false;
    
    size_t backpackIndex = backpackSlot;
    if (backpackIndex >= backpack_.size() || !backpack_[backpackIndex]) return false;
    
    if (hotbar_[hotbarSlot]) return false; // Hotbar slot occupied
    
    hotbar_[hotbarSlot] = std::move(backpack_[backpackIndex]);
    core::Logger::instance().info("[InventorySystem] Moved item from backpack to hotbar slot " + std::to_string(hotbarSlot));
    return true;
}

bool InventorySystem::moveToBackpack(size_t hotbarSlot) {
    if (hotbarSlot >= HOTBAR_SLOTS || !hotbar_[hotbarSlot]) return false;
    
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (!backpack_[i]) {
            backpack_[i] = std::move(hotbar_[hotbarSlot]);
            core::Logger::instance().info("[InventorySystem] Moved item from hotbar to backpack slot " + std::to_string(i));
            return true;
        }
    }
    
    return false; // No free backpack slots
}

float InventorySystem::getCurrentWeight() const {
    return calculateTotalWeight();
}

float InventorySystem::getRemainingWeight() const {
    return MAX_WEIGHT - getCurrentWeight();
}

bool InventorySystem::canCarryWeight(float additionalWeight) const {
    return (getCurrentWeight() + additionalWeight) <= MAX_WEIGHT;
}

bool InventorySystem::isOverloaded() const {
    return getCurrentWeight() > MAX_WEIGHT * 0.8f; // 80% threshold
}

size_t InventorySystem::getAvailableSlots() const {
    size_t available = 0;
    
    for (const auto& item : hotbar_) {
        if (!item) available++;
    }
    
    for (const auto& item : backpack_) {
        if (!item) available++;
    }
    
    return available;
}

size_t InventorySystem::getUsedSlots() const {
    return (HOTBAR_SLOTS + MAX_BACKPACK_SLOTS) - getAvailableSlots();
}

bool InventorySystem::isSlotEmpty(size_t slot) const {
    if (slot < HOTBAR_SLOTS) {
        return !hotbar_[slot];
    } else {
        size_t backpackIndex = slot - HOTBAR_SLOTS;
        return backpackIndex < backpack_.size() && !backpack_[backpackIndex];
    }
}

bool InventorySystem::isHotbarSlot(size_t slot) const {
    return slot < HOTBAR_SLOTS;
}

InventoryItem* InventorySystem::getItem(size_t slot) const {
    if (slot < HOTBAR_SLOTS) {
        return hotbar_[slot].get();
    } else {
        size_t backpackIndex = slot - HOTBAR_SLOTS;
        if (backpackIndex < backpack_.size()) {
            return backpack_[backpackIndex].get();
        }
    }
    return nullptr;
}

InventoryItem* InventorySystem::getItemById(size_t itemId) const {
    for (const auto& item : hotbar_) {
        if (item && item->id == itemId) {
            return item.get();
        }
    }
    
    for (const auto& item : backpack_) {
        if (item && item->id == itemId) {
            return item.get();
        }
    }
    
    return nullptr;
}

InventoryItem* InventorySystem::getHotbarItem(size_t hotbarSlot) const {
    if (hotbarSlot < hotbar_.size()) {
        return hotbar_[hotbarSlot].get();
    }
    return nullptr;
}

InventoryItem* InventorySystem::getCurrentWeapon() const {
    auto item = getHotbarItem(currentHotbarSlot_);
    if (item && WeaponManager::isWeapon(*item)) {
        return item;
    }
    return nullptr;
}

bool InventorySystem::selectHotbarSlot(size_t slot) {
    if (slot < HOTBAR_SLOTS) {
        currentHotbarSlot_ = slot;
        core::Logger::instance().info("[InventorySystem] Selected hotbar slot " + std::to_string(slot));
        return true;
    }
    return false;
}

bool InventorySystem::useItem(size_t slot) {
    auto item = getItem(slot);
    if (!item) return false;
    
    switch (item->type) {
        case InventoryItem::Type::Weapon:
            // Weapons don't get consumed on use, only damaged
            damageItem(slot, 5);
            break;
        case InventoryItem::Type::Consumable:
            // Remove consumable after use
            removeItem(slot);
            break;
        default:
            // Other items don't get consumed
            break;
    }
    
    core::Logger::instance().info("[InventorySystem] Used item: " + item->name);
    return true;
}

void InventorySystem::damageItem(size_t slot, int damage) {
    auto item = getItem(slot);
    if (!item) return;
    
    item->durability = std::max(0, item->durability - damage);
    
    if (item->durability <= 0) {
        core::Logger::instance().info("[InventorySystem] Item destroyed: " + item->name);
        removeItem(slot);
    }
}

void InventorySystem::removeDestroyedItems() {
    for (size_t i = 0; i < hotbar_.size(); ++i) {
        if (hotbar_[i] && hotbar_[i]->durability <= 0) {
            core::Logger::instance().info("[InventorySystem] Removing destroyed item from hotbar: " + hotbar_[i]->name);
            hotbar_[i].reset();
        }
    }
    
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (backpack_[i] && backpack_[i]->durability <= 0) {
            core::Logger::instance().info("[InventorySystem] Removing destroyed item from backpack: " + backpack_[i]->name);
            backpack_[i].reset();
        }
    }
}

void InventorySystem::printInventory() const {
    std::stringstream ss;
    ss << "[InventorySystem] Current inventory:\n";
    ss << "Weight: " << getCurrentWeight() << "/" << MAX_WEIGHT << " kg\n";
    ss << "Hotbar:\n";
    
    for (size_t i = 0; i < hotbar_.size(); ++i) {
        ss << "  [" << i << "] ";
        if (hotbar_[i]) {
            ss << hotbar_[i]->name << " (" << hotbar_[i]->weight << "kg, " << hotbar_[i]->durability << "%)";
        } else {
            ss << "Empty";
        }
        if (i == currentHotbarSlot_) ss << " *SELECTED*";
        ss << "\n";
    }
    
    ss << "Backpack:\n";
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (backpack_[i]) {
            ss << "  [" << (i + HOTBAR_SLOTS) << "] " << backpack_[i]->name 
               << " (" << backpack_[i]->weight << "kg, " << backpack_[i]->durability << "%)\n";
        }
    }
    
    core::Logger::instance().info(ss.str());
}

std::vector<std::pair<size_t, const InventoryItem*>> InventorySystem::getAllItems() const {
    std::vector<std::pair<size_t, const InventoryItem*>> result;
    
    for (size_t i = 0; i < hotbar_.size(); ++i) {
        if (hotbar_[i]) {
            result.emplace_back(i, hotbar_[i].get());
        }
    }
    
    for (size_t i = 0; i < backpack_.size(); ++i) {
        if (backpack_[i]) {
            result.emplace_back(i + HOTBAR_SLOTS, backpack_[i].get());
        }
    }
    
    return result;
}

float InventorySystem::calculateTotalWeight() const {
    float total = 0.0f;
    
    for (const auto& item : hotbar_) {
        if (item) {
            total += item->weight * item->stackCount;
        }
    }
    
    for (const auto& item : backpack_) {
        if (item) {
            total += item->weight * item->stackCount;
        }
    }
    
    return total;
}

// WeaponManager implementation
WeaponManager::WeaponStats WeaponManager::getWeaponStats(const InventoryItem& item) {
    WeaponStats stats;
    
    if (item.name == "Tubería") {
        stats.baseDamage = 15;
        stats.attackSpeed = 1.8f;
        stats.range = 40.0f;
        stats.durabilityLoss = 8;
        stats.type = WeaponType::Blunt;
    } else if (item.name == "Cuchillo") {
        stats.baseDamage = 25;
        stats.attackSpeed = 1.2f;
        stats.range = 28.0f;
        stats.durabilityLoss = 3;
        stats.type = WeaponType::Blade;
    } else {
        // Default improvised weapon
        stats.baseDamage = item.damage;
        stats.attackSpeed = 1.5f;
        stats.range = 32.0f;
        stats.durabilityLoss = 5;
        stats.type = WeaponType::Improvised;
    }
    
    return stats;
}

bool WeaponManager::isWeapon(const InventoryItem& item) {
    return item.type == InventoryItem::Type::Weapon || item.damage > 0;
}

int WeaponManager::calculateDamage(const InventoryItem& weapon, float criticalChance) {
    auto stats = getWeaponStats(weapon);
    int damage = stats.baseDamage;
    
    // Apply durability modifier
    float durabilityFactor = weapon.durability / 100.0f;
    damage = static_cast<int>(damage * (0.5f + 0.5f * durabilityFactor));
    
    // Apply critical hit
    if (criticalChance > 0.0f && (rand() % 100) < (criticalChance * 100)) {
        damage = static_cast<int>(damage * 1.5f);
        core::Logger::instance().info("[WeaponManager] Critical hit! Damage: " + std::to_string(damage));
    }
    
    return std::max(1, damage);
}

} // namespace gameplay
