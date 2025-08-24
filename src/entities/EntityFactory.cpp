#include "EntityFactory.h"
#include "../core/Logger.h"
#include "Wall.h"
#include <fstream>
#include <stdexcept>

namespace entities {

EntityFactory& EntityFactory::getInstance() {
    static EntityFactory instance;
    return instance;
}

EntityFactory::EntityFactory() {
    registerDefaultTypes();
    core::Logger::instance().info("EntityFactory: Initialized with default entity types");
}

std::unique_ptr<Entity> EntityFactory::createEntity(const std::string& type, Entity::Id id, const nlohmann::json& config) {
    auto it = m_creators.find(type);
    if (it == m_creators.end()) {
        core::Logger::instance().error("EntityFactory: Unknown entity type '" + type + "'");
        return nullptr;
    }
    
    // Merge default config with provided config
    nlohmann::json finalConfig = config;
    auto defaultIt = m_defaultConfigs.find(type);
    if (defaultIt != m_defaultConfigs.end()) {
        finalConfig = mergeConfigs(defaultIt->second, config);
    }
    
    try {
        auto entity = it->second(id, finalConfig);
        if (entity) {
            core::Logger::instance().info("EntityFactory: Created " + type + " entity with id=" + std::to_string(id));
        }
        return entity;
    } catch (const std::exception& e) {
        core::Logger::instance().error("EntityFactory: Failed to create " + type + " entity: " + e.what());
        return nullptr;
    }
}

std::unique_ptr<Player> EntityFactory::createPlayer(Entity::Id id, const nlohmann::json& config) {
    auto entity = createEntity("Player", id, config);
    return std::unique_ptr<Player>(dynamic_cast<Player*>(entity.release()));
}

std::unique_ptr<ai::Enemy> EntityFactory::createEnemy(Entity::Id id, const nlohmann::json& config) {
    auto entity = createEntity("Enemy", id, config);
    return std::unique_ptr<ai::Enemy>(dynamic_cast<ai::Enemy*>(entity.release()));
}

std::unique_ptr<gameplay::Item> EntityFactory::createItem(Entity::Id id, const nlohmann::json& config) {
    auto entity = createEntity("Item", id, config);
    return std::unique_ptr<gameplay::Item>(dynamic_cast<gameplay::Item*>(entity.release()));
}

std::unique_ptr<Entity> EntityFactory::createWall(Entity::Id id, const nlohmann::json& config) {
    return createEntity("Wall", id, config);
}

void EntityFactory::registerEntityType(const std::string& type, EntityCreator creator) {
    m_creators[type] = creator;
    core::Logger::instance().info("EntityFactory: Registered type '" + type + "'");
}

bool EntityFactory::isTypeRegistered(const std::string& type) const {
    return m_creators.find(type) != m_creators.end();
}

std::vector<std::string> EntityFactory::getRegisteredTypes() const {
    std::vector<std::string> types;
    types.reserve(m_creators.size());
    for (const auto& pair : m_creators) {
        types.push_back(pair.first);
    }
    return types;
}

void EntityFactory::setDefaultConfig(const std::string& type, const nlohmann::json& config) {
    m_defaultConfigs[type] = config;
    core::Logger::instance().info("EntityFactory: Set default config for type '" + type + "'");
}

nlohmann::json EntityFactory::getDefaultConfig(const std::string& type) const {
    auto it = m_defaultConfigs.find(type);
    if (it != m_defaultConfigs.end()) {
        return it->second;
    }
    return nlohmann::json::object();
}

void EntityFactory::loadDefaultsFromFile(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            core::Logger::instance().warning("EntityFactory: Could not open config file: " + configPath);
            return;
        }
        
        nlohmann::json config;
        file >> config;
        
        if (config.contains("entityDefaults") && config["entityDefaults"].is_object()) {
            for (const auto& [type, typeConfig] : config["entityDefaults"].items()) {
                m_defaultConfigs[type] = typeConfig;
            }
            core::Logger::instance().info("EntityFactory: Loaded defaults from " + configPath);
        }
    } catch (const std::exception& e) {
        core::Logger::instance().error("EntityFactory: Failed to load config from " + configPath + ": " + e.what());
    }
}

void EntityFactory::saveDefaultsToFile(const std::string& configPath) const {
    try {
        nlohmann::json config;
        config["entityDefaults"] = m_defaultConfigs;
        
        std::ofstream file(configPath);
        if (!file.is_open()) {
            core::Logger::instance().error("EntityFactory: Could not open config file for writing: " + configPath);
            return;
        }
        
        file << config.dump(2);
        core::Logger::instance().info("EntityFactory: Saved defaults to " + configPath);
    } catch (const std::exception& e) {
        core::Logger::instance().error("EntityFactory: Failed to save config to " + configPath + ": " + e.what());
    }
}

void EntityFactory::clearRegistrations() {
    m_creators.clear();
    m_defaultConfigs.clear();
    core::Logger::instance().info("EntityFactory: Cleared all registrations");
}

size_t EntityFactory::getRegistrationCount() const {
    return m_creators.size();
}

