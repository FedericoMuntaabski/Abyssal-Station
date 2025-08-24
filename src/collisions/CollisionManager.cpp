#include "CollisionManager.h"
#include "../entities/Entity.h"
#include "../core/Logger.h"
#include "../ai/Enemy.h"
#include "../gameplay/Item.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <limits>

namespace collisions {

CollisionManager::CollisionManager(const Config& config) : config_(config) {
    initializeSpatialPartition();
    
    // Initialize default layer collision matrix (all layers can collide with each other by default)
    using entities::Entity;
    std::vector<std::uint32_t> layers = {
        static_cast<std::uint32_t>(Entity::Layer::Default),
        static_cast<std::uint32_t>(Entity::Layer::Player),
        static_cast<std::uint32_t>(Entity::Layer::Enemy),
        static_cast<std::uint32_t>(Entity::Layer::Item),
        static_cast<std::uint32_t>(Entity::Layer::Wall)
    };
    
    for (auto layerA : layers) {
        for (auto layerB : layers) {
            // Items don't collide with other items by default
            if (layerA == static_cast<std::uint32_t>(Entity::Layer::Item) && 
                layerB == static_cast<std::uint32_t>(Entity::Layer::Item)) {
                setLayerCollisionMatrix(layerA, layerB, false);
            } else {
                setLayerCollisionMatrix(layerA, layerB, true);
            }
        }
    }
}

void CollisionManager::setConfig(const Config& config) {
    config_ = config;
    initializeSpatialPartition();
}

void CollisionManager::addCollider(entities::Entity* owner, const sf::FloatRect& bounds) {
    if (!owner) return;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Try to find existing collider for owner and update
    for (auto& cb : colliders_) {
        if (cb.owner() == owner) {
            cb.setBounds(bounds);
            cb.setLayer(owner->collisionLayer());
            updateSpatialPartition();
            
            if (config_.enableProfiling) {
                auto endTime = std::chrono::high_resolution_clock::now();
                profileData_.totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            }
            return;
        }
    }

    // Not found -> add new and set its layer from owner
    colliders_.emplace_back(owner, bounds);
    colliders_.back().setLayer(owner->collisionLayer());
    
    // Enable dynamic resize for entities that might change size
    colliders_.back().setDynamicResize(true);
    
    updateSpatialPartition();
    
    if (config_.enableProfiling) {
        auto endTime = std::chrono::high_resolution_clock::now();
        profileData_.totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    }
}

void CollisionManager::updateColliderBounds(entities::Entity* owner, const sf::FloatRect& bounds) {
    // This is just an alias for addCollider since it already handles updates
    addCollider(owner, bounds);
}

void CollisionManager::removeCollider(entities::Entity* owner) {
    if (!owner) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    colliders_.erase(std::remove_if(colliders_.begin(), colliders_.end(), [owner](const CollisionBox& cb) {
        return cb.owner() == owner;
    }), colliders_.end());
    
    updateSpatialPartition();
    
    if (config_.enableProfiling) {
        auto endTime = std::chrono::high_resolution_clock::now();
        profileData_.totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    }
}

void CollisionManager::addMultiShapeCollider(entities::Entity* owner, std::vector<std::unique_ptr<CollisionShape>> shapes) {
    if (!owner || shapes.empty()) return;
    
    // Create or update collider
    CollisionBox* collider = findCollider(owner);
    if (!collider) {
        colliders_.emplace_back(owner, sf::FloatRect());
        collider = &colliders_.back();
        collider->setLayer(owner->collisionLayer());
        collider->setDynamicResize(true);
    }
    
    // Clear existing shapes and add new ones
    collider->clearShapes();
    for (auto& shape : shapes) {
        if (shape) {
            collider->addShape(std::move(shape));
        }
    }
    
    updateSpatialPartition();
}

void CollisionManager::updateMultiShapeCollider(entities::Entity* owner) {
    if (!owner) return;
    
    CollisionBox* collider = findCollider(owner);
    if (collider && collider->isDynamicResize()) {
        collider->updateFromEntity();
        updateSpatialPartition();
    }
}

std::vector<entities::Entity*> CollisionManager::checkCollisions(entities::Entity* owner) const {
    std::vector<entities::Entity*> result;
    if (!owner) return result;

    auto startTime = std::chrono::high_resolution_clock::now();
    
    const CollisionBox* subject = findCollider(owner);
    if (!subject) return result;

    std::vector<const CollisionBox*> candidates;
    
    if (spatialPartition_) {
        // Use spatial partitioning for broad phase
        auto broadPhaseStart = std::chrono::high_resolution_clock::now();
        candidates = spatialPartition_->query(subject->getBounds());
        
        if (config_.enableProfiling) {
            auto broadPhaseEnd = std::chrono::high_resolution_clock::now();
            profileData_.broadPhaseTime += std::chrono::duration_cast<std::chrono::microseconds>(broadPhaseEnd - broadPhaseStart);
            profileData_.broadPhaseTests += static_cast<int>(candidates.size());
        }
    } else {
        // Brute force approach
        for (const auto& cb : colliders_) {
            candidates.push_back(&cb);
        }
    }

    // Narrow phase
    auto narrowPhaseStart = std::chrono::high_resolution_clock::now();
    
    for (const auto* cb : candidates) {
        if (cb->owner() == owner) continue;
        
        // Check layer collision matrix
        if (!getLayerCollisionMatrix(subject->layer(), cb->layer())) continue;
        
        auto inter = subject->getBounds().findIntersection(cb->getBounds());
        if (inter.has_value()) {
            // Log collision for debug
            core::Logger::instance().info("[CollisionManager] Collision detected between entities id=" + std::to_string(owner->id()) +
                " and id=" + std::to_string(cb->owner()->id()));
            result.push_back(cb->owner());
        }
    }
    
    if (config_.enableProfiling) {
        auto narrowPhaseEnd = std::chrono::high_resolution_clock::now();
        profileData_.narrowPhaseTime += std::chrono::duration_cast<std::chrono::microseconds>(narrowPhaseEnd - narrowPhaseStart);
        profileData_.narrowPhaseTests += static_cast<int>(candidates.size());
        profileData_.totalQueries++;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        profileData_.totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    }

    return result;
}

std::vector<CollisionResult> CollisionManager::checkCollisionsDetailed(entities::Entity* owner) const {
    std::vector<CollisionResult> results;
    if (!owner) return results;

    const CollisionBox* subject = findCollider(owner);
    if (!subject) return results;

    std::vector<const CollisionBox*> candidates;
    
    if (spatialPartition_) {
        candidates = spatialPartition_->query(subject->getBounds());
    } else {
        for (const auto& cb : colliders_) {
            candidates.push_back(&cb);
        }
    }

    for (const auto* cb : candidates) {
        if (cb->owner() == owner) continue;
        if (!getLayerCollisionMatrix(subject->layer(), cb->layer())) continue;
        
        CollisionResult result;
        if (testCollision(subject->getBounds(), cb->getBounds(), result)) {
            result.entityA = owner;
            result.entityB = cb->owner();
            results.push_back(result);
        }
    }

    return results;
}

entities::Entity* CollisionManager::firstColliderForBounds(const sf::FloatRect& bounds, entities::Entity* exclude, std::uint32_t allowedLayers) const {
    std::vector<const CollisionBox*> candidates;
    
    if (spatialPartition_) {
        candidates = spatialPartition_->query(bounds);
    } else {
        for (const auto& cb : colliders_) {
            candidates.push_back(&cb);
        }
    }
    
    for (const auto* cb : candidates) {
        if (cb->owner() == exclude) continue;
        // Filter by allowedLayers mask
        if (allowedLayers != 0xFFFFFFFFu) {
            if ((cb->layer() & allowedLayers) == 0) continue;
        }
        auto opt = bounds.findIntersection(cb->getBounds());
        if (opt.has_value()) return cb->owner();
    }
    return nullptr;
}

RaycastHit CollisionManager::raycast(const sf::Vector2f& origin, const sf::Vector2f& direction, float maxDistance, 
                                    entities::Entity* exclude, std::uint32_t allowedLayers) const {
    sf::Vector2f endPoint = origin + direction * maxDistance;
    return segmentIntersection(origin, endPoint, exclude, allowedLayers);
}

bool CollisionManager::segmentIntersectsAny(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude, std::uint32_t allowedLayers) const {
    return segmentIntersection(p0, p1, exclude, allowedLayers).valid;
}

RaycastHit CollisionManager::segmentIntersection(const sf::Vector2f& p0, const sf::Vector2f& p1, entities::Entity* exclude, std::uint32_t allowedLayers) const {
    RaycastHit closestHit;
    float closestDistance = std::numeric_limits<float>::max();
    
    std::vector<const CollisionBox*> candidates;
    
    if (spatialPartition_) {
        candidates = spatialPartition_->querySegment(p0, p1);
    } else {
        for (const auto& cb : colliders_) {
            candidates.push_back(&cb);
        }
    }
    
    for (const auto* cb : candidates) {
        if (cb->owner() == exclude) continue;
        if (allowedLayers != 0xFFFFFFFFu) {
            if ((cb->layer() & allowedLayers) == 0) continue;
        }
        
        RaycastHit hit;
        if (segmentIntersectsRect(p0, p1, cb->getBounds(), hit)) {
            if (hit.distance < closestDistance) {
                closestDistance = hit.distance;
                closestHit = hit;
                closestHit.entity = cb->owner();
            }
        }
    }
    
    return closestHit;
}

std::vector<CollisionResult> CollisionManager::sweepTest(const sf::FloatRect& bounds, const sf::Vector2f& velocity, float deltaTime, 
                                                         entities::Entity* exclude, std::uint32_t allowedLayers) const {
    std::vector<CollisionResult> results;
    
    // Create swept bounds
    sf::Vector2f displacement = velocity * deltaTime;
    sf::FloatRect sweptBounds = bounds;
    
    if (displacement.x < 0) {
        sweptBounds.position.x += displacement.x;
        sweptBounds.size.x += -displacement.x;
    } else {
        sweptBounds.size.x += displacement.x;
    }
    
    if (displacement.y < 0) {
        sweptBounds.position.y += displacement.y;
        sweptBounds.size.y += -displacement.y;
    } else {
        sweptBounds.size.y += displacement.y;
    }
    
    std::vector<const CollisionBox*> candidates;
    
    if (spatialPartition_) {
        candidates = spatialPartition_->query(sweptBounds);
    } else {
        for (const auto& cb : colliders_) {
            candidates.push_back(&cb);
        }
    }
    
    for (const auto* cb : candidates) {
        if (cb->owner() == exclude) continue;
        if (allowedLayers != 0xFFFFFFFFu) {
            if ((cb->layer() & allowedLayers) == 0) continue;
        }
        
        CollisionResult result;
        if (testCollision(sweptBounds, cb->getBounds(), result)) {
            result.entityB = cb->owner();
            results.push_back(result);
        }
    }
    
    return results;
}

bool CollisionManager::layerMaskIntersects(std::uint32_t layerA, std::uint32_t allowedLayers) const {
    return (layerA & allowedLayers) != 0;
}

void CollisionManager::setLayerCollisionMatrix(std::uint32_t layerA, std::uint32_t layerB, bool canCollide) {
    std::uint64_t key = makeLayerPairKey(layerA, layerB);
    layerCollisionMatrix_[key] = canCollide;
    
    // Ensure symmetry
    if (layerA != layerB) {
        std::uint64_t reverseKey = makeLayerPairKey(layerB, layerA);
        layerCollisionMatrix_[reverseKey] = canCollide;
    }
}

bool CollisionManager::getLayerCollisionMatrix(std::uint32_t layerA, std::uint32_t layerB) const {
    std::uint64_t key = makeLayerPairKey(layerA, layerB);
    auto it = layerCollisionMatrix_.find(key);
    return it != layerCollisionMatrix_.end() ? it->second : true; // Default to true
}

void CollisionManager::resetProfileData() {
    profileData_ = ProfileData{};
}

std::string CollisionManager::getSpatialPartitionStats() const {
    std::ostringstream oss;
    oss << "Spatial Partition: ";
    
    switch (config_.spatialPartition) {
        case SpatialPartitionType::None:
            oss << "None (Brute Force)";
            break;
        case SpatialPartitionType::QuadTree:
            if (auto* quadTree = dynamic_cast<const QuadTree*>(spatialPartition_.get())) {
                auto stats = quadTree->getStats();
                oss << "QuadTree - Nodes: " << stats.totalNodes 
                    << ", Leaves: " << stats.leafNodes 
                    << ", Objects: " << stats.totalObjects 
                    << ", Max Depth: " << stats.maxDepthReached;
            }
            break;
        case SpatialPartitionType::SpatialHash:
            oss << "SpatialHash - Cell Size: " << config_.spatialHashConfig.cellSize;
            break;
    }
    
    return oss.str();
}

void CollisionManager::rebuildSpatialPartition() {
    updateSpatialPartition();
}

void CollisionManager::initializeSpatialPartition() {
    switch (config_.spatialPartition) {
        case SpatialPartitionType::QuadTree:
            spatialPartition_ = std::make_unique<QuadTree>(config_.quadTreeConfig);
            break;
        case SpatialPartitionType::SpatialHash:
            spatialPartition_ = std::make_unique<SpatialHash>(config_.spatialHashConfig);
            break;
        case SpatialPartitionType::None:
        default:
            spatialPartition_.reset();
            break;
    }
}

void CollisionManager::updateSpatialPartition() {
    if (!spatialPartition_) return;
    
    spatialPartition_->clear();
    for (const auto& cb : colliders_) {
        spatialPartition_->insert(cb);
    }
}

CollisionBox* CollisionManager::findCollider(entities::Entity* owner) {
    for (auto& cb : colliders_) {
        if (cb.owner() == owner) {
            return &cb;
        }
    }
    return nullptr;
}

const CollisionBox* CollisionManager::findCollider(entities::Entity* owner) const {
    for (const auto& cb : colliders_) {
        if (cb.owner() == owner) {
            return &cb;
        }
    }
    return nullptr;
}

bool CollisionManager::testCollision(const sf::FloatRect& a, const sf::FloatRect& b, CollisionResult& result) const {
    auto intersection = a.findIntersection(b);
    if (intersection.has_value()) {
        result.intersection = intersection.value();
        result.normal = calculateCollisionNormal(a, b);
        return true;
    }
    return false;
}

sf::Vector2f CollisionManager::calculateCollisionNormal(const sf::FloatRect& a, const sf::FloatRect& b) const {
    sf::Vector2f centerA(a.position.x + a.size.x * 0.5f, a.position.y + a.size.y * 0.5f);
    sf::Vector2f centerB(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f);
    
    sf::Vector2f direction = centerB - centerA;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length > 0.f) {
        return sf::Vector2f(direction.x / length, direction.y / length);
    }
    
