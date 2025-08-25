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
    
    // Initialize advanced AI variables
    lastKnownPlayerPosition_ = sf::Vector2f(0.f, 0.f);
    timeSinceLastSighting_ = 0.f;
    memoryDuration_ = 30.0f;
    alertLevel_ = 0.0f;
    isStalkMode_ = false;
    stalkTimer_ = 0.0f;
    stalkDistance_ = 100.0f;
    
    // Initialize perception capabilities
    hearingRange_ = 200.0f;
    vibrationRange_ = 50.0f;
    lightDetectionRange_ = 150.0f;
    communicationRange_ = 300.0f;
    
    // Initialize attack variations
    baseDamage_ = 25;
    criticalChance_ = 0.1f;
    ambushDamageMultiplier_ = 2.0f;
    
    // Initialize movement prediction
    playerMovementHistory_.reserve(maxHistorySize_);
    predictionAccuracy_ = 0.7f;
    
    // Initialize psychological effects
    psychologicalDamage_ = 25.0f;
    intimidationRadius_ = 80.0f;
    batteryDrainRate_ = 1.0f;
    
    // Initialize communication
    lastCommunicationTime_ = 0.0f;
    sharedInformation_.reserve(10);
    hasEscalatedAlert_ = false;
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
    
    // Update advanced AI timers
    timeSinceLastSighting_ += deltaTime;
    stalkTimer_ += deltaTime;
    lastCommunicationTime_ += deltaTime;
    
    // Decay alert level over time
    alertLevel_ = std::max(0.0f, alertLevel_ - deltaTime * 0.1f);
    
    // Multi-modal player detection if player reference is available
    bool playerDetected = false;
    if (targetPlayer_) {
        sf::Vector2f playerPos = targetPlayer_->position();
        
        // Calculate noise level based on player movement
        float noiseLevel = 0.0f;
        if (targetPlayer_->isRunning()) {
            noiseLevel = 1.0f; // Maximum noise when running
        } else if (targetPlayer_->isMoving()) {
            noiseLevel = 0.4f; // Medium noise when walking
        } else {
            noiseLevel = 0.1f; // Minimal noise when standing
        }
        
        playerDetected = detectPlayerBySight(playerPos) ||
                        detectPlayerBySound(playerPos, noiseLevel) ||
                        detectPlayerByVibration(playerPos) ||
                        detectPlayerByLight(playerPos, targetPlayer_->isFlashlightOn());
        
        if (playerDetected) {
            rememberPlayerPosition(playerPos);
            alertLevel_ = std::min(1.0f, alertLevel_ + deltaTime * 0.5f);
        }
    } else if (playerPos) {
        // Fallback to legacy detection
        playerDetected = detectPlayer(*playerPos);
        if (playerDetected) {
            rememberPlayerPosition(*playerPos);
        }
    } else {
        playerDetected = detectPlayer();
        if (playerDetected && targetPlayer_) {
            rememberPlayerPosition(targetPlayer_->position());
        }
    }

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
            if (playerDetected) {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " detectó al jugador -> CHASE");
                changeState(AIState::CHASE);
            }
        }
        break;

        case AIState::PATROL:
        {
            if (playerDetected) {
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
            
            if (!playerDetected) {
                // Lost player, try to predict their movement or investigate
                if (hasRecentPlayerMemory() && alertLevel_ > 0.5f) {
                    changeState(AIState::PREDICT_MOVEMENT);
                } else if (hasRecentPlayerMemory()) {
                    changeState(AIState::INVESTIGATE_NOISE);
                } else {
                    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " retornando a patrulla..");
                    changeState(AIState::RETURN);
                }
                break;
            }
            
            // Alert nearby enemies if haven't communicated recently
            if (lastCommunicationTime_ > 3.0f && alertLevel_ > 0.3f) {
                changeState(AIState::COMMUNICATE);
                break;
            }
            
            // Decide between direct chase, stalking, or ambush based on alert level
            if (alertLevel_ > 0.8f) {
                // High alert: aggressive chase
                moveTowards(ppos, deltaTime);
            } else if (alertLevel_ > 0.4f && stalkTimer_ < 15.0f) {
                // Medium alert: stalk the player
                enterStalkMode(ppos);
                changeState(AIState::STALK);
                break;
            } else {
                // Low alert: normal chase
                moveTowards(ppos, deltaTime);
            }
            
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
                    // Choose attack type based on alert level and stalking mode
                    if (isStalkMode_) {
                        performPsychologicalAttack(*targetPlayer_);
                    } else if (alertLevel_ > 0.7f) {
                        performAmbushAttack(*targetPlayer_);
                    } else {
                        performPhysicalAttack(*targetPlayer_);
                    }
                    
                    // Alert nearby enemies if not already done
                    if (lastCommunicationTime_ > 2.0f) {
                        alertNearbyEnemies(targetPlayer_->position());
                    }
                }
            }
            if (playerDetected) {
                // Continue chasing if player still detected
                changeState(AIState::CHASE);
            } else if (hasRecentPlayerMemory()) {
                // Switch to investigation if recently lost player
                changeState(AIState::INVESTIGATE_NOISE);
            } else {
                Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " retornando a patrulla..");
                changeState(AIState::RETURN);
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

        // Advanced AI states
        case AIState::INVESTIGATE_NOISE:
        {
            if (playerDetected) {
                changeState(AIState::CHASE);
                break;
            }
            
            // Move toward last known player position
            if (hasRecentPlayerMemory()) {
                moveTowards(lastKnownPlayerPosition_, deltaTime);
                sf::Vector2f toTarget = lastKnownPlayerPosition_ - position_;
                if ((toTarget.x*toTarget.x + toTarget.y*toTarget.y) < 16.0f) {
                    // Reached investigation point, start searching
                    changeState(AIState::SEARCH_LAST_KNOWN);
                }
            } else {
                changeState(AIState::RETURN);
            }
        }
        break;

        case AIState::STALK:
        {
            if (targetPlayer_) {
                // Maintain distance while following player
                sf::Vector2f playerPos = targetPlayer_->position();
                sf::Vector2f toPlayer = playerPos - position_;
                float distance = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y);
                
                if (distance > stalkDistance_ + 20.0f) {
                    // Too far, move closer
                    moveTowards(playerPos, deltaTime);
                } else if (distance < stalkDistance_ - 20.0f) {
                    // Too close, move away
                    sf::Vector2f awayFromPlayer = position_ - playerPos;
                    float len = std::sqrt(awayFromPlayer.x*awayFromPlayer.x + awayFromPlayer.y*awayFromPlayer.y);
                    if (len > 0.001f) {
                        awayFromPlayer /= len;
                        intendedPosition_ = position_ + awayFromPlayer * speed_ * deltaTime;
                    }
                }
                
                // Perform psychological attacks
                performPsychologicalAttack(*targetPlayer_);
                
                // Occasionally generate ambient noise
                if (static_cast<int>(stalkTimer_) % 5 == 0 && stalkTimer_ - deltaTime < static_cast<int>(stalkTimer_)) {
                    generateAmbientNoise();
                }
                
                // Transition to direct attack if player gets too close or after stalking too long
                if (distance < 50.0f || stalkTimer_ > 30.0f) {
                    changeState(AIState::ATTACK);
                }
            } else {
                changeState(AIState::RETURN);
            }
        }
        break;

        case AIState::AMBUSH:
        {
            if (targetPlayer_) {
                sf::Vector2f playerPos = targetPlayer_->position();
                
                // Calculate player velocity from movement history
                sf::Vector2f playerVelocity(0.f, 0.f);
                if (playerMovementHistory_.size() >= 2) {
                    sf::Vector2f lastPos = playerMovementHistory_.back();
                    sf::Vector2f secondLastPos = playerMovementHistory_[playerMovementHistory_.size() - 2];
                    playerVelocity = lastPos - secondLastPos;
                }
                
                sf::Vector2f predictedPos = predictPlayerMovement(playerPos, playerVelocity);
                
                // Move to intercept position
                moveTowards(predictedPos, deltaTime);
                
                sf::Vector2f toPlayer = playerPos - position_;
                float distance = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y);
                
                if (distance <= attackRange_) {
                    performAmbushAttack(*targetPlayer_);
                    changeState(AIState::CHASE);
                }
            } else {
                changeState(AIState::RETURN);
            }
        }
        break;

        case AIState::COMMUNICATE:
        {
            // Alert nearby enemies about player position
            if (targetPlayer_) {
                alertNearbyEnemies(targetPlayer_->position());
            }
            
            // Return to previous behavior after communication
            changeState(AIState::CHASE);
        }
        break;

        case AIState::SEARCH_LAST_KNOWN:
        {
            if (playerDetected) {
                changeState(AIState::CHASE);
                break;
            }
            
            // Search pattern around last known position
            static float searchTimer = 0.0f;
            searchTimer += deltaTime;
            
            if (searchTimer >= 5.0f) {
                searchTimer = 0.0f;
                changeState(AIState::RETURN);
            }
            
            // Simple circular search pattern
            float searchRadius = 50.0f;
            float angle = searchTimer * 2.0f; // 2 radians per second
            sf::Vector2f searchTarget = lastKnownPlayerPosition_ + sf::Vector2f(
                searchRadius * std::cos(angle),
                searchRadius * std::sin(angle)
            );
            
            moveTowards(searchTarget, deltaTime);
        }
        break;

        case AIState::ESCALATE_ALERT:
        {
            // Increase alert level and communicate with nearby enemies
            escalateAlert();
            
            if (targetPlayer_) {
                alertNearbyEnemies(targetPlayer_->position());
                changeState(AIState::CHASE);
            } else {
                changeState(AIState::INVESTIGATE_NOISE);
            }
        }
        break;

        case AIState::PREDICT_MOVEMENT:
        {
            if (targetPlayer_) {
                sf::Vector2f playerPos = targetPlayer_->position();
                
                // Calculate player velocity from movement history
                sf::Vector2f playerVelocity(0.f, 0.f);
                if (playerMovementHistory_.size() >= 2) {
                    sf::Vector2f lastPos = playerMovementHistory_.back();
                    sf::Vector2f secondLastPos = playerMovementHistory_[playerMovementHistory_.size() - 2];
                    playerVelocity = lastPos - secondLastPos;
                }
                
                sf::Vector2f predictedPos = predictPlayerMovement(playerPos, playerVelocity);
                moveTowards(predictedPos, deltaTime);
                
                // Switch to direct chase if prediction seems wrong
                sf::Vector2f actualPos = targetPlayer_->position();
                sf::Vector2f toPredicted = predictedPos - position_;
                sf::Vector2f toActual = actualPos - position_;
                
                if (std::sqrt(toPredicted.x*toPredicted.x + toPredicted.y*toPredicted.y) >
                    std::sqrt(toActual.x*toActual.x + toActual.y*toActual.y)) {
                    changeState(AIState::CHASE);
                }
            } else {
                changeState(AIState::RETURN);
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

// =================================================================
// ADVANCED AI METHODS IMPLEMENTATION
// =================================================================

// Multi-modal perception methods
bool Enemy::detectPlayerBySight(const sf::Vector2f& playerPosition) const {
    sf::Vector2f enemyPos = position_;
    float distance = std::sqrt(std::pow(playerPosition.x - enemyPos.x, 2) + std::pow(playerPosition.y - enemyPos.y, 2));
    
    // Check if player is within sight range
    if (distance > visionRange_) return false;
    
    // Simple line-of-sight check (could be enhanced with actual obstacle detection)
    // For now, assume clear line of sight within range
    return true;
}

bool Enemy::detectPlayerBySound(const sf::Vector2f& playerPos, float noiseLevel) const {
    sf::Vector2f enemyPos = position_;
    float distance = std::sqrt(std::pow(playerPos.x - enemyPos.x, 2) + std::pow(playerPos.y - enemyPos.y, 2));
    
    // Check if player is within hearing range
    if (distance > hearingRange_) return false;
    
    // Detect based on noise level threshold
    float detectionThreshold = 0.3f; // 30% noise threshold
    
    return noiseLevel > detectionThreshold;
}

bool Enemy::detectPlayerByVibration(const sf::Vector2f& playerPos) const {
    sf::Vector2f enemyPos = position_;
    float distance = std::sqrt(std::pow(playerPos.x - enemyPos.x, 2) + std::pow(playerPos.y - enemyPos.y, 2));
    
    // Vibration detection for very close proximity
    return distance <= vibrationRange_;
}

bool Enemy::detectPlayerByLight(const sf::Vector2f& playerPos, bool flashlightOn) const {
    sf::Vector2f enemyPos = position_;
    float distance = std::sqrt(std::pow(playerPos.x - enemyPos.x, 2) + std::pow(playerPos.y - enemyPos.y, 2));
    
    // Check if player is within light detection range and has flashlight on
    return distance <= lightDetectionRange_ && flashlightOn;
}

// Memory and tracking methods
void Enemy::rememberPlayerPosition(const sf::Vector2f& position) {
    lastKnownPlayerPosition_ = position;
    timeSinceLastSighting_ = 0.f;
    
    // Update player movement history for prediction
    updatePlayerMovementHistory(position);
}

// Psychological warfare methods
void Enemy::enterStalkMode(const sf::Vector2f& playerPosition) {
    isStalkMode_ = true;
    stalkTimer_ = 0.f;
    lastKnownPlayerPosition_ = playerPosition;
    
    if (aiAgent_) {
        aiAgent_->changeState(AIState::STALK);
    }
}

void Enemy::performPsychologicalAttack() {
    if (!isStalkMode_) return;
    
    // This version without player parameter - placeholder for ambient effects
    generateAmbientNoise();
    
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " performing ambient psychological attack");
}

void Enemy::performPsychologicalAttack(entities::Player& player) {
    if (!isStalkMode_) return;
    
    sf::Vector2f playerPos = player.position();
    sf::Vector2f enemyPos = position_;
    float distance = std::sqrt(std::pow(playerPos.x - enemyPos.x, 2) + std::pow(playerPos.y - enemyPos.y, 2));
    
    // Psychological effect when within intimidation radius
    if (distance <= intimidationRadius_) {
        // Drain player's battery
        player.drainBattery(batteryDrainRate_);
        
        // Apply psychological fatigue
        player.drainFatigue(psychologicalDamage_);
        
        Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " performing psychological attack on player");
    }
}

