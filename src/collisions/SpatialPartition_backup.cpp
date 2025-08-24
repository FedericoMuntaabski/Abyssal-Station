#include "SpatialPartition.h"
#include "../entities/Entity.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

namespace collisions {

// int QuadTree::Node::getQuadrant(const sf::FloatRect& objBounds) const {
    float centerX = bounds.position.x + bounds.size.x * 0.5f;
    float centerY = bounds.position.y + bounds.size.y * 0.5f;
    
    bool north = objBounds.position.y + objBounds.size.y <= centerY;
    bool south = objBounds.position.y >= centerY;
    bool west = objBounds.position.x + objBounds.size.x <= centerX;
    bool east = objBounds.position.x >= centerX;
    
    if (north && west) return 0; // NW
    if (north && east) return 1; // NE
    if (south && west) return 2; // SW
    if (south && east) return 3; // SE
    
    return -1; // Cannot fit completely in any quadrant
}ntation
QuadTree::QuadTree(const Config& config) : config_(config) {
    root_ = std::make_unique<Node>(config_.bounds, 0);
}

void QuadTree::clear() {
    root_ = std::make_unique<Node>(config_.bounds, 0);
    colliders_.clear();
}

void QuadTree::insert(const CollisionBox& collider) {
    // Store a copy of the collider
    auto colliderCopy = std::make_unique<CollisionBox>(collider);
    const CollisionBox* colliderPtr = colliderCopy.get();
    colliders_.push_back(std::move(colliderCopy));
    
    insertIntoNode(root_.get(), colliderPtr);
}

void QuadTree::remove(entities::Entity* entity) {
    // Remove from storage
    colliders_.erase(
        std::remove_if(colliders_.begin(), colliders_.end(),
            [entity](const std::unique_ptr<CollisionBox>& cb) {
                return cb->owner() == entity;
            }),
        colliders_.end());
    
    // Rebuild tree (simple approach - could be optimized)
    auto oldColliders = std::move(colliders_);
    clear();
    for (const auto& cb : oldColliders) {
        insert(*cb);
    }
}

std::vector<const CollisionBox*> QuadTree::query(const sf::FloatRect& bounds) const {
    std::vector<const CollisionBox*> result;
    queryNode(root_.get(), bounds, result);
    return result;
}

std::vector<const CollisionBox*> QuadTree::querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<const CollisionBox*> result;
    querySegmentNode(root_.get(), p0, p1, result);
    return result;
}

QuadTree::Stats QuadTree::getStats() const {
    Stats stats;
    getStatsFromNode(root_.get(), stats);
    return stats;
}

void QuadTree::insertIntoNode(Node* node, const CollisionBox* collider) {
    if (!node) return;
    
    // If leaf and not at max depth and too many objects, subdivide
    if (node->isLeaf() && node->depth < config_.maxDepth && 
        static_cast<int>(node->objects.size()) >= config_.maxObjectsPerNode) {
        node->subdivide();
    }
    
    if (!node->isLeaf()) {
        // Try to insert into appropriate child
        int quadrant = node->getQuadrant(collider->getBounds());
        if (quadrant != -1) {
            insertIntoNode(node->children[quadrant].get(), collider);
            return;
        }
    }
    
    // Insert into this node
    node->objects.push_back(collider);
}

void QuadTree::queryNode(const Node* node, const sf::FloatRect& bounds, std::vector<const CollisionBox*>& result) const {
    if (!node || !bounds.findIntersection(node->bounds).has_value()) {
        return;
    }
    
    // Add objects from this node
    for (const auto* obj : node->objects) {
        if (bounds.findIntersection(obj->getBounds()).has_value()) {
            result.push_back(obj);
        }
    }
    
    // Recurse into children
    if (!node->isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            queryNode(node->children[i].get(), bounds, result);
        }
    }
}

void QuadTree::querySegmentNode(const Node* node, const sf::Vector2f& p0, const sf::Vector2f& p1, std::vector<const CollisionBox*>& result) const {
    if (!node || !segmentIntersectsRect(p0, p1, node->bounds)) {
        return;
    }
    
    // Check objects in this node
    for (const auto* obj : node->objects) {
        if (segmentIntersectsRect(p0, p1, obj->getBounds())) {
            result.push_back(obj);
        }
    }
    
    // Recurse into children
    if (!node->isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            querySegmentNode(node->children[i].get(), p0, p1, result);
        }
    }
}

