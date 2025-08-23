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
    if (collected_) return;

    // Mark as collected and remove collider
    collected_ = true;
    if (collisionManager_) collisionManager_->removeCollider(this);

    // Log event
    core::Logger::instance().info(std::string("Item collected: id=") + std::to_string(id_) );

    // Notify the player entity
    player.onItemCollected(id_);

    // For now, move the item off-screen or hide
    shape_.setPosition(sf::Vector2f(-1000.f, -1000.f));
}

void Item::update(float deltaTime)
{
    // Sync shape position with entity position (in case external move)
    shape_.setPosition(position_);

    // If not collected, keep collider bounds updated
    if (!collected_ && collisionManager_) {
        collisionManager_->addCollider(this, sf::FloatRect(position_, size_));
    }
}

void Item::render(sf::RenderWindow& window)
{
    if (collected_) return;
    window.draw(shape_);
}
