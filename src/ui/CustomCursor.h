#ifndef ABYSSAL_STATION_SRC_UI_CUSTOMCURSOR_H
#define ABYSSAL_STATION_SRC_UI_CUSTOMCURSOR_H

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>

namespace ui {

/**
 * @brief Custom cursor implementation that replaces the system cursor with a custom texture
 * when the game window is in focus.
 */
class CustomCursor {
public:
    CustomCursor();
    ~CustomCursor() = default;

    /**
     * @brief Load cursor texture from file
     * @param texturePath Path to cursor texture file
     * @return true if loaded successfully, false otherwise
     */
    bool loadTexture(const std::string& texturePath);

    /**
     * @brief Update cursor position and visibility
     * @param window The render window
     */
    void update(sf::RenderWindow& window);

    /**
     * @brief Render the custom cursor
     * @param window The render window
     */
    void render(sf::RenderWindow& window);

    /**
     * @brief Enable or disable custom cursor
     * @param enabled True to show custom cursor, false to use system cursor
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if custom cursor is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const noexcept { return m_enabled; }

    /**
     * @brief Set cursor scale factor
     * @param scale Scale factor (1.0f = original size)
     */
    void setScale(float scale);

    /**
     * @brief Set cursor offset from mouse position
     * @param offset Offset in pixels
     */
    void setOffset(const sf::Vector2f& offset) { m_offset = offset; }

private:
    std::unique_ptr<sf::Sprite> m_sprite;
    std::shared_ptr<sf::Texture> m_texture;
    sf::Vector2f m_offset{0.f, 0.f};
    float m_scale{1.0f};
    bool m_enabled{false};
    bool m_textureLoaded{false};
    bool m_windowHasFocus{false};

    void updateVisibility(sf::RenderWindow& window);
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_CUSTOMCURSOR_H