void Enemy::generateAmbientNoise() {
    // Implementation would trigger audio system to play creepy sounds
    // This is a placeholder for ambient sound generation
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " generating ambient noise for psychological effect");
}

// Attack variation methods
void Enemy::performPhysicalAttack(entities::Player& player) {
    if (attackTimer_ > 0.f) return;
    
    int damage = calculateAttackDamage();
    
    // Check for critical hit
    if (static_cast<float>(rand()) / RAND_MAX < criticalChance_) {
        damage = static_cast<int>(damage * 1.5f);
        Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " landed a critical hit!");
    }
    
    player.applyDamage(damage);
    attackTimer_ = attackCooldown_;
    
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " attacked player for " + std::to_string(damage) + " damage");
}

void Enemy::performAmbushAttack(entities::Player& player) {
    if (attackTimer_ > 0.f) return;
    
    int damage = static_cast<int>(calculateAttackDamage() * ambushDamageMultiplier_);
    
    player.applyDamage(damage);
    attackTimer_ = attackCooldown_;
    
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " ambushed player for " + std::to_string(damage) + " damage");
}

int Enemy::calculateAttackDamage() const {
    return baseDamage_;
}

// Communication methods
void Enemy::alertNearbyEnemies(const sf::Vector2f& playerPosition) {
    // This would need access to EnemyManager or similar to find nearby enemies
    // For now, we'll log the alert
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " alerting nearby enemies about player at position (" + 
                           std::to_string(playerPosition.x) + ", " + std::to_string(playerPosition.y) + ")");
    
    lastCommunicationTime_ = 0.f; // Reset communication timer
}

