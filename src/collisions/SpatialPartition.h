#ifndef ABYSSAL_STATION_SRC_COLLISIONS_SPATIALPARTITION_H
#define ABYSSAL_STATION_SRC_COLLISIONS_SPATIALPARTITION_H

#include "CollisionBox.h"
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace entities { class Entity; }

namespace collisions {

// Abstract base class for spatial partitioning systems
class SpatialPartition {
public:
    virtual ~SpatialPartition() = default;
    
    virtual void clear() = 0;
    virtual void insert(const CollisionBox& collider) = 0;
    virtual void remove(entities::Entity* entity) = 0;
    
    // Query for potential collisions with a given bounds
    virtual std::vector<const CollisionBox*> query(const sf::FloatRect& bounds) const = 0;
    
    // Query for potential collisions along a line segment
    virtual std::vector<const CollisionBox*> querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const = 0;
};

// QuadTree implementation for spatial partitioning
class QuadTree : public SpatialPartition {
public:
    struct Config {
        int maxDepth = 6;
        int maxObjectsPerNode = 10;
        // Use SFML 3.x constructor syntax
        Config() : bounds({0.f, 0.f}, {2048.f, 2048.f}) {}
        sf::FloatRect bounds;
    };

    explicit QuadTree(const Config& config = Config{});
    ~QuadTree() override = default;

    void clear() override;
    void insert(const CollisionBox& collider) override;
    void remove(entities::Entity* entity) override;
    
    std::vector<const CollisionBox*> query(const sf::FloatRect& bounds) const override;
    std::vector<const CollisionBox*> querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const override;

    // Statistics for debugging/optimization
    struct Stats {
        int totalNodes = 0;
        int leafNodes = 0;
        int totalObjects = 0;
        int maxDepthReached = 0;
    };
    Stats getStats() const;

private:
    struct Node {
        sf::FloatRect bounds;
        std::vector<const CollisionBox*> objects;
        std::unique_ptr<Node> children[4]; // NW, NE, SW, SE
        int depth;
        
        Node(const sf::FloatRect& b, int d) : bounds(b), depth(d) {}
        
        bool isLeaf() const { return !children[0]; }
        void subdivide();
        int getQuadrant(const sf::FloatRect& objBounds) const;
        sf::FloatRect getQuadrantBounds(int quadrant) const;
    };

    Config config_;
    std::unique_ptr<Node> root_;
    
    void insertIntoNode(Node* node, const CollisionBox* collider);
    void queryNode(const Node* node, const sf::FloatRect& bounds, std::vector<const CollisionBox*>& result) const;
    void querySegmentNode(const Node* node, const sf::Vector2f& p0, const sf::Vector2f& p1, std::vector<const CollisionBox*>& result) const;
    void getStatsFromNode(const Node* node, Stats& stats) const;
    
    // Helper: check if segment intersects rectangle
    bool segmentIntersectsRect(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::FloatRect& rect) const;
};

// Spatial Hash implementation (alternative to QuadTree)
class SpatialHash : public SpatialPartition {
public:
    struct Config {
        float cellSize = 64.0f;
        // Use SFML 3.x constructor syntax
        Config() : bounds({0.f, 0.f}, {2048.f, 2048.f}) {}
        sf::FloatRect bounds;
    };

    explicit SpatialHash(const Config& config = Config{});
    ~SpatialHash() override = default;

    void clear() override;
    void insert(const CollisionBox& collider) override;
    void remove(entities::Entity* entity) override;
    
    std::vector<const CollisionBox*> query(const sf::FloatRect& bounds) const override;
    std::vector<const CollisionBox*> querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const override;

private:
    Config config_;
    std::unordered_map<int64_t, std::vector<const CollisionBox*>> cells_;
    
    // Hash a 2D cell coordinate to a single integer
    int64_t hashCell(int x, int y) const;
    
    // Get cell coordinates for a point
    std::pair<int, int> getCellCoords(float x, float y) const;
    
    // Get all cells that a rectangle overlaps
    std::vector<std::pair<int, int>> getCellsForRect(const sf::FloatRect& rect) const;
    
    // Get all cells that a line segment passes through
    std::vector<std::pair<int, int>> getCellsForSegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const;
};

} // namespace collisions

#endif // ABYSSAL_STATION_SRC_COLLISIONS_SPATIALPARTITION_H
