#include "ai/Enemy.h"
#include "core/Logger.h"
#include "entities/Player.h"
#include "../collisions/CollisionManager.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <cmath>
#include <sstream>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ai {

using core::Logger;

Enemy::Enemy(Id id, const sf::Vector2f& position, const sf::Vector2f& size, float speed, float visionRange, float attackRange, const std::vector<sf::Vector2f>& patrolPoints, BehaviorProfile profile)
    : entities::Entity(id, position, size)
    , speed_(speed)
    , visionRange_(visionRange)
    , attackRange_(attackRange)
    , behaviorProfile_(profile)
    , patrolPoints_(patrolPoints)
{
    // mark this entity as Enemy layer
    setCollisionLayer(entities::Entity::Layer::Enemy);
    shape_.setSize(size_);
    shape_.setFillColor(sf::Color::Red);
    shape_.setPosition(position_);
    
    // Initialize enhanced AI by default
    AIAgentConfig config;
    config.profile = profile;
    config.speed = speed;
    config.attackRange = attackRange;
    config.perception.sightRange = visionRange;
    
    aiAgent_ = std::make_unique<AIAgent>(this, config);
    aiAgent_->setPatrolPoints(patrolPoints);
    useEnhancedAI_ = true;
}

Enemy::~Enemy() = default;

void Enemy::setAIAgent(std::unique_ptr<AIAgent> agent) {
    aiAgent_ = std::move(agent);
    useEnhancedAI_ = aiAgent_ != nullptr;
}

void Enemy::setBehaviorProfile(BehaviorProfile profile) {
    behaviorProfile_ = profile;
    if (aiAgent_) {
        AIAgentConfig config = aiAgent_->getConfig();
        config.profile = profile;
        aiAgent_->setConfig(config);
    }
}

void Enemy::setPatrolPoints(const std::vector<sf::Vector2f>& points) {
    patrolPoints_ = points;
    if (aiAgent_) {
        aiAgent_->setPatrolPoints(points);
    }
}

void Enemy::addPatrolPoint(const sf::Vector2f& point) {
    patrolPoints_.push_back(point);
    if (aiAgent_) {
        aiAgent_->addPatrolPoint(point);
    }
}

void Enemy::onDamageReceived(float damage, entities::Entity* source) {
    if (aiAgent_) {
        aiAgent_->onDamageReceived(damage, source);
    }
}

void Enemy::onSoundHeard(const sf::Vector2f& soundPosition, float intensity) {
    if (aiAgent_) {
        aiAgent_->onSoundHeard(soundPosition, intensity);
    }
}

// Legacy state methods for backward compatibility
const char* Enemy::legacyStateToString(AIState s) {
    return stateToString(s);
}

void Enemy::changeState(AIState newState) {
    if (useEnhancedAI_ && aiAgent_) {
        aiAgent_->changeState(newState);
    } else {
        if (newState == legacyState_) return;
        
        // Only log transitions if cooldown expired
        if (logTimer_ <= 0.f) {
            std::ostringstream ss;
            ss << "[AI] Enemy " << id_ << " -> " << legacyStateToString(newState);
            Logger::instance().info(ss.str());
            logTimer_ = logCooldown_;
        }
        
        // If entering patrol or return, pick the nearest patrol point as the target
        if ((newState == AIState::PATROL || newState == AIState::RETURN) && !patrolPoints_.empty()) {
            currentPatrolIndex_ = findNearestPatrolIndex();
        }
        legacyState_ = newState;
    }
}

AIState Enemy::getCurrentState() const {
    if (useEnhancedAI_ && aiAgent_) {
        return aiAgent_->getCurrentState();
    }
    return legacyState_;
}

bool Enemy::detectPlayer(const sf::Vector2f& playerPos) const {
    sf::Vector2f d = playerPos - position_;
    float dist2 = d.x * d.x + d.y * d.y;
    if (dist2 > (visionRange_ * visionRange_)) return false;
    
    // If collisionManager_ is available, ensure walls do not block vision
    if (collisionManager_) {
        sf::Vector2f a = position_ + (size_ * 0.5f);
        sf::Vector2f b = playerPos + (size_ * 0.5f);
        if (collisionManager_->segmentIntersectsAny(a, b, const_cast<ai::Enemy*>(this), entities::kLayerMaskWall)) return false;
    }
    return true;
}

// Convenience overload that checks the stored targetPlayer_
bool Enemy::detectPlayer() const {
    if (!targetPlayer_) return false;
    return detectPlayer(targetPlayer_->position());
}

std::size_t Enemy::findNearestPatrolIndex() const {
    if (patrolPoints_.empty()) return 0u;
    std::size_t best = 0u;
    float bestDist2 = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < patrolPoints_.size(); ++i) {
        sf::Vector2f d = patrolPoints_[i] - position_;
        float dist2 = d.x*d.x + d.y*d.y;
        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            best = i;
        }
    }
    return best;
}

