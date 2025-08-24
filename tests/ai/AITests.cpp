#include <gtest/gtest.h>
#include "ai/AIState.h"
#include "ai/Perception.h"
#include "ai/Pathfinding.h"
#include "ai/AISystem.h"
#include "ai/AIManager.h"
#include "entities/Entity.h"
#include "entities/Player.h"
#include "collisions/CollisionManager.h"

namespace ai {
namespace test {

// Mock entity for testing
class MockEntity : public entities::Entity {
public:
    MockEntity(Id id = 1, const sf::Vector2f& pos = {0, 0}, const sf::Vector2f& size = {32, 32})
        : Entity(id, pos, size) {}
    
    void update(float deltaTime) override {}
    void render(sf::RenderWindow& window) override {}
};

class AIStateTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AIStateTest, StateToStringConversions) {
    EXPECT_STREQ(stateToString(AIState::IDLE), "IDLE");
    EXPECT_STREQ(stateToString(AIState::PATROL), "PATROL");
    EXPECT_STREQ(stateToString(AIState::CHASE), "CHASE");
    EXPECT_STREQ(stateToString(AIState::ATTACK), "ATTACK");
    EXPECT_STREQ(stateToString(AIState::FLEE), "FLEE");
    EXPECT_STREQ(stateToString(AIState::RETURN), "RETURN");
    EXPECT_STREQ(stateToString(AIState::INVESTIGATE), "INVESTIGATE");
    EXPECT_STREQ(stateToString(AIState::ALERT), "ALERT");
    EXPECT_STREQ(stateToString(AIState::STUNNED), "STUNNED");
    EXPECT_STREQ(stateToString(AIState::DEAD), "DEAD");
}

TEST_F(AIStateTest, ProfileToStringConversions) {
    EXPECT_STREQ(profileToString(BehaviorProfile::AGGRESSIVE), "AGGRESSIVE");
    EXPECT_STREQ(profileToString(BehaviorProfile::DEFENSIVE), "DEFENSIVE");
    EXPECT_STREQ(profileToString(BehaviorProfile::NEUTRAL), "NEUTRAL");
    EXPECT_STREQ(profileToString(BehaviorProfile::PASSIVE), "PASSIVE");
    EXPECT_STREQ(profileToString(BehaviorProfile::GUARD), "GUARD");
    EXPECT_STREQ(profileToString(BehaviorProfile::SCOUT), "SCOUT");
}

class PerceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.sightRange = 100.0f;
        config_.sightAngle = 60.0f;
        config_.hearingRange = 80.0f;
        config_.proximityRange = 30.0f;
        config_.memoryDuration = 5.0f;
        
        perceptionSystem_ = std::make_unique<PerceptionSystem>(config_);
    }
    
    PerceptionConfig config_;
    std::unique_ptr<PerceptionSystem> perceptionSystem_;
};

TEST_F(PerceptionTest, BasicSightDetection) {
    sf::Vector2f observerPos(0, 0);
    sf::Vector2f observerFacing(1, 0);  // Facing right
    sf::Vector2f targetPos(50, 0);      // Target in front
    
    bool canSee = perceptionSystem_->canSee(observerPos, observerFacing, targetPos, nullptr);
    EXPECT_TRUE(canSee);
}

TEST_F(PerceptionTest, SightDetectionOutOfRange) {
    sf::Vector2f observerPos(0, 0);
    sf::Vector2f observerFacing(1, 0);
    sf::Vector2f targetPos(150, 0);     // Target too far
    
    bool canSee = perceptionSystem_->canSee(observerPos, observerFacing, targetPos, nullptr);
    EXPECT_FALSE(canSee);
}

TEST_F(PerceptionTest, SightDetectionOutOfCone) {
    sf::Vector2f observerPos(0, 0);
    sf::Vector2f observerFacing(1, 0);  // Facing right
    sf::Vector2f targetPos(0, 50);      // Target to the side (90 degrees)
    
    bool canSee = perceptionSystem_->canSee(observerPos, observerFacing, targetPos, nullptr);
    EXPECT_FALSE(canSee);
}

TEST_F(PerceptionTest, HearingDetection) {
    sf::Vector2f observerPos(0, 0);
    sf::Vector2f soundPos(50, 0);
    
    bool canHear = perceptionSystem_->canHear(observerPos, soundPos);
    EXPECT_TRUE(canHear);
    
    sf::Vector2f farSoundPos(100, 0);
    bool canHearFar = perceptionSystem_->canHear(observerPos, farSoundPos);
    EXPECT_FALSE(canHearFar);
}

