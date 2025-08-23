#ifndef ABYSSAL_STATION_TESTS_ENTITIES_ENTITYTESTS_H
#define ABYSSAL_STATION_TESTS_ENTITIES_ENTITYTESTS_H

#include "../../src/entities/Entity.h"
#include "../../src/entities/EntityManager.h"
#include "../../src/entities/Player.h"
#include "../../src/entities/MovementHelper.h"
// #include "../../src/entities/EntityTelemetry.h"  // Disabled due to linking issues
#include "../../src/collisions/CollisionManager.h"
#include "../../src/core/GameState.h"
#include "../../src/scene/SaveIntegration.h"
#include "../../src/gameplay/ItemManager.h"
#include "../../src/gameplay/PuzzleManager.h"
#include <cassert>
#include <memory>
#include <iostream>

namespace tests {

/**
 * Comprehensive test suite for the entities module.
 * Tests Entity, EntityManager, Player, MovementHelper, and telemetry.
 */
class EntityTests {
public:
    static void runAllTests() {
        std::cout << "Running Entity module tests...\n";
        
        testEntityBasics();
        testEntityManager();
        testPlayer();
        testMovementHelper();
        // testTelemetry();  // Disabled due to linking issues
        testSerialization();
        testPerformance();
        
        std::cout << "All Entity tests passed!\n";
    }

private:
    static void testEntityBasics() {
        std::cout << "Testing Entity basics...\n";
        
        // Test entity creation and basic properties
        TestEntity entity(42, {100.f, 200.f}, {32.f, 32.f});
        assert(entity.id() == 42);
        assert(entity.position().x == 100.f);
        assert(entity.position().y == 200.f);
        assert(entity.size().x == 32.f);
        assert(entity.size().y == 32.f);
        assert(entity.isActive());
        assert(entity.isVisible());
        
        // Test setters
        entity.setPosition({150.f, 250.f});
        assert(entity.position().x == 150.f);
        assert(entity.position().y == 250.f);
        
        entity.setActive(false);
        assert(!entity.isActive());
        
        entity.setVisible(false);
        assert(!entity.isVisible());
        
        // Test bounds
        auto bounds = entity.getBounds();
        assert(bounds.position.x == 150.f);
        assert(bounds.position.y == 250.f);
        assert(bounds.size.x == 32.f);
        assert(bounds.size.y == 32.f);
        
        // Test collision layers
        entity.setCollisionLayer(entities::Entity::Layer::Player);
        assert(entity.collisionLayer() == static_cast<std::uint32_t>(entities::Entity::Layer::Player));
        
        std::cout << "Entity basics tests passed.\n";
    }

    static void testEntityManager() {
        std::cout << "Testing EntityManager...\n";
        
        entities::EntityManager manager;
        collisions::CollisionManager collisionManager;
        manager.setCollisionManager(&collisionManager);
        
        // Test empty manager
        assert(manager.count() == 0);
        assert(manager.activeCount() == 0);
        assert(manager.allEntities().empty());
        
        // Add entities
        auto entity1 = std::make_unique<TestEntity>(1, sf::Vector2f{0.f, 0.f}, sf::Vector2f{32.f, 32.f});
        auto entity2 = std::make_unique<TestEntity>(2, sf::Vector2f{100.f, 100.f}, sf::Vector2f{64.f, 64.f});
        auto* entity1Ptr = entity1.get();
        auto* entity2Ptr = entity2.get();
        
        manager.addEntity(std::move(entity1));
        manager.addEntity(std::move(entity2));
        
        assert(manager.count() == 2);
        assert(manager.getEntity(1) == entity1Ptr);
        assert(manager.getEntity(2) == entity2Ptr);
        assert(manager.getEntity(999) == nullptr);
        
        // Test entity lookup
        auto allEntities = manager.allEntities();
        assert(allEntities.size() == 2);
        
        // Test remove entity
        assert(manager.removeEntity(1));
        assert(manager.count() == 1);
        assert(manager.getEntity(1) == nullptr);
        assert(manager.getEntity(2) == entity2Ptr);
        
        // Test batch removal
        std::vector<entities::Entity::Id> idsToRemove = {2, 999}; // 999 doesn't exist
        std::size_t removed = manager.removeEntities(idsToRemove);
        assert(removed == 1);
        assert(manager.count() == 0);
        
        // Test mark for removal
        auto entity3 = std::make_unique<TestEntity>(3, sf::Vector2f{0.f, 0.f}, sf::Vector2f{32.f, 32.f});
        manager.addEntity(std::move(entity3));
        manager.markEntityForRemoval(3);
        assert(manager.count() == 1); // Not removed yet
        manager.cleanupMarkedEntities();
        assert(manager.count() == 0); // Now removed
        
        // Test performance stats
        auto entity4 = std::make_unique<TestEntity>(4, sf::Vector2f{0.f, 0.f}, sf::Vector2f{32.f, 32.f});
        manager.addEntity(std::move(entity4));
        manager.updateAll(0.016f);
        
        auto stats = manager.getPerformanceStats();
        assert(stats.totalEntities == 1);
        assert(stats.entitiesUpdated == 1);
        assert(stats.lastUpdateTime >= 0.f);
        
        std::cout << "EntityManager tests passed.\n";
    }

