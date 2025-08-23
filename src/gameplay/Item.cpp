#include "Item.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include "../collisions/CollisionManager.h"

using namespace gameplay;

Item::Item(entities::Entity::Id id,
           const sf::Vector2f& position,
           const sf::Vector2f& size,
           ItemType type,
           collisions::CollisionManager* collisionManager)
    : entities::Entity(id, position, size)
    , type_(type)
    , collected_(false)
    , shape_(size)
    , collisionManager_(collisionManager)
{
    // mark this entity as an Item for collision filtering
    setCollisionLayer(entities::Entity::Layer::Item);
    shape_.setPosition(sf::Vector2f(position_.x, position_.y));
    shape_.setFillColor(sf::Color::Yellow);

    // Bounds checking - warn if item is placed outside reasonable bounds
    constexpr float MAX_COORDINATE = 10000.f;
    if (position_.x < -MAX_COORDINATE || position_.x > MAX_COORDINATE ||
        position_.y < -MAX_COORDINATE || position_.y > MAX_COORDINATE) {
        core::Logger::instance().warning(std::string("Item created outside reasonable bounds: id=") + std::to_string(id) + 
                                        ", position=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");
    }

    // Register collider if collision manager available
    if (collisionManager_) {
        collisionManager_->addCollider(this, sf::FloatRect(position_, size_));
    }
}

Item::~Item()
{
    if (collisionManager_) {
        collisionManager_->removeCollider(this);
    }
}

void Item::setCollisionManager(collisions::CollisionManager* cm)
{
    if (collisionManager_ == cm) return;
    if (collisionManager_) collisionManager_->removeCollider(this);
    collisionManager_ = cm;
    if (collisionManager_) collisionManager_->addCollider(this, sf::FloatRect(position_, size_));
}

void Item::interact(entities::Player& player)
{
    if (collected_ || disabled_) {
        if (collected_) {
            core::Logger::instance().warning(std::string("Attempt to interact with already collected item: id=") + std::to_string(id_));
        } else {
            core::Logger::instance().warning(std::string("Attempt to interact with disabled item: id=") + std::to_string(id_));
        }
        return;
    }

    // Start collection animation
    isAnimating_ = true;
    animationTime_ = 0.0f;
    
    // Mark as collected and remove collider
    collected_ = true;
    if (collisionManager_) collisionManager_->removeCollider(this);

    // Enhanced logging with position and type
    std::string typeStr = (type_ == ItemType::Key) ? "Key" : 
                         (type_ == ItemType::Tool) ? "Tool" : "Collectible";
    core::Logger::instance().info(std::string("Item collected: id=") + std::to_string(id_) + 
                                 ", type=" + typeStr + 
                                 ", position=(" + std::to_string(position_.x) + "," + std::to_string(position_.y) + ")");

    // Notify the player entity
    player.onItemCollected(id_);
}

void Item::update(float deltaTime)
{
    // Handle collection animation
    if (isAnimating_) {
        animationTime_ += deltaTime;
        constexpr float ANIMATION_DURATION = 0.5f; // 500ms
        
        if (animationTime_ >= ANIMATION_DURATION) {
            // Animation complete - hide item
            isAnimating_ = false;
            shape_.setPosition(sf::Vector2f(-1000.f, -1000.f));
        } else {
            // Scale-up and fade animation
            float progress = animationTime_ / ANIMATION_DURATION;
            float scale = 1.0f + progress * 0.5f; // Scale up by 50%
            float alpha = 255.0f * (1.0f - progress); // Fade out
            
            shape_.setScale({scale, scale});
            sf::Color color = shape_.getFillColor();
            color.a = static_cast<std::uint8_t>(alpha);
            shape_.setFillColor(color);
        }
        return; // Don't update position during animation
    }

    // Sync shape position with entity position (in case external move)
    shape_.setPosition(position_);

    // Only update collider bounds if not collected, not disabled, and position or size changed
    if (!collected_ && !disabled_ && collisionManager_) {
        static sf::Vector2f lastPosition_ = position_;
        static sf::Vector2f lastSize_ = size_;
        
        if (lastPosition_ != position_ || lastSize_ != size_) {
            collisionManager_->updateColliderBounds(this, sf::FloatRect(position_, size_));
            lastPosition_ = position_;
            lastSize_ = size_;
        }
    }
}

void Item::render(sf::RenderWindow& window)
{
    if (collected_ && !isAnimating_) return;
    if (disabled_) {
        // Render disabled items with different color
        sf::Color originalColor = shape_.getFillColor();
        shape_.setFillColor(sf::Color(100, 100, 100, 128)); // Gray and transparent
        window.draw(shape_);
        shape_.setFillColor(originalColor);
    } else {
        window.draw(shape_);
    }
}
