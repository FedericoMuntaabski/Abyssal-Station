#include "Pathfinding.h"
#include "collisions/CollisionManager.h"
#include "entities/Entity.h"
#include "core/Logger.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace ai {

PathfindingSystem::PathfindingSystem(const PathfindingConfig& config) : config_(config) {}

PathfindingResult PathfindingSystem::findPath(
    const sf::Vector2f& start,
    const sf::Vector2f& goal,
    collisions::CollisionManager* collisionManager,
    entities::Entity* pathEntity
) {
    PathfindingResult result;
    
    if (!collisionManager) {
        core::Logger::instance().warning("[AI] PathfindingSystem: No collision manager provided");
        return result;
    }
    
    // Check if direct path is clear first
    if (isPathClear(start, goal, collisionManager, pathEntity)) {
        result.path = {start, goal};
        result.success = true;
        result.totalCost = std::sqrt(std::pow(goal.x - start.x, 2) + std::pow(goal.y - start.y, 2));
        result.iterations = 1;
        return result;
    }
    
    // Convert to grid coordinates
    sf::Vector2f startGrid = worldToGrid(start);
    sf::Vector2f goalGrid = worldToGrid(goal);
    
    // A* algorithm
    std::priority_queue<PathNode*, std::vector<PathNode*>, NodeCompare> openSet;
    std::unordered_set<sf::Vector2f, Vector2fHash, Vector2fEqual> closedSet;
    std::unordered_map<sf::Vector2f, PathNode*, Vector2fHash, Vector2fEqual> allNodes;
    
    PathNode* startNode = new PathNode(startGrid, 0.0f, heuristic(startGrid, goalGrid));
    openSet.push(startNode);
    allNodes[startGrid] = startNode;
    
    int iterations = 0;
    PathNode* goalNode = nullptr;
    
    while (!openSet.empty() && iterations < config_.maxIterations) {
        iterations++;
        
        PathNode* current = openSet.top();
        openSet.pop();
        
        if (closedSet.find(current->position) != closedSet.end()) {
            continue;
        }
        
        closedSet.insert(current->position);
        
        // Check if we reached the goal
        if (std::abs(current->position.x - goalGrid.x) < 0.01f && 
            std::abs(current->position.y - goalGrid.y) < 0.01f) {
            goalNode = current;
            break;
        }
        
        // Explore neighbors
        auto neighbors = getNeighbors(current->position);
        for (const auto& neighborPos : neighbors) {
            if (closedSet.find(neighborPos) != closedSet.end()) {
                continue;
            }
            
            sf::Vector2f worldNeighbor = gridToWorld(neighborPos);
            if (!isWalkable(worldNeighbor, collisionManager, pathEntity)) {
                continue;
            }
            
            float tentativeG = current->gCost + heuristic(current->position, neighborPos);
            
            auto nodeIt = allNodes.find(neighborPos);
            PathNode* neighborNode = nullptr;
            
            if (nodeIt == allNodes.end()) {
                neighborNode = new PathNode(neighborPos, tentativeG, heuristic(neighborPos, goalGrid), current);
                allNodes[neighborPos] = neighborNode;
                openSet.push(neighborNode);
            } else {
                neighborNode = nodeIt->second;
                if (tentativeG < neighborNode->gCost) {
                    neighborNode->gCost = tentativeG;
                    neighborNode->parent = current;
                    openSet.push(neighborNode); // Re-add with updated cost
                }
            }
        }
    }
    
    result.iterations = iterations;
    
    if (goalNode) {
        // Reconstruct path
        auto gridPath = reconstructPath(goalNode);
        
        // Convert back to world coordinates
        for (const auto& gridPos : gridPath) {
            result.path.push_back(gridToWorld(gridPos));
        }
        
        // Smooth the path
        result.path = smoothPath(result.path, collisionManager, pathEntity);
        
        result.success = true;
        result.totalCost = goalNode->gCost;
    }
    
    // Cleanup
    for (auto& pair : allNodes) {
        delete pair.second;
    }
    
    return result;
}