void QuadTree::getStatsFromNode(const Node* node, Stats& stats) const {
    if (!node) return;
    
    stats.totalNodes++;
    stats.totalObjects += static_cast<int>(node->objects.size());
    stats.maxDepthReached = std::max(stats.maxDepthReached, node->depth);
    
    if (node->isLeaf()) {
        stats.leafNodes++;
    } else {
        for (int i = 0; i < 4; ++i) {
            getStatsFromNode(node->children[i].get(), stats);
        }
    }
}

bool QuadTree::segmentIntersectsRect(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::FloatRect& rect) const {
    // Liang-Barsky algorithm
    float t0 = 0.f, t1 = 1.f;
    sf::Vector2f d = p1 - p0;
    
    auto clip = [&](float p, float q) -> bool {
        if (std::abs(p) < 1e-6f) {
            return q >= 0.f;
        }
        float t = q / p;
        if (p < 0.f) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
        return true;
    };
    
    float minX = rect.left;
    float minY = rect.top;
    float maxX = rect.left + rect.width;
    float maxY = rect.top + rect.height;
    
    if (!clip(-d.x, p0.x - minX)) return false;
    if (!clip( d.x, maxX - p0.x)) return false;
    if (!clip(-d.y, p0.y - minY)) return false;
    if (!clip( d.y, maxY - p0.y)) return false;
    
    return t0 <= t1;
}

void QuadTree::Node::subdivide() {
    if (!isLeaf()) return;
    
    float halfWidth = bounds.size.x * 0.5f;
    float halfHeight = bounds.size.y * 0.5f;
    
    // Create children: NW, NE, SW, SE
    children[0] = std::make_unique<Node>(sf::FloatRect({bounds.position.x, bounds.position.y}, {halfWidth, halfHeight}), depth + 1);
    children[1] = std::make_unique<Node>(sf::FloatRect({bounds.position.x + halfWidth, bounds.position.y}, {halfWidth, halfHeight}), depth + 1);
    children[2] = std::make_unique<Node>(sf::FloatRect({bounds.position.x, bounds.position.y + halfHeight}, {halfWidth, halfHeight}), depth + 1);
    children[3] = std::make_unique<Node>(sf::FloatRect({bounds.position.x + halfWidth, bounds.position.y + halfHeight}, {halfWidth, halfHeight}), depth + 1);
}

int QuadTree::Node::getQuadrant(const sf::FloatRect& objBounds) const {
    float centerX = bounds.left + bounds.width * 0.5f;
    float centerY = bounds.top + bounds.height * 0.5f;
    
    bool north = objBounds.top + objBounds.height <= centerY;
    bool south = objBounds.top >= centerY;
    bool west = objBounds.left + objBounds.width <= centerX;
    bool east = objBounds.left >= centerX;
    
    if (north && west) return 0; // NW
    if (north && east) return 1; // NE
    if (south && west) return 2; // SW
    if (south && east) return 3; // SE
    
    return -1; // Doesn't fit entirely in any quadrant
}

sf::FloatRect QuadTree::Node::getQuadrantBounds(int quadrant) const {
    float halfWidth = bounds.width * 0.5f;
    float halfHeight = bounds.height * 0.5f;
    
    switch (quadrant) {
        case 0: return sf::FloatRect(bounds.left, bounds.top, halfWidth, halfHeight); // NW
        case 1: return sf::FloatRect(bounds.left + halfWidth, bounds.top, halfWidth, halfHeight); // NE
        case 2: return sf::FloatRect(bounds.left, bounds.top + halfHeight, halfWidth, halfHeight); // SW
        case 3: return sf::FloatRect(bounds.left + halfWidth, bounds.top + halfHeight, halfWidth, halfHeight); // SE
        default: return bounds;
    }
}

// SpatialHash Implementation
SpatialHash::SpatialHash(const Config& config) : config_(config) {}

void SpatialHash::clear() {
    cells_.clear();
    colliders_.clear();
}