    return sf::Vector2f(0.f, 0.f);
}

bool CollisionManager::rayIntersectsRect(const sf::Vector2f& origin, const sf::Vector2f& direction, const sf::FloatRect& rect, RaycastHit& hit) const {
    // Ray-AABB intersection using slab method
    sf::Vector2f invDir(1.f / direction.x, 1.f / direction.y);
    
    float t1 = (rect.position.x - origin.x) * invDir.x;
    float t2 = (rect.position.x + rect.size.x - origin.x) * invDir.x;
    float t3 = (rect.position.y - origin.y) * invDir.y;
    float t4 = (rect.position.y + rect.size.y - origin.y) * invDir.y;
    
    float tmin = std::max(std::min(t1, t2), std::min(t3, t4));
    float tmax = std::min(std::max(t1, t2), std::max(t3, t4));
    
    if (tmax < 0 || tmin > tmax) {
        return false;
    }
    
    float t = (tmin < 0) ? tmax : tmin;
    if (t < 0) return false;
    
    hit.point = origin + direction * t;
    hit.distance = t;
    hit.valid = true;
    
    // Calculate normal
    sf::Vector2f center(rect.position.x + rect.size.x * 0.5f, rect.position.y + rect.size.y * 0.5f);
    sf::Vector2f toHit = hit.point - center;
    
    if (std::abs(toHit.x) > std::abs(toHit.y)) {
        hit.normal = sf::Vector2f((toHit.x > 0) ? 1.f : -1.f, 0.f);
    } else {
        hit.normal = sf::Vector2f(0.f, (toHit.y > 0) ? 1.f : -1.f);
    }
    
    return true;
}

bool CollisionManager::segmentIntersectsRect(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::FloatRect& rect, RaycastHit& hit) const {
    sf::Vector2f direction = p1 - p0;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length == 0.f) return false;
    
    direction = sf::Vector2f(direction.x / length, direction.y / length);
    
    if (rayIntersectsRect(p0, direction, rect, hit)) {
        if (hit.distance <= length) {
            return true;
        }
    }
    
    hit.valid = false;
    return false;
}

std::uint64_t CollisionManager::makeLayerPairKey(std::uint32_t layerA, std::uint32_t layerB) const {
    // Ensure consistent ordering
    if (layerA > layerB) std::swap(layerA, layerB);
    return (static_cast<std::uint64_t>(layerA) << 32) | static_cast<std::uint64_t>(layerB);
}

} // namespace collisions