void Enemy::escalateAlert() {
    if (hasEscalatedAlert_) return;
    
    alertLevel_ = std::min(alertLevel_ + 0.3f, 1.0f);
    hasEscalatedAlert_ = true;
    
    if (aiAgent_) {
        aiAgent_->changeState(AIState::ESCALATE_ALERT);
    }
    
    Logger::instance().info("[AI] Enemy " + std::to_string(id_) + " escalating alert level to " + std::to_string(alertLevel_));
}

bool Enemy::isInCommunicationRange(const Enemy& otherEnemy) const {
    sf::Vector2f otherPos = otherEnemy.position();
    sf::Vector2f myPos = position_;
    float distance = std::sqrt(std::pow(otherPos.x - myPos.x, 2) + std::pow(otherPos.y - myPos.y, 2));
    
    return distance <= communicationRange_;
}

// Movement prediction methods
void Enemy::updatePlayerMovementHistory(const sf::Vector2f& playerPos) {
    playerMovementHistory_.push_back(playerPos);
    
    // Keep only the last maxHistorySize_ positions
    if (playerMovementHistory_.size() > maxHistorySize_) {
        playerMovementHistory_.erase(playerMovementHistory_.begin());
    }
}

sf::Vector2f Enemy::predictPlayerMovement(const sf::Vector2f& playerPos, const sf::Vector2f& playerVelocity) const {
    // Simple linear prediction based on velocity
    float predictionTime = 1.5f; // Predict 1.5 seconds ahead
    sf::Vector2f predictedPos = playerPos + playerVelocity * predictionTime;
    
    // Add some randomness based on prediction accuracy
    float randomFactor = 1.0f - predictionAccuracy_;
    float offsetX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 50.0f * randomFactor;
    float offsetY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 50.0f * randomFactor;
    
    return sf::Vector2f(predictedPos.x + offsetX, predictedPos.y + offsetY);
}

sf::Vector2f Enemy::getOptimalInterceptPosition(const sf::Vector2f& playerPos, const sf::Vector2f& playerVelocity) const {
    // Calculate interception point based on enemy and player speeds
    sf::Vector2f enemyPos = position_;
    float enemySpeed = speed_;
    float playerSpeed = std::sqrt(playerVelocity.x * playerVelocity.x + playerVelocity.y * playerVelocity.y);
    
    if (playerSpeed == 0.f) {
        return playerPos; // Player is stationary
    }
    
    // Time to intercept calculation
    sf::Vector2f toPlayer = playerPos - enemyPos;
    float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    
    float timeToIntercept = distance / (enemySpeed + playerSpeed);
    
    // Calculate intercept position
    sf::Vector2f interceptPos = playerPos + playerVelocity * timeToIntercept;
    
    return interceptPos;
}

} // namespace ai
