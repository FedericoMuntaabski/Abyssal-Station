#ifndef ABYSSAL_STATION_SRC_UI_SURVIVALHUD_H
#define ABYSSAL_STATION_SRC_UI_SURVIVALHUD_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <array>

// Forward declarations
namespace entities { class Player; }
namespace core { class AssetManager; }

namespace ui {

class SurvivalHUD {
public:
    SurvivalHUD();
    ~SurvivalHUD() = default;

    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void setPlayer(entities::Player* player) { player_ = player; }
    void setAssetManager(core::AssetManager* assetManager) { assetManager_ = assetManager; }

private:
    void initializeElements();
    void updateVitalBars();
    void updateStealthIndicators();
    void updateNotifications(float deltaTime);
    void renderVitalBars(sf::RenderWindow& window);
    void renderHotbar(sf::RenderWindow& window);
    void renderStealthIndicators(sf::RenderWindow& window);
    void renderNotifications(sf::RenderWindow& window);
    
    // Notification system
    struct Notification {
        std::string message;
        sf::Color color;
        float lifetime;
        float fadeTime;
        bool active;
    };
    
    void showNotification(const std::string& message, sf::Color color, float duration = 3.0f);

    // References
    entities::Player* player_{nullptr};
    core::AssetManager* assetManager_{nullptr};
    
    // Vital bars (bottom-left)
    sf::RectangleShape healthBarBg_;
    sf::RectangleShape healthBar_;
    sf::RectangleShape batteryBarBg_;
    sf::RectangleShape batteryBar_;
    sf::RectangleShape fatigueBarBg_;
    sf::RectangleShape fatigueBar_;
    
    // Weight indicator
    sf::Text weightText_;
    
    // Stamina bar (center-bottom, dynamic)
    sf::RectangleShape staminaBarBg_;
    sf::RectangleShape staminaBar_;
    bool showStaminaBar_{false};
    float staminaBarTimer_{0.0f};
    
    // Hotbar (bottom-right)
    std::array<sf::RectangleShape, 4> hotbarSlots_;
    std::array<sf::RectangleShape, 4> hotbarBorders_;
    sf::Text hotbarWeightText_;
    
    // Stealth indicators
    sf::CircleShape flashlightIcon_;
    sf::CircleShape stealthIcon_;
    sf::CircleShape noiseIndicator_;
    
    // Injury indicator
    sf::CircleShape injuryIcon_;
    
    // Notifications (top-right)
    std::vector<Notification> notifications_;
    sf::Text notificationText_;
    
    // Fonts and styling
    sf::Font* font_{nullptr};
    
    // Constants
    static constexpr float BAR_WIDTH = 120.0f;
    static constexpr float BAR_HEIGHT = 16.0f;
    static constexpr float BAR_SPACING = 25.0f;
    static constexpr float MARGIN = 20.0f;
    static constexpr float HOTBAR_SLOT_SIZE = 40.0f;
    static constexpr float INDICATOR_RADIUS = 8.0f;
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_SURVIVALHUD_H
