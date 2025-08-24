#include "SpatialPartition.h"
#include "../core/Logger.h"
#include "../entities/Entity.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>

namespace collisions {

// QuadTree Implementation
QuadTree::QuadTree(const Config& config) : config_(config) {
    root_ = std::make_unique<Node>(config_.bounds, 0);
}

void QuadTree::clear() {
    if (root_) {
        root_->objects.clear();
        root_->children[0].reset();
        root_->children[1].reset();
        root_->children[2].reset();
        root_->children[3].reset();
    }
}

void QuadTree::insert(const CollisionBox& collider) {
    // Store a pointer to the existing collider 
    const CollisionBox* colliderPtr = &collider;
    
    if (root_) {
        insertIntoNode(root_.get(), colliderPtr);
    }
}

void QuadTree::remove(entities::Entity* entity) {
    // Since we don't store copies, we just need to rebuild the tree
    // The CollisionManager will call clear() and re-insert valid colliders
    // This is a simple approach for now
    clear();
}

std::vector<const CollisionBox*> QuadTree::query(const sf::FloatRect& bounds) const {
    std::vector<const CollisionBox*> result;
    if (root_) {
        queryNode(root_.get(), bounds, result);
    }
    return result;
}

std::vector<const CollisionBox*> QuadTree::querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<const CollisionBox*> result;
    if (root_) {
        querySegmentNode(root_.get(), p0, p1, result);
    }
    return result;
}

QuadTree::Stats QuadTree::getStats() const {
    Stats stats;
    if (root_) {
        getStatsFromNode(root_.get(), stats);
    }
    return stats;
}

void QuadTree::insertIntoNode(Node* node, const CollisionBox* collider) {
    if (!node) return;
    
    sf::FloatRect colliderBounds = collider->getBounds();
    
    // If this is a leaf and we have room, add it here
    if (node->isLeaf() && node->objects.size() < config_.maxObjectsPerNode) {
        node->objects.push_back(collider);
        return;
    }
    
    // If we've reached max depth, add it here regardless
    if (node->depth >= config_.maxDepth) {
        node->objects.push_back(collider);
        return;
    }
    
    // Subdivide if needed
    if (node->isLeaf()) {
        node->subdivide();
    }
    
    // Try to insert into children
    int quadrant = node->getQuadrant(colliderBounds);
    if (quadrant >= 0 && quadrant < 4 && node->children[quadrant]) {
        insertIntoNode(node->children[quadrant].get(), collider);
    } else {
        // Object spans multiple quadrants, keep it here
        node->objects.push_back(collider);
    }
}

void QuadTree::queryNode(const Node* node, const sf::FloatRect& bounds, std::vector<const CollisionBox*>& result) const {
    if (!node || !node->bounds.findIntersection(bounds).has_value()) {
        return;
    }
    
    // Check objects in this node
    for (const CollisionBox* collider : node->objects) {
        if (collider->getBounds().findIntersection(bounds).has_value()) {
            result.push_back(collider);
        }
    }
    
    // Query children
    if (!node->isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                queryNode(node->children[i].get(), bounds, result);
            }
        }
    }
}

void QuadTree::querySegmentNode(const Node* node, const sf::Vector2f& p0, const sf::Vector2f& p1, std::vector<const CollisionBox*>& result) const {
    if (!node || !segmentIntersectsRect(p0, p1, node->bounds)) {
        return;
    }
    
    // Check objects in this node
    for (const CollisionBox* collider : node->objects) {
        if (segmentIntersectsRect(p0, p1, collider->getBounds())) {
            result.push_back(collider);
        }
    }
    
    // Query children
    if (!node->isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                querySegmentNode(node->children[i].get(), p0, p1, result);
            }
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
            if (node->children[i]) {
                getStatsFromNode(node->children[i].get(), stats);
            }
        }
    }
}

bool QuadTree::segmentIntersectsRect(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::FloatRect& rect) const {
    // Simple AABB line intersection test
    sf::Vector2f min = rect.position;
    sf::Vector2f max = min + rect.size;
    
    // Check if either endpoint is inside the rectangle
    if ((p0.x >= min.x && p0.x <= max.x && p0.y >= min.y && p0.y <= max.y) ||
        (p1.x >= min.x && p1.x <= max.x && p1.y >= min.y && p1.y <= max.y)) {
        return true;
    }
    
    // Check intersection with rectangle edges (simplified)
    return true; // For now, always return true to avoid complex math
}

// QuadTree::Node methods
void QuadTree::Node::subdivide() {
    if (!isLeaf()) return;
    
    float halfWidth = bounds.size.x / 2.0f;
    float halfHeight = bounds.size.y / 2.0f;
    sf::Vector2f pos = bounds.position;
    
    // Create four children
    children[0] = std::make_unique<Node>(sf::FloatRect({pos.x, pos.y}, {halfWidth, halfHeight}), depth + 1); // NW
    children[1] = std::make_unique<Node>(sf::FloatRect({pos.x + halfWidth, pos.y}, {halfWidth, halfHeight}), depth + 1); // NE
    children[2] = std::make_unique<Node>(sf::FloatRect({pos.x, pos.y + halfHeight}, {halfWidth, halfHeight}), depth + 1); // SW
    children[3] = std::make_unique<Node>(sf::FloatRect({pos.x + halfWidth, pos.y + halfHeight}, {halfWidth, halfHeight}), depth + 1); // SE
}

