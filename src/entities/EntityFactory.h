#ifndef ABYSSAL_STATION_SRC_ENTITIES_ENTITYFACTORY_H
#define ABYSSAL_STATION_SRC_ENTITIES_ENTITYFACTORY_H

#include "Entity.h"
#include "Player.h"
#include "Wall.h"
#include "../ai/Enemy.h"
#include "../gameplay/Item.h"
#include "../ai/AIState.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

namespace entities {

/**
 * Factory Pattern implementation for Entity creation.
 * Provides centralized, configurable entity creation with:
 * - Type registration system
 * - JSON-based configuration
 * - Default parameter management
 * - Extensible creation callbacks
 */
class EntityFactory {
public:
    // Type alias for creation functions
    using EntityCreator = std::function<std::unique_ptr<Entity>(Entity::Id, const nlohmann::json&)>;
    
    // Singleton access
    static EntityFactory& getInstance();
    
    // Entity creation methods
    std::unique_ptr<Entity> createEntity(const std::string& type, Entity::Id id, const nlohmann::json& config = {});
    std::unique_ptr<Player> createPlayer(Entity::Id id, const nlohmann::json& config = {});
    std::unique_ptr<ai::Enemy> createEnemy(Entity::Id id, const nlohmann::json& config = {});
    std::unique_ptr<gameplay::Item> createItem(Entity::Id id, const nlohmann::json& config = {});
    std::unique_ptr<Entity> createWall(Entity::Id id, const nlohmann::json& config = {});
    
    // Registration system for extensibility
    void registerEntityType(const std::string& type, EntityCreator creator);
    bool isTypeRegistered(const std::string& type) const;
    std::vector<std::string> getRegisteredTypes() const;
    
    // Configuration management
    void setDefaultConfig(const std::string& type, const nlohmann::json& config);
    nlohmann::json getDefaultConfig(const std::string& type) const;
    void loadDefaultsFromFile(const std::string& configPath);
    void saveDefaultsToFile(const std::string& configPath) const;
    
    // Utility methods
    void clearRegistrations();
    size_t getRegistrationCount() const;

private:
    EntityFactory();
    ~EntityFactory() = default;
    
    // Prevent copying
    EntityFactory(const EntityFactory&) = delete;
    EntityFactory& operator=(const EntityFactory&) = delete;
    
    // Default creators
    void registerDefaultTypes();
    static std::unique_ptr<Entity> createPlayerInternal(Entity::Id id, const nlohmann::json& config);
    static std::unique_ptr<Entity> createEnemyInternal(Entity::Id id, const nlohmann::json& config);
    static std::unique_ptr<Entity> createItemInternal(Entity::Id id, const nlohmann::json& config);
    static std::unique_ptr<Entity> createWallInternal(Entity::Id id, const nlohmann::json& config);
    
    // Helper methods
    nlohmann::json mergeConfigs(const nlohmann::json& defaults, const nlohmann::json& overrides) const;
    sf::Vector2f parseVector2f(const nlohmann::json& json, const sf::Vector2f& defaultValue = {0.f, 0.f}) const;
    sf::Color parseColor(const nlohmann::json& json, const sf::Color& defaultValue = sf::Color::White) const;
    
    // Storage
    std::unordered_map<std::string, EntityCreator> m_creators;
    std::unordered_map<std::string, nlohmann::json> m_defaultConfigs;
};

// Convenience macros for registration
#define REGISTER_ENTITY_TYPE(className, typeName) \
    EntityFactory::getInstance().registerEntityType(typeName, \
        [](Entity::Id id, const nlohmann::json& config) -> std::unique_ptr<Entity> { \
            return std::make_unique<className>(id, config); \
        })

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITYFACTORY_H
