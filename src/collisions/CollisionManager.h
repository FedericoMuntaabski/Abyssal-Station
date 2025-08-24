#ifndef ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H
#define ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H

#include "CollisionBox.h"
#include "CollisionEvents.h"
#include "SpatialPartition.h"
#include <vector>
#include <memory>
#include <chrono>

namespace entities { class Entity; }

namespace collisions {

// Collision detection result with additional information
struct CollisionResult {
    entities::Entity* entityA;
    entities::Entity* entityB;
    sf::FloatRect intersection;
    sf::Vector2f normal; // Collision normal (from A to B)
    bool isTrigger{false}; // True if either shape is a trigger
};

// Raycast result with hit information
struct RaycastHit {
    entities::Entity* entity{nullptr};
    sf::Vector2f point{0.f, 0.f}; // Hit point
    sf::Vector2f normal{0.f, 0.f}; // Surface normal at hit point
    float distance{0.f}; // Distance from ray origin
    bool valid{false};
};

class CollisionManager {
public:
    enum class SpatialPartitionType {
        None,     // Brute force O(n²)
        QuadTree, // Hierarchical spatial partitioning
        SpatialHash // Grid-based spatial partitioning
    };

    struct Config {
        SpatialPartitionType spatialPartition = SpatialPartitionType::QuadTree;
        QuadTree::Config quadTreeConfig;
        SpatialHash::Config spatialHashConfig;
        bool enableProfiling = false;
    };

    explicit CollisionManager(const Config& config = {});
    ~CollisionManager() = default;

    // Configuration
    void setConfig(const Config& config);
    const Config& getConfig() const { return config_; }

    // Register or update a collider for an entity (uses owner's collisionLayer())
    void addCollider(entities::Entity* owner, const sf::FloatRect& bounds);

    // Update collider bounds for an entity if it exists, otherwise add it
    void updateColliderBounds(entities::Entity* owner, const sf::FloatRect& bounds);

    // Remove collider for an entity
    void removeCollider(entities::Entity* owner);

    // Advanced multi-shape collider support
    void addMultiShapeCollider(entities::Entity* owner, std::vector<std::unique_ptr<CollisionShape>> shapes);
    void updateMultiShapeCollider(entities::Entity* owner);

    // Returns list of entities that are colliding with the given entity (excluding itself)
    std::vector<entities::Entity*> checkCollisions(entities::Entity* owner) const;

    // Enhanced collision detection with detailed results
    std::vector<CollisionResult> checkCollisionsDetailed(entities::Entity* owner) const;

    // Return first entity that would collide with the provided bounds (exclude an optional owner)
    // If allowedLayers != 0, only colliders whose layer bit intersects allowedLayers are considered
    entities::Entity* firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Enhanced raycast with hit information
    RaycastHit raycast(const sf::Vector2f& origin, const sf::Vector2f& direction, float maxDistance = 1000.f, 
                      entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Returns true if a line segment from p0 to p1 intersects any collider (optionally excluding an entity)
    // and restricted to allowedLayers. Useful to implement line-of-sight checks that should be blocked by walls.
    bool segmentIntersectsAny(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Enhanced segment intersection with hit information
    RaycastHit segmentIntersection(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Sweep test for predictive collision detection
    std::vector<CollisionResult> sweepTest(const sf::FloatRect& bounds, const sf::Vector2f& velocity, float deltaTime, 
                                          entities::Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const;

    // Event system access
    CollisionEventManager& getEventManager() { return eventManager_; }
    const CollisionEventManager& getEventManager() const { return eventManager_; }

    // Layer filtering utilities
    bool layerMaskIntersects(std::uint32_t layerA, std::uint32_t allowedLayers) const;
    void setLayerCollisionMatrix(std::uint32_t layerA, std::uint32_t layerB, bool canCollide);
    bool getLayerCollisionMatrix(std::uint32_t layerA, std::uint32_t layerB) const;

    // Debug and profiling
    struct ProfileData {
        std::chrono::microseconds totalTime{0};
        std::chrono::microseconds broadPhaseTime{0};
        std::chrono::microseconds narrowPhaseTime{0};
        int totalQueries = 0;
        int broadPhaseTests = 0;
        int narrowPhaseTests = 0;
    };
    
    const ProfileData& getProfileData() const { return profileData_; }
    void resetProfileData();

    // Spatial partition statistics
    std::string getSpatialPartitionStats() const;

    // Update spatial partitioning (call this when many entities have moved)
    void rebuildSpatialPartition();

private:
    Config config_;
    std::vector<CollisionBox> colliders_;
    std::unique_ptr<SpatialPartition> spatialPartition_;
    CollisionEventManager eventManager_;
    
    // Layer collision matrix (which layers can collide with which)
    std::unordered_map<std::uint64_t, bool> layerCollisionMatrix_;
    
    // Profiling data
    mutable ProfileData profileData_;
    
    void initializeSpatialPartition();
    void updateSpatialPartition();
    
    // Helper methods
    CollisionBox* findCollider(entities::Entity* owner);
    const CollisionBox* findCollider(entities::Entity* owner) const;
    
    // Collision detection helpers
    bool testCollision(const sf::FloatRect& a, const sf::FloatRect& b, CollisionResult& result) const;
    sf::Vector2f calculateCollisionNormal(const sf::FloatRect& a, const sf::FloatRect& b) const;
    
    // Raycast helpers
    bool rayIntersectsRect(const sf::Vector2f& origin, const sf::Vector2f& direction, const sf::FloatRect& rect, RaycastHit& hit) const;
    bool segmentIntersectsRect(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::FloatRect& rect, RaycastHit& hit) const;
    
    // Layer matrix helpers
    std::uint64_t makeLayerPairKey(std::uint32_t layerA, std::uint32_t layerB) const;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_COLLISIONMANAGER_H
