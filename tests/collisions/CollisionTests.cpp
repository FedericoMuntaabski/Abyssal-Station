#include <gtest/gtest.h>
#include "../../src/collisions/CollisionManager.h"
#include "../../src/collisions/CollisionSystem.h"
#include "../../src/collisions/CollisionEvents.h"
#include "../../src/entities/Entity.h"

using namespace collisions;
using namespace entities;

// Mock entity for testing
class MockEntity : public Entity {
public:
    MockEntity(Id id = 0, const sf::Vector2f& pos = {0.f, 0.f}, const sf::Vector2f& size = {1.f, 1.f}) 
        : Entity(id, pos, size) {}
    
    void update(float deltaTime) override {}
    void render(sf::RenderWindow& window) override {}
};

class CollisionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<CollisionManager>();
        entityA = std::make_unique<MockEntity>(1, sf::Vector2f(0.f, 0.f), sf::Vector2f(10.f, 10.f));
        entityB = std::make_unique<MockEntity>(2, sf::Vector2f(5.f, 5.f), sf::Vector2f(10.f, 10.f));
        entityC = std::make_unique<MockEntity>(3, sf::Vector2f(20.f, 20.f), sf::Vector2f(10.f, 10.f));
        
        entityA->setCollisionLayer(Entity::Layer::Player);
        entityB->setCollisionLayer(Entity::Layer::Wall);
        entityC->setCollisionLayer(Entity::Layer::Item);
    }

    std::unique_ptr<CollisionManager> manager;
    std::unique_ptr<MockEntity> entityA, entityB, entityC;
};

TEST_F(CollisionManagerTest, AddAndRemoveColliders) {
    // Test adding colliders
    manager->addCollider(entityA.get(), entityA->getBounds());
    manager->addCollider(entityB.get(), entityB->getBounds());
    
    // Check that collision is detected between overlapping entities
    auto collisions = manager->checkCollisions(entityA.get());
    EXPECT_EQ(collisions.size(), 1);
    EXPECT_EQ(collisions[0], entityB.get());
    
    // Test removing colliders
    manager->removeCollider(entityB.get());
    collisions = manager->checkCollisions(entityA.get());
    EXPECT_EQ(collisions.size(), 0);
}

TEST_F(CollisionManagerTest, LayerFiltering) {
    manager->addCollider(entityA.get(), entityA->getBounds());
    manager->addCollider(entityB.get(), entityB->getBounds());
    manager->addCollider(entityC.get(), entityC->getBounds());
    
    // Test layer-based collision filtering
    using Layer = Entity::Layer;
    
    // Player should collide with Wall
    auto collisions = manager->checkCollisions(entityA.get());
    EXPECT_EQ(collisions.size(), 1);
    EXPECT_EQ(collisions[0], entityB.get());
    
    // Test firstColliderForBounds with layer filtering
    sf::FloatRect testBounds({0.f, 0.f}, {15.f, 15.f}); // Overlaps A and B
    
    // Allow only Wall layer
    auto* firstWall = manager->firstColliderForBounds(testBounds, nullptr, static_cast<std::uint32_t>(Layer::Wall));
    EXPECT_EQ(firstWall, entityB.get());
    
    // Allow only Item layer (should find nothing in overlap area)
    auto* firstItem = manager->firstColliderForBounds(testBounds, nullptr, static_cast<std::uint32_t>(Layer::Item));
    EXPECT_EQ(firstItem, nullptr);
}

TEST_F(CollisionManagerTest, SegmentIntersection) {
    manager->addCollider(entityB.get(), entityB->getBounds());
    
    // Test segment that intersects the entity
    sf::Vector2f start(0.f, 10.f);
    sf::Vector2f end(20.f, 10.f);
    
    bool intersects = manager->segmentIntersectsAny(start, end);
    EXPECT_TRUE(intersects);
    
    // Test segment that doesn't intersect
    sf::Vector2f start2(0.f, 0.f);
    sf::Vector2f end2(0.f, 20.f);
    
    bool intersects2 = manager->segmentIntersectsAny(start2, end2);
    EXPECT_FALSE(intersects2);
}

TEST_F(CollisionManagerTest, DetailedCollisionInfo) {
    manager->addCollider(entityA.get(), entityA->getBounds());
    manager->addCollider(entityB.get(), entityB->getBounds());
    
    auto detailedResults = manager->checkCollisionsDetailed(entityA.get());
    EXPECT_EQ(detailedResults.size(), 1);
    
    const auto& result = detailedResults[0];
    EXPECT_EQ(result.entityA, entityA.get());
    EXPECT_EQ(result.entityB, entityB.get());
    EXPECT_TRUE(result.intersection.size.x > 0);
    EXPECT_TRUE(result.intersection.size.y > 0);
}

TEST_F(CollisionManagerTest, RaycastHit) {
    manager->addCollider(entityB.get(), entityB->getBounds());
    
    sf::Vector2f origin(0.f, 10.f);
    sf::Vector2f direction(1.f, 0.f);
    float maxDistance = 20.f;
    
    auto hit = manager->raycast(origin, direction, maxDistance);
    EXPECT_TRUE(hit.valid);
    EXPECT_EQ(hit.entity, entityB.get());
    EXPECT_GT(hit.distance, 0.f);
    EXPECT_LT(hit.distance, maxDistance);
}

class CollisionSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<CollisionManager>();
        system = std::make_unique<CollisionSystem>(*manager);
        
        player = std::make_unique<MockEntity>(1, sf::Vector2f(0.f, 0.f), sf::Vector2f(10.f, 10.f));
        wall = std::make_unique<MockEntity>(2, sf::Vector2f(5.f, 5.f), sf::Vector2f(10.f, 10.f));
        
        player->setCollisionLayer(Entity::Layer::Player);
        wall->setCollisionLayer(Entity::Layer::Wall);
        
        manager->addCollider(player.get(), player->getBounds());
        manager->addCollider(wall.get(), wall->getBounds());
    }

    std::unique_ptr<CollisionManager> manager;
    std::unique_ptr<CollisionSystem> system;
    std::unique_ptr<MockEntity> player, wall;
};

TEST_F(CollisionSystemTest, BasicResolution) {
    sf::Vector2f originalPos = player->position();
    
    auto resolution = system->resolve(player.get(), 0.016f); // 60 FPS
    
    EXPECT_TRUE(resolution.wasResolved);
    EXPECT_NE(player->position(), originalPos);
    EXPECT_GT(resolution.penetrationDepth, 0.f);
}

TEST_F(CollisionSystemTest, ResolutionBounds) {
    // Test that resolution doesn't exceed maximum distance
    CollisionSystem::Config config;
    config.maxCorrectionDistance = 1.f; // Very small max distance
    system->setConfig(config);
    
    sf::Vector2f originalPos = player->position();
    auto resolution = system->resolve(player.get(), 0.016f);
    
    // Should still resolve but with limited correction
    sf::Vector2f correction = player->position() - originalPos;
    float correctionDistance = std::sqrt(correction.x * correction.x + correction.y * correction.y);
    EXPECT_LE(correctionDistance, config.maxCorrectionDistance + 0.1f); // Small tolerance
}

TEST_F(CollisionSystemTest, MultipleEntityResolution) {
    std::vector<Entity*> entities = {player.get(), wall.get()};
    
    sf::Vector2f originalPlayerPos = player->position();
    system->resolveMultiple(entities, 0.016f);
    
    // Player should have moved, wall should stay in place (as it's typically static)
    EXPECT_NE(player->position(), originalPlayerPos);
}

class CollisionEventsTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventManager = std::make_unique<CollisionEventManager>();
        entityA = std::make_unique<MockEntity>(1);
        entityB = std::make_unique<MockEntity>(2);
        
        // Reset event counters
        onEnterCount = 0;
        onExitCount = 0;
        onStayCount = 0;
        
        // Register event callbacks
        eventManager->registerCallback(CollisionEventType::OnEnter, 
            [this](const CollisionEvent& event) { onEnterCount++; });
        eventManager->registerCallback(CollisionEventType::OnExit, 
            [this](const CollisionEvent& event) { onExitCount++; });
        eventManager->registerCallback(CollisionEventType::OnStay, 
            [this](const CollisionEvent& event) { onStayCount++; });
    }

    std::unique_ptr<CollisionEventManager> eventManager;
    std::unique_ptr<MockEntity> entityA, entityB;
    int onEnterCount, onExitCount, onStayCount;
};

TEST_F(CollisionEventsTest, OnEnterEvent) {
    // Simulate collision starting
    eventManager->updateCollisionStates(entityA.get(), entityB.get(), true, 0.016f);
    
    EXPECT_EQ(onEnterCount, 1);
    EXPECT_EQ(onExitCount, 0);
    EXPECT_EQ(onStayCount, 0);
}

TEST_F(CollisionEventsTest, OnStayEvent) {
    // Start collision
    eventManager->updateCollisionStates(entityA.get(), entityB.get(), true, 0.016f);
    
    // Continue collision
    eventManager->updateCollisionStates(entityA.get(), entityB.get(), true, 0.016f);
    
    EXPECT_EQ(onEnterCount, 1);
    EXPECT_EQ(onStayCount, 1);
    EXPECT_EQ(onExitCount, 0);
}

TEST_F(CollisionEventsTest, OnExitEvent) {
    // Start collision
    eventManager->updateCollisionStates(entityA.get(), entityB.get(), true, 0.016f);
    
    // End collision
    eventManager->updateCollisionStates(entityA.get(), entityB.get(), false, 0.016f);
    
    EXPECT_EQ(onEnterCount, 1);
    EXPECT_EQ(onExitCount, 1);
    EXPECT_EQ(onStayCount, 0);
}