TEST_F(PerceptionTest, ProximityDetection) {
    sf::Vector2f observerPos(0, 0);
    sf::Vector2f closePos(20, 0);
    
    bool inProximity = perceptionSystem_->isInProximity(observerPos, closePos);
    EXPECT_TRUE(inProximity);
    
    sf::Vector2f farPos(50, 0);
    bool farProximity = perceptionSystem_->isInProximity(observerPos, farPos);
    EXPECT_FALSE(farProximity);
}

TEST_F(PerceptionTest, MemorySystem) {
    MockEntity observer(1);
    sf::Vector2f memoryPos(100, 100);
    float currentTime = 0.0f;
    
    // Add memory
    perceptionSystem_->addMemory(&observer, memoryPos, currentTime);
    
    // Check if memory is valid
    EXPECT_TRUE(perceptionSystem_->hasValidMemory(&observer, currentTime + 1.0f));
    EXPECT_FALSE(perceptionSystem_->hasValidMemory(&observer, currentTime + 10.0f)); // After duration
    
    // Check position retrieval
    sf::Vector2f retrievedPos = perceptionSystem_->getLastKnownPosition(&observer, currentTime + 1.0f);
    EXPECT_FLOAT_EQ(retrievedPos.x, memoryPos.x);
    EXPECT_FLOAT_EQ(retrievedPos.y, memoryPos.y);
    
    // Clear memory
    perceptionSystem_->clearMemory(&observer);
    EXPECT_FALSE(perceptionSystem_->hasValidMemory(&observer, currentTime + 1.0f));
}

class PathfindingTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.gridSize = 32.0f;
        config_.allowDiagonal = true;
        
        pathfindingSystem_ = std::make_unique<PathfindingSystem>(config_);
    }
    
    PathfindingConfig config_;
    std::unique_ptr<PathfindingSystem> pathfindingSystem_;
};

TEST_F(PathfindingTest, GridConversion) {
    sf::Vector2f worldPos(100, 150);
    sf::Vector2f gridPos = pathfindingSystem_->worldToGrid(worldPos);
    
    EXPECT_FLOAT_EQ(gridPos.x, 3.0f);  // 100 / 32 = 3.125, floored to 3
    EXPECT_FLOAT_EQ(gridPos.y, 4.0f);  // 150 / 32 = 4.6875, floored to 4
    
    sf::Vector2f backToWorld = pathfindingSystem_->gridToWorld(gridPos);
    EXPECT_FLOAT_EQ(backToWorld.x, 112.0f);  // 3 * 32 + 16 = 112
    EXPECT_FLOAT_EQ(backToWorld.y, 144.0f);  // 4 * 32 + 16 = 144
}

TEST_F(PathfindingTest, DirectPathClear) {
    sf::Vector2f start(0, 0);
    sf::Vector2f goal(100, 100);
    
    // Without collision manager, path should be clear
    bool isClear = pathfindingSystem_->isPathClear(start, goal, nullptr);
    EXPECT_TRUE(isClear);
}

TEST_F(PathfindingTest, SimplePath) {
    sf::Vector2f start(0, 0);
    sf::Vector2f goal(100, 100);
    
    auto path = pathfindingSystem_->findSimplePath(start, goal, nullptr);
    EXPECT_GE(path.size(), 2u);  // At least start and goal
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

class AIAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        entity_ = std::make_unique<MockEntity>(1, sf::Vector2f(100, 100));
        
        AIAgentConfig config;
        config.profile = BehaviorProfile::NEUTRAL;
        config.speed = 100.0f;
        config.attackRange = 32.0f;
        
        agent_ = std::make_unique<AIAgent>(entity_.get(), config);
    }
    
    std::unique_ptr<MockEntity> entity_;
    std::unique_ptr<AIAgent> agent_;
};

TEST_F(AIAgentTest, InitialState) {
    EXPECT_EQ(agent_->getCurrentState(), AIState::IDLE);
    EXPECT_EQ(agent_->getProfile(), BehaviorProfile::NEUTRAL);
}

TEST_F(AIAgentTest, StateTransitions) {
    agent_->changeState(AIState::PATROL, "Starting patrol");
    EXPECT_EQ(agent_->getCurrentState(), AIState::PATROL);
    
    agent_->changeState(AIState::CHASE, "Target detected");
    EXPECT_EQ(agent_->getCurrentState(), AIState::CHASE);
}

TEST_F(AIAgentTest, PatrolPoints) {
    std::vector<sf::Vector2f> patrolPoints = {
        {50, 50}, {150, 50}, {150, 150}, {50, 150}
    };
    
    agent_->setPatrolPoints(patrolPoints);
    EXPECT_EQ(agent_->getPatrolPoints().size(), 4u);
    
    agent_->addPatrolPoint({200, 200});
    EXPECT_EQ(agent_->getPatrolPoints().size(), 5u);
}

