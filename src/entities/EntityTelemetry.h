#ifndef ABYSSAL_STATION_SRC_ENTITIES_ENTITYTELEMETRY_H
#define ABYSSAL_STATION_SRC_ENTITIES_ENTITYTELEMETRY_H

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include "Entity.h"

namespace entities {

/**
 * Telemetry and analytics system for entity events.
 * Tracks entity lifecycle, movement, collisions, and other events.
 */
class EntityTelemetry {
public:
    struct EntityEvent {
        Entity::Id entityId;
        std::string eventType;
        std::string eventData;
        std::chrono::system_clock::time_point timestamp;
    };

    struct EntityStats {
        std::uint32_t creationCount{0};
        std::uint32_t destructionCount{0};
        std::uint32_t movementEvents{0};
        std::uint32_t collisionEvents{0};
        std::uint32_t interactionEvents{0};
        float totalLifetime{0.f}; // Total lifetime of all entities of this type
        float averageLifetime{0.f};
    };

    // Singleton access
    static EntityTelemetry& instance();

    // Event logging
    void logEntityCreated(Entity::Id id, const std::string& entityType, const sf::Vector2f& position);
    void logEntityDestroyed(Entity::Id id, const std::string& entityType, float lifetime);
    void logEntityMoved(Entity::Id id, const sf::Vector2f& from, const sf::Vector2f& to);
    void logEntityCollision(Entity::Id id, Entity::Id otherId, const sf::Vector2f& position, bool wasBlocked);
    void logEntityInteraction(Entity::Id id, const std::string& interactionType, const std::string& details);

    // Analytics
    const std::vector<EntityEvent>& getEvents() const { return events_; }
    const std::unordered_map<std::string, EntityStats>& getStats() const { return stats_; }
    
    // Get stats for specific entity type
    EntityStats getStatsForType(const std::string& entityType) const;
    
    // Performance metrics
    void logPerformanceMetric(const std::string& metric, float value);
    float getAveragePerformanceMetric(const std::string& metric) const;

    // Configuration
    void setMaxEvents(std::size_t maxEvents) { maxEvents_ = maxEvents; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    // Data export
    std::string exportToJson() const;
    void exportToCsv(const std::string& filename) const;

    // Cleanup
    void clearEvents();
    void clearOldEvents(std::chrono::minutes maxAge);

private:
    EntityTelemetry() = default;
    
    std::vector<EntityEvent> events_;
    std::unordered_map<std::string, EntityStats> stats_;
    std::unordered_map<std::string, std::vector<float>> performanceMetrics_;
    std::size_t maxEvents_{10000};
    bool enabled_{true};

    void addEvent(Entity::Id id, const std::string& eventType, const std::string& eventData);
    void updateStats(const std::string& entityType, const std::string& eventType, float value = 0.f);
};

} // namespace entities

#endif // ABYSSAL_STATION_SRC_ENTITIES_ENTITYTELEMETRY_H