void Enemy::moveTowards(const sf::Vector2f& dst, float dt) {
    sf::Vector2f dir = dst - position_;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len <= 0.0001f) return;
    dir /= len;
    // update facing direction
    facingDir_ = dir;
    sf::Vector2f delta = dir * (speed_ * dt);
    // produce intended move (collision handled by external systems)
    intendedPosition_ = position_ + delta;
}

void Enemy::moveTowards(const sf::Vector2f& dst, float dt, collisions::CollisionManager* collisionManager) {
    // compute intended delta
    sf::Vector2f dir = dst - position_;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len <= 0.0001f) return;
    dir /= len;
    // update facing direction
    facingDir_ = dir;
    sf::Vector2f delta = dir * (speed_ * dt);
    sf::Vector2f intended = position_ + delta;

    // Build bounds for intended position (use project's FloatRect layout)
    sf::FloatRect testBounds;
    testBounds.position.x = intended.x;
    testBounds.position.y = intended.y;
    testBounds.size.x = size_.x;
    testBounds.size.y = size_.y;

    if (!collisionManager) {
        intendedPosition_ = intended;
        return;
    }

    // Exclude items from blocking enemy movement
    auto blocker = collisionManager->firstColliderForBounds(testBounds, this, entities::kLayerMaskAll & ~entities::kLayerMaskItem);
    if (!blocker) {
        intendedPosition_ = intended;
        return;
    }

    // If blocked, try sliding on X axis alone
    sf::Vector2f slideX = position_ + sf::Vector2f(delta.x, 0.f);
    sf::FloatRect tbx;
    tbx.position.x = slideX.x; tbx.position.y = slideX.y;
    tbx.size.x = size_.x; tbx.size.y = size_.y;
    if (!collisionManager->firstColliderForBounds(tbx, this, entities::kLayerMaskAll & ~entities::kLayerMaskItem)) {
        intendedPosition_ = slideX;
        return;
    }

    // Try sliding on Y axis alone
    sf::Vector2f slideY = position_ + sf::Vector2f(0.f, delta.y);
    sf::FloatRect tby;
    tby.position.x = slideY.x; tby.position.y = slideY.y;
    tby.size.x = size_.x; tby.size.y = size_.y;
    if (!collisionManager->firstColliderForBounds(tby, this, entities::kLayerMaskAll & ~entities::kLayerMaskItem)) {
        intendedPosition_ = slideY;
        return;
    }

    // Otherwise stay in place (no move)
    intendedPosition_ = position_;
}

sf::Vector2f Enemy::computeIntendedMove(float deltaTime) const {
    // If intendedPosition_ is set by moveTowards this frame, return it.
    if (intendedPosition_ != sf::Vector2f{0.f, 0.f}) return intendedPosition_;
    // otherwise, no movement intended
    return position_;
}

void Enemy::commitMove(const sf::Vector2f& newPosition) {
    position_ = newPosition;
    shape_.setPosition(position_);
    // clear intended
    intendedPosition_ = sf::Vector2f{0.f, 0.f};
}

void Enemy::update(float deltaTime) {
    if (useEnhancedAI_ && aiAgent_) {
        // Use enhanced AI system
        // Note: This requires EntityManager and CollisionManager to be passed
        // For now, fall back to legacy system
        runLegacyFSM(deltaTime, nullptr);
    } else {
        // Use legacy FSM
        runLegacyFSM(deltaTime, nullptr);
    }
}

void Enemy::update(float deltaTime, const sf::Vector2f& playerPos) {
    if (useEnhancedAI_ && aiAgent_) {
        // Enhanced AI would be updated by AIManager, not here
        // For backward compatibility, still run legacy FSM
        runLegacyFSM(deltaTime, &playerPos);
    } else {
        runLegacyFSM(deltaTime, &playerPos);
    }
}

