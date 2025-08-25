#include "SurvivalHUD.h"
#include "../entities/Player.h"
#include "../core/AssetManager.h"
#include "../core/Logger.h"
#include <algorithm>

namespace ui {

using core::Logger;

// Static dummy font for initialization - will be replaced with real font later
static sf::Font g_dummyFont;

SurvivalHUD::SurvivalHUD() : 
    weightText_(g_dummyFont, "", 14),
    hotbarWeightText_(g_dummyFont, "", 12),
    notificationText_(g_dummyFont, "", 16) {
    initializeElements();
}

void SurvivalHUD::initializeElements() {
    // Initialize vital bars (bottom-left)
    float baseY = 600.0f - MARGIN - BAR_HEIGHT; // Assuming 600px window height
    
    // Health bar (red)
    healthBarBg_.setSize({BAR_WIDTH, BAR_HEIGHT});
    healthBarBg_.setPosition({MARGIN, baseY});
    healthBarBg_.setFillColor(sf::Color(80, 20, 20)); // Dark red background
    
    healthBar_.setSize({BAR_WIDTH, BAR_HEIGHT});
    healthBar_.setPosition({MARGIN, baseY});
    healthBar_.setFillColor(sf::Color(220, 50, 50)); // Bright red
    
    // Battery bar (blue)
    batteryBarBg_.setSize({BAR_WIDTH, BAR_HEIGHT});
    batteryBarBg_.setPosition({MARGIN, baseY - BAR_SPACING});
    batteryBarBg_.setFillColor(sf::Color(20, 20, 80)); // Dark blue background
    
    batteryBar_.setSize({BAR_WIDTH, BAR_HEIGHT});
    batteryBar_.setPosition({MARGIN, baseY - BAR_SPACING});
    batteryBar_.setFillColor(sf::Color(50, 150, 220)); // Bright blue
    
    // Fatigue bar (yellow/orange)
    fatigueBarBg_.setSize({BAR_WIDTH, BAR_HEIGHT});
    fatigueBarBg_.setPosition({MARGIN, baseY - BAR_SPACING * 2});
    fatigueBarBg_.setFillColor(sf::Color(80, 60, 20)); // Dark yellow background
    
    fatigueBar_.setSize({BAR_WIDTH, BAR_HEIGHT});
    fatigueBar_.setPosition({MARGIN, baseY - BAR_SPACING * 2});
    fatigueBar_.setFillColor(sf::Color(220, 180, 50)); // Bright yellow
    
    // Weight text
    weightText_.setPosition({MARGIN, baseY - BAR_SPACING * 3});
    weightText_.setCharacterSize(14);
    weightText_.setFillColor(sf::Color::White);
    
    // Stamina bar (center-bottom, dynamic)
    staminaBarBg_.setSize({BAR_WIDTH * 1.5f, BAR_HEIGHT * 0.75f});
    staminaBarBg_.setPosition({400.0f - BAR_WIDTH * 0.75f, baseY + BAR_SPACING}); // Center horizontally
    staminaBarBg_.setFillColor(sf::Color(40, 40, 40));
    
    staminaBar_.setSize({BAR_WIDTH * 1.5f, BAR_HEIGHT * 0.75f});
    staminaBar_.setPosition({400.0f - BAR_WIDTH * 0.75f, baseY + BAR_SPACING});
    staminaBar_.setFillColor(sf::Color::White);
    
    // Hotbar (bottom-right)
    float hotbarStartX = 800.0f - MARGIN - (HOTBAR_SLOT_SIZE * 4 + 12); // 4 slots with spacing
    for (int i = 0; i < 4; ++i) {
        hotbarSlots_[i].setSize({HOTBAR_SLOT_SIZE, HOTBAR_SLOT_SIZE});
        hotbarSlots_[i].setPosition({hotbarStartX + i * (HOTBAR_SLOT_SIZE + 4), baseY - 10});
        hotbarSlots_[i].setFillColor(sf::Color(40, 40, 40, 180));
        
        hotbarBorders_[i].setSize({HOTBAR_SLOT_SIZE + 2, HOTBAR_SLOT_SIZE + 2});
        hotbarBorders_[i].setPosition({hotbarStartX + i * (HOTBAR_SLOT_SIZE + 4) - 1, baseY - 11});
        hotbarBorders_[i].setFillColor(sf::Color::Transparent);
        hotbarBorders_[i].setOutlineThickness(1.0f);
        hotbarBorders_[i].setOutlineColor(sf::Color(100, 100, 100));
    }
    
    hotbarWeightText_.setPosition({hotbarStartX, baseY + HOTBAR_SLOT_SIZE + 5});
    hotbarWeightText_.setCharacterSize(12);
    hotbarWeightText_.setFillColor(sf::Color::White);
    
    // Stealth indicators (near hotbar)
    flashlightIcon_.setRadius(INDICATOR_RADIUS);
    flashlightIcon_.setPosition({hotbarStartX - 30, baseY - BAR_SPACING});
    flashlightIcon_.setFillColor(sf::Color::Yellow);
    
    stealthIcon_.setRadius(INDICATOR_RADIUS);
    stealthIcon_.setPosition({hotbarStartX - 30, baseY - BAR_SPACING - 25});
    stealthIcon_.setFillColor(sf::Color::Green);
    
    noiseIndicator_.setRadius(INDICATOR_RADIUS);
    noiseIndicator_.setPosition({hotbarStartX - 30, baseY - BAR_SPACING - 50});
    noiseIndicator_.setFillColor(sf::Color::Red);
    
    // Injury indicator
    injuryIcon_.setRadius(INDICATOR_RADIUS);
    injuryIcon_.setPosition({MARGIN + BAR_WIDTH + 10, baseY});
    injuryIcon_.setFillColor(sf::Color(150, 0, 0));
    
    // Notification text
    notificationText_.setPosition({600, 50}); // Top-right area
    notificationText_.setCharacterSize(16);
    notificationText_.setFillColor(sf::Color::White);
    
    Logger::instance().info("[SurvivalHUD] Initialized all UI elements");
}

void SurvivalHUD::update(float deltaTime) {
    if (!player_) return;
    
    updateVitalBars();
    updateStealthIndicators();
    updateNotifications(deltaTime);
    
    // Handle stamina bar visibility
    if (player_->isRunning() || player_->getFatigue() > 80.0f) {
        showStaminaBar_ = true;
        staminaBarTimer_ = 0.0f;
    } else {
        staminaBarTimer_ += deltaTime;
        if (staminaBarTimer_ > 2.0f) { // Hide after 2 seconds
            showStaminaBar_ = false;
        }
    }
}

void SurvivalHUD::updateVitalBars() {
    if (!player_) return;
    
    // Update health bar
    float healthRatio = static_cast<float>(player_->health()) / 100.0f;
    healthBar_.setSize({BAR_WIDTH * healthRatio, BAR_HEIGHT});
    
    // Health bar effects
    if (healthRatio < 0.25f) {
        // Pulsing red effect for low health
        float pulse = (sin(staminaBarTimer_ * 10.0f) + 1.0f) * 0.5f;
        healthBar_.setFillColor(sf::Color(220 + 35 * pulse, 50, 50));
    } else if (healthRatio < 0.5f) {
        healthBar_.setFillColor(sf::Color(220, 50, 50));
    } else {
        healthBar_.setFillColor(sf::Color(100, 220, 100)); // Green when healthy
    }
    
    // Update battery bar
    float batteryRatio = player_->getBattery() / 100.0f;
    batteryBar_.setSize({BAR_WIDTH * batteryRatio, BAR_HEIGHT});
    
    // Battery bar effects
    if (batteryRatio < 0.1f) {
        // Vibration effect for very low battery
        float vibration = sin(staminaBarTimer_ * 15.0f) * 2.0f;
        batteryBar_.setPosition({MARGIN + vibration, batteryBarBg_.getPosition().y});
    } else {
        batteryBar_.setPosition({MARGIN, batteryBarBg_.getPosition().y});
    }
    
    // Update fatigue bar
    float fatigueRatio = player_->getFatigue() / 100.0f;
    fatigueBar_.setSize({BAR_WIDTH * fatigueRatio, BAR_HEIGHT});
    
    // Fatigue color changes
    if (fatigueRatio > 0.8f) {
        fatigueBar_.setFillColor(sf::Color(220, 100, 50)); // Orange when tired
    } else {
        fatigueBar_.setFillColor(sf::Color(220, 180, 50)); // Yellow
    }
    
    // Update weight text
    float weightRatio = player_->getWeight() / player_->getMaxWeight();
    sf::Color weightColor = sf::Color::White;
    if (weightRatio > 0.9f) {
        weightColor = sf::Color::Red;
    } else if (weightRatio > 0.8f) {
        weightColor = sf::Color::Yellow;
    }
    
    weightText_.setString("Weight: " + std::to_string(static_cast<int>(player_->getWeight())) + 
                         "/" + std::to_string(static_cast<int>(player_->getMaxWeight())) + "kg");
    weightText_.setFillColor(weightColor);
    
    // Update stamina bar
    if (showStaminaBar_) {
        float staminaRatio = (100.0f - player_->getFatigue()) / 100.0f;
        staminaBar_.setSize({BAR_WIDTH * 1.5f * staminaRatio, BAR_HEIGHT * 0.75f});
        
        if (staminaRatio < 0.1f) {
            staminaBar_.setFillColor(sf::Color::Red);
        } else {
            staminaBar_.setFillColor(sf::Color::White);
        }
    }
}

void SurvivalHUD::updateStealthIndicators() {
    if (!player_) return;
    
    // Flashlight indicator
    if (player_->isFlashlightOn()) {
        float brightness = player_->getBattery() / 100.0f;
        flashlightIcon_.setFillColor(sf::Color(255, 255 * brightness, 0)); // Yellow to dark
    } else {
        flashlightIcon_.setFillColor(sf::Color(80, 80, 80)); // Dark gray when off
    }
    
    // Stealth indicator (eye icon simulation)
    float detectability = player_->getDetectability();
    if (player_->isCrouching()) {
        stealthIcon_.setFillColor(sf::Color(0, 150, 0)); // Green when crouching
        stealthIcon_.setRadius(INDICATOR_RADIUS * 0.7f); // Smaller when hidden
    } else {
        stealthIcon_.setFillColor(sf::Color(150 * detectability, 150 * (1.0f - detectability), 0));
        stealthIcon_.setRadius(INDICATOR_RADIUS * detectability);
    }
    
    // Noise indicator (show when player is making noise)
    if (player_->isMoving()) {
        float noiseLevel = player_->isRunning() ? 1.0f : 0.5f;
        if (player_->isCrouching()) noiseLevel *= 0.3f;
        
        noiseIndicator_.setRadius(INDICATOR_RADIUS * noiseLevel);
        noiseIndicator_.setFillColor(sf::Color(255 * noiseLevel, 100, 100));
    } else {
        noiseIndicator_.setRadius(2.0f); // Minimal when not moving
        noiseIndicator_.setFillColor(sf::Color(50, 50, 50));
    }
}

void SurvivalHUD::updateNotifications(float deltaTime) {
    // Update notification lifetimes
    for (auto& notification : notifications_) {
        if (notification.active) {
            notification.lifetime -= deltaTime;
            if (notification.lifetime <= 0.0f) {
                notification.active = false;
            } else if (notification.lifetime <= notification.fadeTime) {
                // Fade out effect
                float alpha = notification.lifetime / notification.fadeTime;
                notification.color.a = static_cast<uint8_t>(255 * alpha);
            }
        }
    }
    
    // Remove inactive notifications
    notifications_.erase(
        std::remove_if(notifications_.begin(), notifications_.end(),
                      [](const Notification& n) { return !n.active; }),
        notifications_.end());
    
    // Check for conditions to show notifications
    if (player_) {
        static float lastBatteryCheck = 100.0f;
        if (player_->getBattery() < 20.0f && lastBatteryCheck >= 20.0f) {
            showNotification("Battery Low!", sf::Color::Yellow);
        }
        lastBatteryCheck = player_->getBattery();
        
        static bool lastInjuryState = false;
        if (player_->hasInjury() && !lastInjuryState) {
            showNotification("Critical Injury - Mobility Reduced", sf::Color::Red, 5.0f);
        }
        lastInjuryState = player_->hasInjury();
        
        static float lastWeightCheck = 0.0f;
        float currentWeight = player_->getWeight() / player_->getMaxWeight();
        if (currentWeight > 0.9f && lastWeightCheck <= 0.9f) {
            showNotification("Overloaded - Movement Reduced", sf::Color(255, 165, 0)); // Orange color
        }
        lastWeightCheck = currentWeight;
    }
}

void SurvivalHUD::showNotification(const std::string& message, sf::Color color, float duration) {
    Notification notification;
    notification.message = message;
    notification.color = color;
    notification.lifetime = duration;
    notification.fadeTime = 1.0f;
    notification.active = true;
    
    notifications_.push_back(notification);
    
    Logger::instance().info("[SurvivalHUD] Notification: " + message);
}

void SurvivalHUD::render(sf::RenderWindow& window) {
    if (!player_) return;
    
    renderVitalBars(window);
    renderHotbar(window);
    renderStealthIndicators(window);
    renderNotifications(window);
}

void SurvivalHUD::renderVitalBars(sf::RenderWindow& window) {
    // Render vital bars backgrounds
    window.draw(healthBarBg_);
    window.draw(batteryBarBg_);
    window.draw(fatigueBarBg_);
    
    // Render vital bars
    window.draw(healthBar_);
    window.draw(batteryBar_);
    window.draw(fatigueBar_);
    
    // Render weight text
    if (font_) {
        weightText_.setFont(*font_);
        window.draw(weightText_);
    }
    
    // Render stamina bar if visible
    if (showStaminaBar_) {
        window.draw(staminaBarBg_);
        window.draw(staminaBar_);
    }
    
    // Render injury indicator if player has injury
    if (player_->hasInjury()) {
        window.draw(injuryIcon_);
    }
}

void SurvivalHUD::renderHotbar(sf::RenderWindow& window) {
    // Render hotbar slots
    for (int i = 0; i < 4; ++i) {
        // Only render available slots
        if (i < player_->getAvailableSlots()) {
            window.draw(hotbarSlots_[i]);
            hotbarBorders_[i].setOutlineColor(sf::Color(100, 100, 100));
        } else {
            // Render disabled slots
            hotbarSlots_[i].setFillColor(sf::Color(20, 20, 20, 100));
            window.draw(hotbarSlots_[i]);
            hotbarBorders_[i].setOutlineColor(sf::Color::Red);
        }
        window.draw(hotbarBorders_[i]);
    }
    
    // Render weight indicator for hotbar
    if (font_) {
        float weightRatio = player_->getWeight() / player_->getMaxWeight();
        hotbarWeightText_.setString("Weight: " + std::to_string(static_cast<int>(weightRatio * 100)) + "%");
        hotbarWeightText_.setFont(*font_);
        
        if (weightRatio > 0.9f) {
            hotbarWeightText_.setFillColor(sf::Color::Red);
        } else if (weightRatio > 0.8f) {
            hotbarWeightText_.setFillColor(sf::Color::Yellow);
        } else {
            hotbarWeightText_.setFillColor(sf::Color::White);
        }
        
        window.draw(hotbarWeightText_);
    }
}

void SurvivalHUD::renderStealthIndicators(sf::RenderWindow& window) {
    // Render stealth indicators
    window.draw(flashlightIcon_);
    window.draw(stealthIcon_);
    window.draw(noiseIndicator_);
}

void SurvivalHUD::renderNotifications(sf::RenderWindow& window) {
    if (!font_ || notifications_.empty()) return;
    
    float yOffset = 0.0f;
    for (const auto& notification : notifications_) {
        if (notification.active) {
            notificationText_.setString(notification.message);
            notificationText_.setFillColor(notification.color);
            notificationText_.setFont(*font_);
            notificationText_.setPosition({600, 50 + yOffset});
            
            window.draw(notificationText_);
            yOffset += 25.0f;
        }
    }
}

} // namespace ui