void EntityFactory::registerDefaultTypes() {
    // Register Player
    registerEntityType("Player", createPlayerInternal);
    setDefaultConfig("Player", {
        {"position", {100.0f, 100.0f}},
        {"size", {32.0f, 32.0f}},
        {"speed", 200.0f},
        {"health", 100}
    });
    
    // Register Enemy
    registerEntityType("Enemy", createEnemyInternal);
    setDefaultConfig("Enemy", {
        {"position", {200.0f, 200.0f}},
        {"size", {32.0f, 32.0f}},
        {"speed", 100.0f},
        {"visionRange", 200.0f},
        {"attackRange", 24.0f},
        {"behaviorProfile", "NEUTRAL"},
        {"patrolPoints", nlohmann::json::array()}
    });
    
    // Register Item
    registerEntityType("Item", createItemInternal);
    setDefaultConfig("Item", {
        {"position", {50.0f, 50.0f}},
        {"size", {16.0f, 16.0f}},
        {"itemType", "Collectible"}
    });
    
    // Register Wall
    registerEntityType("Wall", createWallInternal);
    setDefaultConfig("Wall", {
        {"position", {0.0f, 0.0f}},
        {"size", {32.0f, 32.0f}}
    });
}

std::unique_ptr<Entity> EntityFactory::createPlayerInternal(Entity::Id id, const nlohmann::json& config) {
    sf::Vector2f position = EntityFactory::getInstance().parseVector2f(config.value("position", nlohmann::json{100.0f, 100.0f}));
    sf::Vector2f size = EntityFactory::getInstance().parseVector2f(config.value("size", nlohmann::json{32.0f, 32.0f}));
    float speed = config.value("speed", 200.0f);
    int health = config.value("health", 100);
    
    return std::make_unique<Player>(id, position, size, speed, health);
}

std::unique_ptr<Entity> EntityFactory::createEnemyInternal(Entity::Id id, const nlohmann::json& config) {
    sf::Vector2f position = EntityFactory::getInstance().parseVector2f(config.value("position", nlohmann::json{200.0f, 200.0f}));
    sf::Vector2f size = EntityFactory::getInstance().parseVector2f(config.value("size", nlohmann::json{32.0f, 32.0f}));
    float speed = config.value("speed", 100.0f);
    float visionRange = config.value("visionRange", 200.0f);
    float attackRange = config.value("attackRange", 24.0f);
    
    // Parse behavior profile
    ai::BehaviorProfile profile = ai::BehaviorProfile::NEUTRAL;
    std::string profileStr = config.value("behaviorProfile", "NEUTRAL");
    if (profileStr == "AGGRESSIVE") profile = ai::BehaviorProfile::AGGRESSIVE;
    else if (profileStr == "DEFENSIVE") profile = ai::BehaviorProfile::DEFENSIVE;
    else if (profileStr == "PASSIVE") profile = ai::BehaviorProfile::PASSIVE;
    else if (profileStr == "GUARD") profile = ai::BehaviorProfile::GUARD;
    else if (profileStr == "SCOUT") profile = ai::BehaviorProfile::SCOUT;
    
    // Parse patrol points
    std::vector<sf::Vector2f> patrolPoints;
    if (config.contains("patrolPoints") && config["patrolPoints"].is_array()) {
        for (const auto& point : config["patrolPoints"]) {
            patrolPoints.push_back(EntityFactory::getInstance().parseVector2f(point));
        }
    }
    
    return std::make_unique<ai::Enemy>(id, position, size, speed, visionRange, attackRange, patrolPoints, profile);
}

std::unique_ptr<Entity> EntityFactory::createItemInternal(Entity::Id id, const nlohmann::json& config) {
    sf::Vector2f position = EntityFactory::getInstance().parseVector2f(config.value("position", nlohmann::json{50.0f, 50.0f}));
    sf::Vector2f size = EntityFactory::getInstance().parseVector2f(config.value("size", nlohmann::json{16.0f, 16.0f}));
    
    // Parse item type
    gameplay::ItemType itemType = gameplay::ItemType::Collectible;
    std::string typeStr = config.value("itemType", "HEALTH_POTION");
    if (typeStr == "KEY" || typeStr == "Key") itemType = gameplay::ItemType::Key;
    else if (typeStr == "TOOL" || typeStr == "Tool") itemType = gameplay::ItemType::Tool;
    else if (typeStr == "COLLECTIBLE" || typeStr == "Collectible") itemType = gameplay::ItemType::Collectible;
    // Note: COIN, WEAPON, etc. will map to Collectible for now
    
    // Create without CollisionManager - it will be set later by the caller
    return std::make_unique<gameplay::Item>(id, position, size, itemType, nullptr);
}

std::unique_ptr<Entity> EntityFactory::createWallInternal(Entity::Id id, const nlohmann::json& config) {
    sf::Vector2f position = EntityFactory::getInstance().parseVector2f(config.value("position", nlohmann::json{0.0f, 0.0f}));
    sf::Vector2f size = EntityFactory::getInstance().parseVector2f(config.value("size", nlohmann::json{32.0f, 32.0f}));
    // Note: Wall class doesn't support setColor - using default color
    
    return std::make_unique<Wall>(id, position, size);
}

nlohmann::json EntityFactory::mergeConfigs(const nlohmann::json& defaults, const nlohmann::json& overrides) const {
    nlohmann::json result = defaults;
    for (const auto& [key, value] : overrides.items()) {
        result[key] = value;
    }
    return result;
}

sf::Vector2f EntityFactory::parseVector2f(const nlohmann::json& json, const sf::Vector2f& defaultValue) const {
    if (json.is_array() && json.size() >= 2) {
        return sf::Vector2f(json[0].get<float>(), json[1].get<float>());
    }
    return defaultValue;
}

sf::Color EntityFactory::parseColor(const nlohmann::json& json, const sf::Color& defaultValue) const {
    if (json.is_array() && json.size() >= 3) {
        uint8_t r = json[0].get<uint8_t>();
        uint8_t g = json[1].get<uint8_t>();
        uint8_t b = json[2].get<uint8_t>();
        uint8_t a = json.size() >= 4 ? json[3].get<uint8_t>() : 255;
        return sf::Color(r, g, b, a);
    }
    return defaultValue;
}

} // namespace entities
