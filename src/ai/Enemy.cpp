#include "ai/Enemy.h"
#include "core/Logger.h"
#include "entities/Player.h"
#include "../collisions/CollisionManager.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <cmath>
#include <sstream>

namespace ai {

using core::Logger;

Enemy::Enemy(Id id, const sf::Vector2f& position, const sf::Vector2f& size, float speed, float visionRange, float attackRange, const std::vector<sf::Vector2f>& patrolPoints)
    : entities::Entity(id, position, size)
    , speed_(speed)
    , visionRange_(visionRange)
    , attackRange_(attackRange)
    , patrolPoints_(patrolPoints)
{
    shape_.setSize(size_);
    shape_.setFillColor(sf::Color::Red);
    shape_.setPosition(position_);
}

Enemy::~Enemy() = default;

const char* Enemy::stateToString(AIState s) {
    switch (s) {
        case AIState::IDLE: return "IDLE";
        case AIState::PATROL: return "PATROL";
        case AIState::CHASE: return "CHASE";
        case AIState::ATTACK: return "ATTACK";
        case AIState::RETURN: return "RETURN";
        default: return "UNKNOWN";
    }
}

bool Enemy::detectPlayer(const sf::Vector2f& playerPos) const {
    sf::Vector2f d = playerPos - position_;
    float dist2 = d.x * d.x + d.y * d.y;
    return dist2 <= (visionRange_ * visionRange_);
}

// Convenience overload that checks the stored targetPlayer_
bool Enemy::detectPlayer() const {
    if (!targetPlayer_) return false;
    return detectPlayer(targetPlayer_->position());
}

void Enemy::changeState(AIState newState) {
    if (newState == state_) return;
    // Only log transitions if cooldown expired
    if (logTimer_ <= 0.f) {
        std::ostringstream ss;
        ss << "[AI] Enemy " << id_ << " -> " << stateToString(newState);
        Logger::instance().info(ss.str());
        logTimer_ = logCooldown_;
    }
    // If entering patrol or return, pick the nearest patrol point as the target
    if ((newState == AIState::PATROL || newState == AIState::RETURN) && !patrolPoints_.empty()) {
        currentPatrolIndex_ = findNearestPatrolIndex();
    }
    state_ = newState;
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

    auto blocker = collisionManager->firstColliderForBounds(testBounds, this);
    if (!blocker) {
        intendedPosition_ = intended;
        return;
    }

    // If blocked, try sliding on X axis alone
    sf::Vector2f slideX = position_ + sf::Vector2f(delta.x, 0.f);
    sf::FloatRect tbx;
    tbx.position.x = slideX.x; tbx.position.y = slideX.y;
    tbx.size.x = size_.x; tbx.size.y = size_.y;
    if (!collisionManager->firstColliderForBounds(tbx, this)) {
        intendedPosition_ = slideX;
        return;
    }

    // Try sliding on Y axis alone
    sf::Vector2f slideY = position_ + sf::Vector2f(0.f, delta.y);
    sf::FloatRect tby;
    tby.position.x = slideY.x; tby.position.y = slideY.y;
    tby.size.x = size_.x; tby.size.y = size_.y;
    if (!collisionManager->firstColliderForBounds(tby, this)) {
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
    // Forward to core FSM without explicit player position (uses stored targetPlayer_)
    runFSM(deltaTime, nullptr);
}

void Enemy::update(float deltaTime, const sf::Vector2f& playerPos) {
    runFSM(deltaTime, &playerPos);
}

// Core FSM runner. playerPos pointer is optional; when provided it is used for detection
void Enemy::runFSM(float deltaTime, const sf::Vector2f* playerPos) {
    // Update log timer
    if (logTimer_ > 0.f) logTimer_ -= deltaTime;
    // Update attack timer
    if (attackTimer_ > 0.f) attackTimer_ -= deltaTime;

    // Basic FSM
    switch (state_) {
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

    // Debug: draw vision cone (65 degrees) in direction of facingDir_
    const float coneHalfAngleDeg = 32.5f; // half of 65 degrees
    const float coneAngleRad = 65.f * 3.14159265f / 180.f;
    const float half = coneAngleRad * 0.5f;
    sf::Vector2f center = position_ + (size_ * 0.5f);

    // Normalize facingDir_
    sf::Vector2f fd = facingDir_;
    float flen = std::sqrt(fd.x*fd.x + fd.y*fd.y);
    if (flen < 0.0001f) fd = sf::Vector2f(1.f, 0.f); else fd /= flen;

    // Compute cone triangle points: center, left, right
    // Rotate facing vector by +/- half-angle
    float cs = std::cos(half);
    float sn = std::sin(half);
    // left = R(-half) * fd, right = R(+half) * fd (rotation matrix)
    sf::Vector2f left = sf::Vector2f(fd.x * cs - fd.y * sn, fd.x * sn + fd.y * cs);
    sf::Vector2f right = sf::Vector2f(fd.x * cs + fd.y * sn, -fd.x * sn + fd.y * cs);
    left *= visionRange_;
    right *= visionRange_;

    sf::ConvexShape cone;
    cone.setPointCount(3);
    cone.setPoint(0, center);
    cone.setPoint(1, center + left);
    cone.setPoint(2, center + right);
    cone.setFillColor(visionFillColor_);
    cone.setOutlineColor(visionOutlineColor_);
    cone.setOutlineThickness(1.f);
    window.draw(cone);
}

void Enemy::performMovementPlanning(float deltaTime, collisions::CollisionManager* collisionManager) {
    // Decide destination based on state
    switch (state_) {
        case AIState::PATROL:
        case AIState::RETURN:
        {
            if (patrolPoints_.empty()) { intendedPosition_ = position_; return; }
            const sf::Vector2f& dest = patrolPoints_[currentPatrolIndex_];
            moveTowards(dest, deltaTime, collisionManager);
            break;
        }
        case AIState::CHASE:
        {
            if (!targetPlayer_) { intendedPosition_ = position_; return; }
            moveTowards(targetPlayer_->position(), deltaTime, collisionManager);
            break;
        }
        default:
            // Idle or default: no movement
            intendedPosition_ = position_;
            break;
    }
}

void Enemy::attack(entities::Player& player) {
    // check distance
    sf::Vector2f d = player.position() - position_;
    float dist2 = d.x*d.x + d.y*d.y;
    if (dist2 <= (attackRange_ * attackRange_)) {
        // simple fixed damage
    int damage = 10;
    player.applyDamage(damage);
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " atacó Player " + std::to_string(player.id()));
    }
}

} // namespace ai