// Legacy FSM for backward compatibility
void Enemy::runLegacyFSM(float deltaTime, const sf::Vector2f* playerPos) {
    // Update log timer
    if (logTimer_ > 0.f) logTimer_ -= deltaTime;
    // Update attack timer
    if (attackTimer_ > 0.f) attackTimer_ -= deltaTime;

    // Basic FSM
    switch (legacyState_) {
        case AIState::IDLE:
        {
            // If has patrol points, start patrolling
            if (!patrolPoints_.empty()) {
                changeState(AIState::PATROL);
                break;
            }
            // otherwise remain idle but check for player
            if (playerPos ? detectPlayer(*playerPos) : detectPlayer()) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " detectó al jugador -> CHASE");
                changeState(AIState::CHASE);
            }
        }
        break;

        case AIState::PATROL:
        {
            if (playerPos ? detectPlayer(*playerPos) : detectPlayer()) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " detectó al jugador -> CHASE");
                changeState(AIState::CHASE);
                break;
            }
            if (patrolPoints_.empty()) {
                changeState(AIState::IDLE);
                break;
            }
            const sf::Vector2f& dest = patrolPoints_[currentPatrolIndex_];
            moveTowards(dest, deltaTime);
            sf::Vector2f toDest = dest - position_;
            if ((toDest.x*toDest.x + toDest.y*toDest.y) < 4.0f) {
                currentPatrolIndex_ = (currentPatrolIndex_ + 1) % patrolPoints_.size();
            }
        }
        break;

        case AIState::CHASE:
        {
            if (!targetPlayer_) {
                changeState(AIState::RETURN);
                break;
            }
            // Determine the player position to use for chasing/detection
            sf::Vector2f ppos = playerPos ? *playerPos : (targetPlayer_ ? targetPlayer_->position() : position_);
            if (! (playerPos ? detectPlayer(ppos) : detectPlayer()) ) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " retornando a patrulla..");
                changeState(AIState::RETURN);
                break;
            }
            // move to player
            moveTowards(ppos, deltaTime);
            // check attack range
            sf::Vector2f d = ppos - position_;
            float dist2 = d.x*d.x + d.y*d.y;
            if (dist2 <= (attackRange_ * attackRange_)) {
                // placeholder attack
                std::ostringstream ss;
                ss << "[AI] Enemy " << id_ << " alcanzó al jugador -> ATTACK";
                Logger::instance().info(ss.str());
                changeState(AIState::ATTACK);
            }
        }
        break;

        case AIState::ATTACK:
        {
            // placeholder: after attack, try to chase again (or switch to RETURN if player gone)
            if (targetPlayer_) {
                // perform attack if cooldown expired
                if (attackTimer_ <= 0.f) {
                    attack(*targetPlayer_);
                    attackTimer_ = attackCooldown_;
                }
            }
            if (playerPos ? !detectPlayer(*playerPos) : !detectPlayer()) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " retornando a patrulla..");
                changeState(AIState::RETURN);
            } else {
                // for now just go back to CHASE to continue closing distance
                changeState(AIState::CHASE);
            }
        }
        break;

        case AIState::RETURN:
        {
            if (playerPos ? detectPlayer(*playerPos) : detectPlayer()) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " detectó al jugador -> CHASE");
                changeState(AIState::CHASE);
                break;
            }
            if (patrolPoints_.empty()) {
                changeState(AIState::IDLE);
                break;
            }
            // Move back to the nearest patrol point
            const sf::Vector2f& dest = patrolPoints_[currentPatrolIndex_];
            moveTowards(dest, deltaTime);
            sf::Vector2f toDest = dest - position_;
            if ((toDest.x*toDest.x + toDest.y*toDest.y) < 4.0f) {
                changeState(AIState::PATROL);
            }
        }
        break;

        default:
            break;
    }
}

void Enemy::render(sf::RenderWindow& window) {
    // Draw AI shape
    window.draw(shape_);

    // Debug vision cone (if enabled)
    // This could be moved to a debug visualization system
    if (legacyState_ == AIState::CHASE || legacyState_ == AIState::PATROL) {
        // Draw vision cone for debugging
        sf::ConvexShape visionCone;
        visionCone.setPointCount(3);
        sf::Vector2f center = position_ + size_ * 0.5f;
        visionCone.setPoint(0, center);
        
        float angleRad = std::atan2(facingDir_.y, facingDir_.x);
        float halfAngle = 32.5f * static_cast<float>(M_PI) / 180.0f; // 65 degrees / 2
        
        sf::Vector2f leftPoint = center + sf::Vector2f(
            visionRange_ * std::cos(angleRad - halfAngle),
            visionRange_ * std::sin(angleRad - halfAngle)
        );
        sf::Vector2f rightPoint = center + sf::Vector2f(
            visionRange_ * std::cos(angleRad + halfAngle),
            visionRange_ * std::sin(angleRad + halfAngle)
        );
        
        visionCone.setPoint(1, leftPoint);
        visionCone.setPoint(2, rightPoint);
        visionCone.setFillColor(visionFillColor_);
        visionCone.setOutlineColor(visionOutlineColor_);
        visionCone.setOutlineThickness(1.0f);
        
        window.draw(visionCone);
    }
}

void Enemy::attack(entities::Player& player) {
    if (attackTimer_ > 0.f) return; // Still on cooldown
    
    player.applyDamage(10); // Deal 10 damage
    attackTimer_ = attackCooldown_;
    
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " attacked player for 10 damage");
}

void Enemy::performMovementPlanning(float deltaTime, collisions::CollisionManager* collisionManager) {
    collisionManager_ = collisionManager;
    
    if (useEnhancedAI_ && aiAgent_) {
        // Enhanced AI handles its own movement planning
        // The movement would be handled through the AIAgent's pathfinding system
        return;
    }
    
    // Legacy movement planning
    // This is handled within the FSM moveTowards calls
}

} // namespace ai