TEST_F(AIAgentTest, TargetManagement) {
    MockEntity target1(2, sf::Vector2f(200, 200));
    MockEntity target2(3, sf::Vector2f(300, 300));
    
    agent_->addTarget(&target1, Priority::MEDIUM);
    agent_->addTarget(&target2, Priority::HIGH);
    
    EXPECT_EQ(agent_->getAllTargets().size(), 2u);
    EXPECT_EQ(agent_->getPrimaryTarget(), &target2);  // Higher priority
    
    agent_->removeTarget(&target2);
    EXPECT_EQ(agent_->getAllTargets().size(), 1u);
    EXPECT_EQ(agent_->getPrimaryTarget(), &target1);
    
    agent_->clearTargets();
    EXPECT_EQ(agent_->getAllTargets().size(), 0u);
    EXPECT_EQ(agent_->getPrimaryTarget(), nullptr);
}

TEST_F(AIAgentTest, DebugInfo) {
    auto debugInfo = agent_->getDebugInfo();
    EXPECT_EQ(debugInfo.currentState, AIState::IDLE);
    EXPECT_EQ(debugInfo.profile, BehaviorProfile::NEUTRAL);
    EXPECT_GE(debugInfo.timeInCurrentState, 0.0f);
}

class AIManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        CoordinationConfig config;
        config.enableCoordination = true;
        config.alertRadius = 100.0f;
        
        manager_ = std::make_unique<AIManager>(config);
        
        entity1_ = std::make_unique<MockEntity>(1, sf::Vector2f(100, 100));
        entity2_ = std::make_unique<MockEntity>(2, sf::Vector2f(200, 200));
    }
    
    std::unique_ptr<AIManager> manager_;
    std::unique_ptr<MockEntity> entity1_;
    std::unique_ptr<MockEntity> entity2_;
};

TEST_F(AIManagerTest, AgentManagement) {
    AIAgentConfig config;
    config.profile = BehaviorProfile::AGGRESSIVE;
    
    manager_->addAgent(entity1_.get(), config);
    EXPECT_NE(manager_->getAgent(entity1_.get()), nullptr);
    
    manager_->addAgent(entity2_.get(), config);
    auto metrics = manager_->getPerformanceMetrics();
    EXPECT_EQ(metrics.totalAgents, 2);
    
    manager_->removeAgent(entity1_.get());
    EXPECT_EQ(manager_->getAgent(entity1_.get()), nullptr);
    
    manager_->clearAllAgents();
    metrics = manager_->getPerformanceMetrics();
    EXPECT_EQ(metrics.totalAgents, 0);
}

TEST_F(AIManagerTest, EventBroadcasting) {
    AIAgentConfig config;
    manager_->addAgent(entity1_.get(), config);
    manager_->addAgent(entity2_.get(), config);
    
    // Test damage event
    manager_->onEntityDamaged(entity1_.get(), 10.0f, entity2_.get());
    
    // Test sound event
    manager_->onSoundMade(sf::Vector2f(150, 150), 0.8f, entity1_.get());
    
    // Test death event
    manager_->onEntityDied(entity1_.get());
    auto metrics = manager_->getPerformanceMetrics();
    EXPECT_EQ(metrics.totalAgents, 1);  // entity1 should be removed
}

TEST_F(AIManagerTest, Coordination) {
    AIAgentConfig config;
    manager_->addAgent(entity1_.get(), config);
    manager_->addAgent(entity2_.get(), config);
    
    // Test alert broadcasting
    manager_->alertAgentsInRadius(sf::Vector2f(150, 150), 100.0f, entity1_.get());
    
    // Test target sharing
    MockEntity target(3, sf::Vector2f(250, 250));
    manager_->shareTargetInformation(&target, target.position());
}

TEST_F(AIManagerTest, PerformanceMetrics) {
    AIAgentConfig config;
    manager_->addAgent(entity1_.get(), config);
    manager_->addAgent(entity2_.get(), config);
    
    auto metrics = manager_->getPerformanceMetrics();
    EXPECT_EQ(metrics.totalAgents, 2);
    EXPECT_GE(metrics.averageUpdateTime, 0.0f);
    
    manager_->resetPerformanceMetrics();
    metrics = manager_->getPerformanceMetrics();
    EXPECT_EQ(metrics.totalPerceptionChecks, 0);
}

TEST_F(AIManagerTest, DebugInfo) {
    AIAgentConfig config;
    manager_->addAgent(entity1_.get(), config);
    
    auto debugInfo = manager_->getDebugInfo();
    EXPECT_EQ(debugInfo.agentDebugInfo.size(), 1u);
    EXPECT_GE(debugInfo.performance.totalAgents, 1);
}

} // namespace test
} // namespace ai