void SpatialHash::insert(const CollisionBox& collider) {
    auto colliderCopy = std::make_unique<CollisionBox>(collider);
    const CollisionBox* colliderPtr = colliderCopy.get();
    colliders_.push_back(std::move(colliderCopy));
    
    auto cells = getCellsForRect(colliderPtr->getBounds());
    for (const auto& [x, y] : cells) {
        int64_t hash = hashCell(x, y);
        cells_[hash].push_back(colliderPtr);
    }
}

void SpatialHash::remove(entities::Entity* entity) {
    // Remove from all cells
    for (auto& [hash, cellObjects] : cells_) {
        cellObjects.erase(
            std::remove_if(cellObjects.begin(), cellObjects.end(),
                [entity](const CollisionBox* cb) {
                    return cb->owner() == entity;
                }),
            cellObjects.end());
    }
    
    // Remove from storage
    colliders_.erase(
        std::remove_if(colliders_.begin(), colliders_.end(),
            [entity](const std::unique_ptr<CollisionBox>& cb) {
                return cb->owner() == entity;
            }),
        colliders_.end());
}

std::vector<const CollisionBox*> SpatialHash::query(const sf::FloatRect& bounds) const {
    std::vector<const CollisionBox*> result;
    std::unordered_set<const CollisionBox*> uniqueResults;
    
    auto cells = getCellsForRect(bounds);
    for (const auto& [x, y] : cells) {
        int64_t hash = hashCell(x, y);
        auto it = cells_.find(hash);
        if (it != cells_.end()) {
            for (const auto* obj : it->second) {
                if (uniqueResults.find(obj) == uniqueResults.end() && 
                    bounds.findIntersection(obj->getBounds()).has_value()) {
                    uniqueResults.insert(obj);
                    result.push_back(obj);
                }
            }
        }
    }
    
    return result;
}

std::vector<const CollisionBox*> SpatialHash::querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<const CollisionBox*> result;
    std::unordered_set<const CollisionBox*> uniqueResults;
    
    auto cells = getCellsForSegment(p0, p1);
    for (const auto& [x, y] : cells) {
        int64_t hash = hashCell(x, y);
        auto it = cells_.find(hash);
        if (it != cells_.end()) {
            for (const auto* obj : it->second) {
                if (uniqueResults.find(obj) == uniqueResults.end()) {
                    uniqueResults.insert(obj);
                    result.push_back(obj);
                }
            }
        }
    }
    
    return result;
}

int64_t SpatialHash::hashCell(int x, int y) const {
    return (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
}

std::pair<int, int> SpatialHash::getCellCoords(float x, float y) const {
    return {
        static_cast<int>(std::floor((x - config_.bounds.left) / config_.cellSize)),
        static_cast<int>(std::floor((y - config_.bounds.top) / config_.cellSize))
    };
}

std::vector<std::pair<int, int>> SpatialHash::getCellsForRect(const sf::FloatRect& rect) const {
    std::vector<std::pair<int, int>> cells;
    
    auto [minX, minY] = getCellCoords(rect.left, rect.top);
    auto [maxX, maxY] = getCellCoords(rect.left + rect.width, rect.top + rect.height);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            cells.emplace_back(x, y);
        }
    }
    
    return cells;
}

std::vector<std::pair<int, int>> SpatialHash::getCellsForSegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<std::pair<int, int>> cells;
    
    auto [x0, y0] = getCellCoords(p0.x, p0.y);
    auto [x1, y1] = getCellCoords(p1.x, p1.y);
    
    // Bresenham-like algorithm for line rasterization
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int x = x0;
    int y = y0;
    int stepX = (x0 < x1) ? 1 : -1;
    int stepY = (y0 < y1) ? 1 : -1;
    
    if (dx > dy) {
        int err = dx / 2;
        while (x != x1) {
            cells.emplace_back(x, y);
            err -= dy;
            if (err < 0) {
                y += stepY;
                err += dx;
            }
            x += stepX;
        }
    } else {
        int err = dy / 2;
        while (y != y1) {
            cells.emplace_back(x, y);
            err -= dx;
            if (err < 0) {
                x += stepX;
                err += dy;
            }
            y += stepY;
        }
    }
    
    cells.emplace_back(x1, y1);
    return cells;
}

} // namespace collisions