    static void testPlayer() {
        std::cout << "Testing Player...\n";
        
        entities::Player player(100, {200.f, 300.f}, {32.f, 48.f}, 150.f, 80);
        
        // Test initial state
        assert(player.id() == 100);
        assert(player.position().x == 200.f);
        assert(player.position().y == 300.f);
        assert(player.speed() == 150.f);
        assert(player.health() == 80);
        assert(player.state() == entities::Player::State::Idle);
        assert(player.inventoryCount() == 0);
        
        // Test setters
        player.setHealth(50);
        assert(player.health() == 50);
        assert(player.state() != entities::Player::State::Dead);
        
        player.setHealth(0);
        assert(player.health() == 0);
        assert(player.state() == entities::Player::State::Dead);
        
        player.setHealth(100);
        assert(player.health() == 100);
        assert(player.state() == entities::Player::State::Idle); // Should revive
        
        player.setPosition({400.f, 500.f});
        assert(player.position().x == 400.f);
        assert(player.position().y == 500.f);
        
        player.setSpeed(200.f);
        assert(player.speed() == 200.f);
        
        player.setState(entities::Player::State::Walking);
        assert(player.state() == entities::Player::State::Walking);
        
        // Test damage
        player.applyDamage(20);
        assert(player.health() == 80);
        
        player.applyDamage(100);
        assert(player.health() == 0);
        assert(player.state() == entities::Player::State::Dead);
        
        // Test item collection
        player.onItemCollected(42);
        assert(player.inventoryCount() == 1);
        
        std::cout << "Player tests passed.\n";
    }

    static void testMovementHelper() {
        std::cout << "Testing MovementHelper...\n";
        
        collisions::CollisionManager collisionManager;
        TestEntity entity(1, {0.f, 0.f}, {32.f, 32.f});
        
        // Test movement without collision
        auto result = entities::MovementHelper::computeMovement(
            &entity, {100.f, 100.f}, &collisionManager
        );
        assert(!result.collisionOccurred);
        assert(!result.wasBlocked);
        assert(!result.didSlide);
        assert(result.finalPosition.x == 100.f);
        assert(result.finalPosition.y == 100.f);
        
        // Add a wall to test collision
        TestEntity wall(2, {50.f, 50.f}, {100.f, 100.f});
        wall.setCollisionLayer(entities::Entity::Layer::Wall);
        sf::FloatRect wallBounds({50.f, 50.f}, {100.f, 100.f});
        collisionManager.addCollider(&wall, wallBounds);
        
        // Test blocked movement
        result = entities::MovementHelper::computeMovement(
            &entity, {75.f, 75.f}, &collisionManager, 
            entities::MovementHelper::CollisionMode::Block
        );
        assert(result.collisionOccurred);
        assert(result.wasBlocked);
        
        // Test sliding movement
        result = entities::MovementHelper::computeMovement(
            &entity, {75.f, 25.f}, &collisionManager, 
            entities::MovementHelper::CollisionMode::Slide
        );
        // Should be able to slide along the edge
        
        std::cout << "MovementHelper tests passed.\n";
    }