std::vector<sf::Vector2f> PathfindingSystem::findSimplePath(
    const sf::Vector2f& start,
    const sf::Vector2f& goal,
    collisions::CollisionManager* collisionManager,
    entities::Entity* pathEntity
) {
    // Try direct path first
    if (isPathClear(start, goal, collisionManager, pathEntity)) {
        return {start, goal};
    }
    
    // Try simple avoidance by moving around obstacles
    std::vector<sf::Vector2f> path;
    path.push_back(start);
    
    sf::Vector2f direction = goal - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f) {
        direction /= length;
    }
    
    // Try perpendicular detours
    sf::Vector2f perpendicular(-direction.y, direction.x);
    float detourDistance = config_.gridSize * 2.0f;
    
    std::vector<sf::Vector2f> detourPoints = {
        start + perpendicular * detourDistance,
        start - perpendicular * detourDistance
    };
    
    for (const auto& detour : detourPoints) {
        if (isPathClear(start, detour, collisionManager, pathEntity) &&
            isPathClear(detour, goal, collisionManager, pathEntity)) {
            path.push_back(detour);
            path.push_back(goal);
            return path;
        }
    }
    
    // If simple avoidance fails, fall back to A*
    auto result = findPath(start, goal, collisionManager, pathEntity);
    return result.path;
}

bool PathfindingSystem::isPathClear(
    const sf::Vector2f& start,
    const sf::Vector2f& goal,
    collisions::CollisionManager* collisionManager,
    entities::Entity* excludeEntity
) const {
    if (!collisionManager) return true;
    
    return !collisionManager->segmentIntersectsAny(start, goal, excludeEntity, config_.obstacleLayerMask);
}

std::vector<sf::Vector2f> PathfindingSystem::smoothPath(
    const std::vector<sf::Vector2f>& path,
    collisions::CollisionManager* collisionManager,
    entities::Entity* pathEntity
) const {
    if (path.size() <= 2) return path;
    
    std::vector<sf::Vector2f> smoothed;
    smoothed.push_back(path[0]);
    
    size_t currentIndex = 0;
    while (currentIndex < path.size() - 1) {
        size_t farthestIndex = currentIndex + 1;
        
        // Find the farthest point we can reach directly
        for (size_t i = currentIndex + 2; i < path.size(); ++i) {
            if (isPathClear(path[currentIndex], path[i], collisionManager, pathEntity)) {
                farthestIndex = i;
            } else {
                break;
            }
        }
        
        smoothed.push_back(path[farthestIndex]);
        currentIndex = farthestIndex;
    }
    
    return smoothed;
}

sf::Vector2f PathfindingSystem::worldToGrid(const sf::Vector2f& worldPos) const {
    return sf::Vector2f(
        std::floor(worldPos.x / config_.gridSize),
        std::floor(worldPos.y / config_.gridSize)
    );
}

sf::Vector2f PathfindingSystem::gridToWorld(const sf::Vector2f& gridPos) const {
    return sf::Vector2f(
        gridPos.x * config_.gridSize + config_.gridSize * 0.5f,
        gridPos.y * config_.gridSize + config_.gridSize * 0.5f
    );
}

float PathfindingSystem::heuristic(const sf::Vector2f& a, const sf::Vector2f& b) const {
    float dx = std::abs(a.x - b.x);
    float dy = std::abs(a.y - b.y);
    
    if (config_.allowDiagonal) {
        // Diagonal distance
        return std::max(dx, dy) + (config_.diagonalCost - 1.0f) * std::min(dx, dy);
    } else {
        // Manhattan distance
        return dx + dy;
    }
}

std::vector<sf::Vector2f> PathfindingSystem::getNeighbors(const sf::Vector2f& position) const {
    std::vector<sf::Vector2f> neighbors;
    
    // Cardinal directions
    neighbors.emplace_back(position.x + 1, position.y);
    neighbors.emplace_back(position.x - 1, position.y);
    neighbors.emplace_back(position.x, position.y + 1);
    neighbors.emplace_back(position.x, position.y - 1);
    
    if (config_.allowDiagonal) {
        // Diagonal directions
        neighbors.emplace_back(position.x + 1, position.y + 1);
        neighbors.emplace_back(position.x + 1, position.y - 1);
        neighbors.emplace_back(position.x - 1, position.y + 1);
        neighbors.emplace_back(position.x - 1, position.y - 1);
    }
    
    return neighbors;
}

bool PathfindingSystem::isWalkable(const sf::Vector2f& position, collisions::CollisionManager* cm, entities::Entity* entity) const {
    if (!cm) return true;
    
    // Check if there's a collider at this position
    sf::FloatRect testBounds;
    testBounds.position = position - sf::Vector2f(config_.gridSize * 0.5f, config_.gridSize * 0.5f);
    testBounds.size = sf::Vector2f(config_.gridSize, config_.gridSize);
    
    auto blocker = cm->firstColliderForBounds(testBounds, entity, config_.obstacleLayerMask);
    return blocker == nullptr;
}

std::vector<sf::Vector2f> PathfindingSystem::reconstructPath(PathNode* goalNode) const {
    std::vector<sf::Vector2f> path;
    PathNode* current = goalNode;
    
    while (current) {
        path.push_back(current->position);
        current = current->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}

} // namespace ai
