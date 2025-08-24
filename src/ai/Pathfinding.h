#ifndef ABYSSAL_STATION_SRC_AI_PATHFINDING_H
#define ABYSSAL_STATION_SRC_AI_PATHFINDING_H

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>

namespace collisions { class CollisionManager; }
namespace entities { class Entity; }

namespace ai {

// A* pathfinding node
struct PathNode {
    sf::Vector2f position;
    float gCost;        // Distance from start
    float hCost;        // Heuristic distance to goal
    float fCost() const { return gCost + hCost; }
    PathNode* parent;
    
    PathNode(const sf::Vector2f& pos, float g = 0.0f, float h = 0.0f, PathNode* p = nullptr)
        : position(pos), gCost(g), hCost(h), parent(p) {}
};

// Pathfinding configuration
struct PathfindingConfig {
    float gridSize = 32.0f;          // Size of each pathfinding grid cell
    float maxPathLength = 1000.0f;   // Maximum path length to prevent infinite searches
    int maxIterations = 1000;        // Maximum A* iterations
    bool allowDiagonal = true;       // Allow diagonal movement
    float diagonalCost = 1.414f;     // Cost multiplier for diagonal moves
    std::uint32_t obstacleLayerMask = 0xFFFFFFFF; // What layers are considered obstacles
};

// Result of a pathfinding operation
struct PathfindingResult {
    std::vector<sf::Vector2f> path;
    bool success;
    float totalCost;
    int iterations;
    
    PathfindingResult() : success(false), totalCost(0.0f), iterations(0) {}
};

// A* pathfinding system
class PathfindingSystem {
public:
    explicit PathfindingSystem(const PathfindingConfig& config = PathfindingConfig{});
    
    // Find path from start to goal, avoiding obstacles
    PathfindingResult findPath(
        const sf::Vector2f& start,
        const sf::Vector2f& goal,
        collisions::CollisionManager* collisionManager,
        entities::Entity* pathEntity = nullptr
    );
    
    // Simplified pathfinding for basic movement (direct line with obstacle avoidance)
    std::vector<sf::Vector2f> findSimplePath(
        const sf::Vector2f& start,
        const sf::Vector2f& goal,
        collisions::CollisionManager* collisionManager,
        entities::Entity* pathEntity = nullptr
    );
    
    // Check if a straight line path is clear
    bool isPathClear(
        const sf::Vector2f& start,
        const sf::Vector2f& goal,
        collisions::CollisionManager* collisionManager,
        entities::Entity* excludeEntity = nullptr
    ) const;
    
    // Smooth a path to reduce unnecessary waypoints
    std::vector<sf::Vector2f> smoothPath(
        const std::vector<sf::Vector2f>& path,
        collisions::CollisionManager* collisionManager,
        entities::Entity* pathEntity = nullptr
    ) const;
    
    // Configuration
    void setConfig(const PathfindingConfig& config) { config_ = config; }
    const PathfindingConfig& getConfig() const { return config_; }
    
    // Grid utilities
    sf::Vector2f worldToGrid(const sf::Vector2f& worldPos) const;
    sf::Vector2f gridToWorld(const sf::Vector2f& gridPos) const;
    
private:
    PathfindingConfig config_;
    
    // Hash function for Vector2f to use in unordered_map
    struct Vector2fHash {
        std::size_t operator()(const sf::Vector2f& v) const {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
        }
    };
    
    struct Vector2fEqual {
        bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
            return std::abs(a.x - b.x) < 0.01f && std::abs(a.y - b.y) < 0.01f;
        }
    };
    
    // A* algorithm helpers
    float heuristic(const sf::Vector2f& a, const sf::Vector2f& b) const;
    std::vector<sf::Vector2f> getNeighbors(const sf::Vector2f& position) const;
    bool isWalkable(const sf::Vector2f& position, collisions::CollisionManager* cm, entities::Entity* entity) const;
    std::vector<sf::Vector2f> reconstructPath(PathNode* goalNode) const;
    
    // Node comparison for priority queue
    struct NodeCompare {
        bool operator()(const PathNode* a, const PathNode* b) const {
            return a->fCost() > b->fCost();
        }
    };
};

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_PATHFINDING_H