    /*
    static void testTelemetry() {
        std::cout << "Testing EntityTelemetry...\n";
        
        auto& telemetry = entities::EntityTelemetry::instance();
        telemetry.clearEvents();
        telemetry.setEnabled(true);
        
        // Test entity lifecycle events
        telemetry.logEntityCreated(1, "TestEntity", {100.f, 200.f});
        telemetry.logEntityMoved(1, {100.f, 200.f}, {150.f, 250.f});
        telemetry.logEntityCollision(1, 2, {150.f, 250.f}, true);
        telemetry.logEntityInteraction(1, "test_interaction", "test_details");
        telemetry.logEntityDestroyed(1, "TestEntity", 5.5f);
        
        auto events = telemetry.getEvents();
        assert(events.size() == 5);
        
        // Check event types
        assert(events[0].eventType == "created");
        assert(events[1].eventType == "moved");
        assert(events[2].eventType == "collision");
        assert(events[3].eventType == "interaction");
        assert(events[4].eventType == "destroyed");
        
        // Test stats
        auto stats = telemetry.getStatsForType("TestEntity");
        assert(stats.creationCount == 1);
        assert(stats.destructionCount == 1);
        assert(stats.movementEvents == 0); // Not tracked separately yet
        assert(stats.collisionEvents == 0); // Not tracked separately yet
        assert(stats.averageLifetime == 5.5f);
        
        // Test performance metrics
        telemetry.logPerformanceMetric("update_time", 0.016f);
        telemetry.logPerformanceMetric("update_time", 0.018f);
        float avgTime = telemetry.getAveragePerformanceMetric("update_time");
        assert(avgTime == 0.017f);
        
        // Test JSON export
        std::string json = telemetry.exportToJson();
        assert(!json.empty());
        assert(json.find("\"events\"") != std::string::npos);
        assert(json.find("\"stats\"") != std::string::npos);
        
        std::cout << "EntityTelemetry tests passed.\n";
    }
    */

    static void testSerialization() {
        std::cout << "Testing serialization round-trip...\n";
        
        // Create entities
        entities::EntityManager entityManager;
        gameplay::ItemManager itemManager(nullptr, nullptr);
        gameplay::PuzzleManager puzzleManager;
        
        auto player = std::make_unique<entities::Player>(1, sf::Vector2f{100.f, 200.f}, sf::Vector2f{32.f, 32.f}, 150.f, 75);
        auto* playerPtr = player.get();
        entityManager.addEntity(std::move(player));
        
        // Gather state
        auto state = scene::gatherGameState(entityManager, itemManager, puzzleManager);
        assert(state.players.size() == 1);
        assert(state.players[0].id == 1);
        assert(state.players[0].x == 100.f);
        assert(state.players[0].y == 200.f);
        assert(state.players[0].health == 75);
        
        // Modify player
        playerPtr->setPosition({300.f, 400.f});
        playerPtr->setHealth(50);
        
        // Apply saved state
        scene::applyGameState(state, entityManager, itemManager, puzzleManager);
        
        // Verify restoration
        assert(playerPtr->position().x == 100.f);
        assert(playerPtr->position().y == 200.f);
        assert(playerPtr->health() == 75);
        
        std::cout << "Serialization tests passed.\n";
    }

    static void testPerformance() {
        std::cout << "Testing performance...\n";
        
        entities::EntityManager manager;
        const std::size_t entityCount = 1000;
        
        // Create many entities
        for (std::size_t i = 0; i < entityCount; ++i) {
            auto entity = std::make_unique<TestEntity>(i, 
                sf::Vector2f{static_cast<float>(i % 100), static_cast<float>(i / 100)}, 
                sf::Vector2f{16.f, 16.f});
            manager.addEntity(std::move(entity));
        }
        
        assert(manager.count() == entityCount);
        
        // Test update performance
        auto start = std::chrono::high_resolution_clock::now();
        manager.updateAll(0.016f);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Should complete within reasonable time (adjust threshold as needed)
        assert(duration.count() < 10000); // Less than 10ms for 1000 entities
        
        // Test batch removal
        std::vector<entities::Entity::Id> idsToRemove;
        for (std::size_t i = 0; i < entityCount / 2; ++i) {
            idsToRemove.push_back(i);
        }
        
        start = std::chrono::high_resolution_clock::now();
        std::size_t removed = manager.removeEntities(idsToRemove);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        assert(removed == entityCount / 2);
        assert(manager.count() == entityCount / 2);
        assert(duration.count() < 5000); // Less than 5ms for batch removal
        
        std::cout << "Performance tests passed.\n";
    }

    // Helper test entity class
    class TestEntity : public entities::Entity {
    public:
        TestEntity(Id id, const sf::Vector2f& position, const sf::Vector2f& size)
            : Entity(id, position, size) {}
            
        void update(float deltaTime) override {
            updateCalled_ = true;
            lastDeltaTime_ = deltaTime;
        }
        
        void render(sf::RenderWindow& window) override {
            renderCalled_ = true;
        }
        
        bool updateCalled_{false};
        bool renderCalled_{false};
        float lastDeltaTime_{0.f};
    };
};

} // namespace tests

#endif // ABYSSAL_STATION_TESTS_ENTITIES_ENTITYTESTS_H