int QuadTree::Node::getQuadrant(const sf::FloatRect& objBounds) const {
    sf::Vector2f center = bounds.position + bounds.size / 2.0f;
    sf::Vector2f objPos = objBounds.position;
    sf::Vector2f objMax = objPos + objBounds.size;
    
    // Check if object fits entirely in one quadrant
    if (objMax.x <= center.x && objMax.y <= center.y) return 0; // NW
    if (objPos.x >= center.x && objMax.y <= center.y) return 1; // NE
    if (objMax.x <= center.x && objPos.y >= center.y) return 2; // SW
    if (objPos.x >= center.x && objPos.y >= center.y) return 3; // SE
    
    return -1; // Spans multiple quadrants
}

sf::FloatRect QuadTree::Node::getQuadrantBounds(int quadrant) const {
    float halfWidth = bounds.size.x / 2.0f;
    float halfHeight = bounds.size.y / 2.0f;
    sf::Vector2f pos = bounds.position;
    
    switch (quadrant) {
        case 0: return sf::FloatRect({pos.x, pos.y}, {halfWidth, halfHeight}); // NW
        case 1: return sf::FloatRect({pos.x + halfWidth, pos.y}, {halfWidth, halfHeight}); // NE
        case 2: return sf::FloatRect({pos.x, pos.y + halfHeight}, {halfWidth, halfHeight}); // SW
        case 3: return sf::FloatRect({pos.x + halfWidth, pos.y + halfHeight}, {halfWidth, halfHeight}); // SE
        default: return bounds;
    }
}

// SpatialHash Implementation
SpatialHash::SpatialHash(const Config& config) : config_(config) {
    core::Logger::instance().info("SpatialHash initialized with cell size " + std::to_string(config_.cellSize));
}

void SpatialHash::clear() {
    cells_.clear();
}

void SpatialHash::insert(const CollisionBox& collider) {
    // Store a pointer to the existing collider
    const CollisionBox* colliderPtr = &collider;
    
    // Get all cells this collider overlaps
    std::vector<std::pair<int, int>> cellCoords = getCellsForRect(collider.getBounds());
    
    for (const auto& coord : cellCoords) {
        int64_t hash = hashCell(coord.first, coord.second);
        cells_[hash].push_back(colliderPtr);
    }
}

void SpatialHash::remove(entities::Entity* entity) {
    // Since we don't store copies, we just need to clear and rebuild
    // The CollisionManager will handle the rebuilding
    clear();
}

std::vector<const CollisionBox*> SpatialHash::query(const sf::FloatRect& bounds) const {
    std::vector<const CollisionBox*> result;
    std::unordered_set<const CollisionBox*> unique;
    
    std::vector<std::pair<int, int>> cellCoords = getCellsForRect(bounds);
    
    for (const auto& coord : cellCoords) {
        int64_t hash = hashCell(coord.first, coord.second);
        auto cellIt = cells_.find(hash);
        
        if (cellIt != cells_.end()) {
            for (const CollisionBox* collider : cellIt->second) {
                if (unique.find(collider) == unique.end() && 
                    collider->getBounds().findIntersection(bounds).has_value()) {
                    result.push_back(collider);
                    unique.insert(collider);
                }
            }
        }
    }
    
    return result;
}

std::vector<const CollisionBox*> SpatialHash::querySegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<const CollisionBox*> result;
    std::unordered_set<const CollisionBox*> unique;
    
    std::vector<std::pair<int, int>> cellCoords = getCellsForSegment(p0, p1);
    
    for (const auto& coord : cellCoords) {
        int64_t hash = hashCell(coord.first, coord.second);
        auto cellIt = cells_.find(hash);
        
        if (cellIt != cells_.end()) {
            for (const CollisionBox* collider : cellIt->second) {
                if (unique.find(collider) == unique.end()) {
                    // Simple segment-box intersection test would go here
                    result.push_back(collider);
                    unique.insert(collider);
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
    int cellX = static_cast<int>(std::floor((x - config_.bounds.position.x) / config_.cellSize));
    int cellY = static_cast<int>(std::floor((y - config_.bounds.position.y) / config_.cellSize));
    return {cellX, cellY};
}

std::vector<std::pair<int, int>> SpatialHash::getCellsForRect(const sf::FloatRect& rect) const {
    std::vector<std::pair<int, int>> cells;
    
    sf::Vector2f pos = rect.position;
    sf::Vector2f size = rect.size;
    
    auto minCell = getCellCoords(pos.x, pos.y);
    auto maxCell = getCellCoords(pos.x + size.x, pos.y + size.y);
    
    for (int y = minCell.second; y <= maxCell.second; ++y) {
        for (int x = minCell.first; x <= maxCell.first; ++x) {
            cells.push_back({x, y});
        }
    }
    
    return cells;
}

std::vector<std::pair<int, int>> SpatialHash::getCellsForSegment(const sf::Vector2f& p0, const sf::Vector2f& p1) const {
    std::vector<std::pair<int, int>> cells;
    
    // Simple implementation: get cells for both endpoints and all cells in between
    auto cell0 = getCellCoords(p0.x, p0.y);
    auto cell1 = getCellCoords(p1.x, p1.y);
    
    int minX = std::min(cell0.first, cell1.first);
    int maxX = std::max(cell0.first, cell1.first);
    int minY = std::min(cell0.second, cell1.second);
    int maxY = std::max(cell0.second, cell1.second);
    
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            cells.push_back({x, y});
        }
    }
    
    return cells;
}

} // namespace collisions