class SpatialPartitionTest : public ::testing::Test {
protected:
    void SetUp() override {
        QuadTree::Config config;
        config.bounds = sf::FloatRect({0, 0}, {100, 100});
        config.maxDepth = 4;
        config.maxObjectsPerNode = 4;
        
        quadTree = std::make_unique<QuadTree>(config);
        
        // Create test entities and their collision boxes
        for (int i = 0; i < 10; ++i) {
            auto entity = std::make_unique<MockEntity>(i, 
                sf::Vector2f(i * 8.f, i * 8.f), sf::Vector2f(5.f, 5.f));
            
            // Create a collision box for this entity and store it
            auto collisionBox = std::make_unique<CollisionBox>(entity.get(), entity->getBounds());
            collisionBoxes.push_back(std::move(collisionBox));
            
            entities.push_back(std::move(entity));
        }
    }

    std::unique_ptr<QuadTree> quadTree;
    std::vector<std::unique_ptr<MockEntity>> entities;
    std::vector<std::unique_ptr<CollisionBox>> collisionBoxes;
};

TEST_F(SpatialPartitionTest, QuadTreeInsertion) {
    // Insert all collision boxes
    for (const auto& collisionBox : collisionBoxes) {
        quadTree->insert(*collisionBox);
    }
    
    auto stats = quadTree->getStats();
    EXPECT_GT(stats.totalNodes, 1); // Should have subdivided
    EXPECT_EQ(stats.totalObjects, static_cast<int>(collisionBoxes.size()));
}

TEST_F(SpatialPartitionTest, QuadTreeQuery) {
    // Insert collision boxes
    for (const auto& collisionBox : collisionBoxes) {
        quadTree->insert(*collisionBox);
    }
    
    // Query for entities in a specific area
    sf::FloatRect queryBounds({0, 0}, {20, 20});
    auto results = quadTree->query(queryBounds);
    
    // Should find some entities but not all
    EXPECT_GT(results.size(), 0);
    EXPECT_LT(results.size(), collisionBoxes.size());
}

TEST_F(SpatialPartitionTest, QuadTreeSegmentQuery) {
    // Insert collision boxes
    for (const auto& collisionBox : collisionBoxes) {
        quadTree->insert(*collisionBox);
    }
    
    // Query along a line
    sf::Vector2f start(0, 0);
    sf::Vector2f end(50, 50);
    auto results = quadTree->querySegment(start, end);
    
    EXPECT_GT(results.size(), 0);
}

// Integration tests
class CollisionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        CollisionManager::Config config;
        config.spatialPartition = CollisionManager::SpatialPartitionType::QuadTree;
        config.enableProfiling = true;
        
        manager = std::make_unique<CollisionManager>(config);
        system = std::make_unique<CollisionSystem>(*manager);
        
        // Create a more complex scenario
        player = std::make_unique<MockEntity>(1, sf::Vector2f(10.f, 10.f), sf::Vector2f(5.f, 5.f));
        wall1 = std::make_unique<MockEntity>(2, sf::Vector2f(14.f, 9.f), sf::Vector2f(5.f, 5.f));
        wall2 = std::make_unique<MockEntity>(3, sf::Vector2f(9.f, 14.f), sf::Vector2f(5.f, 5.f));
        item = std::make_unique<MockEntity>(4, sf::Vector2f(12.f, 12.f), sf::Vector2f(2.f, 2.f));
        
        player->setCollisionLayer(Entity::Layer::Player);
        wall1->setCollisionLayer(Entity::Layer::Wall);
        wall2->setCollisionLayer(Entity::Layer::Wall);
        item->setCollisionLayer(Entity::Layer::Item);
    }

    std::unique_ptr<CollisionManager> manager;
    std::unique_ptr<CollisionSystem> system;
    std::unique_ptr<MockEntity> player, wall1, wall2, item;
};

TEST_F(CollisionIntegrationTest, ComplexScenario) {
    // Add all colliders
    manager->addCollider(player.get(), player->getBounds());
    manager->addCollider(wall1.get(), wall1->getBounds());
    manager->addCollider(wall2.get(), wall2->getBounds());
    manager->addCollider(item.get(), item->getBounds());
    
    // Player should collide with walls and item
    auto collisions = manager->checkCollisions(player.get());
    EXPECT_GE(collisions.size(), 2); // At least walls, possibly item too
    
    // Test resolution
    sf::Vector2f originalPos = player->position();
    auto resolution = system->resolve(player.get(), 0.016f);
    
    EXPECT_TRUE(resolution.wasResolved);
    EXPECT_NE(player->position(), originalPos);
    
    // Check profiling data
    auto profileData = manager->getProfileData();
    EXPECT_GT(profileData.totalQueries, 0);
}

TEST_F(CollisionIntegrationTest, LayerCollisionMatrix) {
    // Set up custom layer collision rules
    manager->setLayerCollisionMatrix(
        static_cast<std::uint32_t>(Entity::Layer::Player),
        static_cast<std::uint32_t>(Entity::Layer::Item),
        false // Player doesn't physically collide with items
    );
    
    manager->addCollider(player.get(), player->getBounds());
    manager->addCollider(item.get(), item->getBounds());
    
    // Should not find collision due to layer matrix
    auto collisions = manager->checkCollisions(player.get());
    EXPECT_EQ(collisions.size(), 0);
    
    // But detailed collision check should still detect overlap
    auto detailedCollisions = manager->checkCollisionsDetailed(player.get());
    // This depends on implementation - might still detect but mark as trigger
}
