#include "EntityTelemetry.h"
#include "../core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <numeric>

namespace entities {

using core::Logger;

EntityTelemetry& EntityTelemetry::instance() {
    static EntityTelemetry instance;
    return instance;
}

void EntityTelemetry::logEntityCreated(Entity::Id id, const std::string& entityType, const sf::Vector2f& position) {
    if (!enabled_) return;
    
    std::string eventData = "position:" + std::to_string(position.x) + "," + std::to_string(position.y);
    addEvent(id, "created", eventData);
    updateStats(entityType, "created");
    
    Logger::instance().info("[EntityTelemetry] Entity created: type=" + entityType + " id=" + std::to_string(id));
}

void EntityTelemetry::logEntityDestroyed(Entity::Id id, const std::string& entityType, float lifetime) {
    if (!enabled_) return;
    
    std::string eventData = "lifetime:" + std::to_string(lifetime);
    addEvent(id, "destroyed", eventData);
    updateStats(entityType, "destroyed", lifetime);
    
    Logger::instance().info("[EntityTelemetry] Entity destroyed: type=" + entityType + " id=" + std::to_string(id) + " lifetime=" + std::to_string(lifetime));
}

void EntityTelemetry::logEntityMoved(Entity::Id id, const sf::Vector2f& from, const sf::Vector2f& to) {
    if (!enabled_) return;
    
    float distance = std::sqrt((to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y));
    std::string eventData = "from:" + std::to_string(from.x) + "," + std::to_string(from.y) + 
                           " to:" + std::to_string(to.x) + "," + std::to_string(to.y) + 
                           " distance:" + std::to_string(distance);
    addEvent(id, "moved", eventData);
    
    // Only log significant movements to avoid spam
    if (distance > 1.0f) {
        Logger::instance().info("[EntityTelemetry] Entity moved: id=" + std::to_string(id) + " distance=" + std::to_string(distance));
    }
}

void EntityTelemetry::logEntityCollision(Entity::Id id, Entity::Id otherId, const sf::Vector2f& position, bool wasBlocked) {
    if (!enabled_) return;
    
    std::string eventData = "other:" + std::to_string(otherId) + 
                           " position:" + std::to_string(position.x) + "," + std::to_string(position.y) + 
                           " blocked:" + (wasBlocked ? "true" : "false");
    addEvent(id, "collision", eventData);
    
    Logger::instance().info("[EntityTelemetry] Entity collision: id=" + std::to_string(id) + " with=" + std::to_string(otherId) + " blocked=" + (wasBlocked ? "true" : "false"));
}

void EntityTelemetry::logEntityInteraction(Entity::Id id, const std::string& interactionType, const std::string& details) {
    if (!enabled_) return;
    
    std::string eventData = "type:" + interactionType + " details:" + details;
    addEvent(id, "interaction", eventData);
    
    Logger::instance().info("[EntityTelemetry] Entity interaction: id=" + std::to_string(id) + " type=" + interactionType);
}

EntityTelemetry::EntityStats EntityTelemetry::getStatsForType(const std::string& entityType) const {
    auto it = stats_.find(entityType);
    return (it != stats_.end()) ? it->second : EntityStats{};
}

void EntityTelemetry::logPerformanceMetric(const std::string& metric, float value) {
    if (!enabled_) return;
    
    performanceMetrics_[metric].push_back(value);
    
    // Keep only recent measurements to prevent memory growth
    auto& values = performanceMetrics_[metric];
    if (values.size() > 1000) {
        values.erase(values.begin(), values.begin() + 500);
    }
}

float EntityTelemetry::getAveragePerformanceMetric(const std::string& metric) const {
    auto it = performanceMetrics_.find(metric);
    if (it == performanceMetrics_.end() || it->second.empty()) {
        return 0.f;
    }
    
    const auto& values = it->second;
    float sum = std::accumulate(values.begin(), values.end(), 0.f);
    return sum / static_cast<float>(values.size());
}

std::string EntityTelemetry::exportToJson() const {
    nlohmann::json j;
    
    // Export events
    j["events"] = nlohmann::json::array();
    for (const auto& event : events_) {
        nlohmann::json eventJson;
        eventJson["entityId"] = event.entityId;
        eventJson["eventType"] = event.eventType;
        eventJson["eventData"] = event.eventData;
        eventJson["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            event.timestamp.time_since_epoch()).count();
        j["events"].push_back(eventJson);
    }
    
    // Export stats
    j["stats"] = nlohmann::json::object();
    for (const auto& [type, stats] : stats_) {
        nlohmann::json statsJson;
        statsJson["creationCount"] = stats.creationCount;
        statsJson["destructionCount"] = stats.destructionCount;
        statsJson["movementEvents"] = stats.movementEvents;
        statsJson["collisionEvents"] = stats.collisionEvents;
        statsJson["interactionEvents"] = stats.interactionEvents;
        statsJson["totalLifetime"] = stats.totalLifetime;
        statsJson["averageLifetime"] = stats.averageLifetime;
        j["stats"][type] = statsJson;
    }
    
    return j.dump(2);
}

void EntityTelemetry::exportToCsv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        Logger::instance().error("[EntityTelemetry] Failed to open file for CSV export: " + filename);
        return;
    }
    
    // Write CSV header
    file << "Timestamp,EntityId,EventType,EventData\n";
    
    // Write events
    for (const auto& event : events_) {
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            event.timestamp.time_since_epoch()).count();
        file << timestamp << "," << event.entityId << "," << event.eventType << "," << event.eventData << "\n";
    }
    
    file.close();
    Logger::instance().info("[EntityTelemetry] Exported " + std::to_string(events_.size()) + " events to " + filename);
}

void EntityTelemetry::clearEvents() {
    events_.clear();
    Logger::instance().info("[EntityTelemetry] Cleared all events");
}

void EntityTelemetry::clearOldEvents(std::chrono::minutes maxAge) {
    auto cutoffTime = std::chrono::system_clock::now() - maxAge;
    auto newEnd = std::remove_if(events_.begin(), events_.end(),
        [cutoffTime](const EntityEvent& event) {
            return event.timestamp < cutoffTime;
        });
    
    std::size_t removedCount = std::distance(newEnd, events_.end());
    events_.erase(newEnd, events_.end());
    
    Logger::instance().info("[EntityTelemetry] Removed " + std::to_string(removedCount) + " old events");
}

void EntityTelemetry::addEvent(Entity::Id id, const std::string& eventType, const std::string& eventData) {
    if (events_.size() >= maxEvents_) {
        // Remove oldest events to make room
        events_.erase(events_.begin(), events_.begin() + maxEvents_ / 4);
    }
    
    EntityEvent event;
    event.entityId = id;
    event.eventType = eventType;
    event.eventData = eventData;
    event.timestamp = std::chrono::system_clock::now();
    
    events_.push_back(event);
}

void EntityTelemetry::updateStats(const std::string& entityType, const std::string& eventType, float value) {
    auto& stats = stats_[entityType];
    
    if (eventType == "created") {
        stats.creationCount++;
    } else if (eventType == "destroyed") {
        stats.destructionCount++;
        stats.totalLifetime += value;
        if (stats.destructionCount > 0) {
            stats.averageLifetime = stats.totalLifetime / static_cast<float>(stats.destructionCount);
        }
    } else if (eventType == "moved") {
        stats.movementEvents++;
    } else if (eventType == "collision") {
        stats.collisionEvents++;
    } else if (eventType == "interaction") {
        stats.interactionEvents++;
    }
}

} // namespace entities
