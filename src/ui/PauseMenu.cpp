#include "PauseMenu.h"
#include "../scene/SceneManager.h"
#include "../scene/MenuScene.h"
#include "../input/InputManager.h"
#include "../input/Action.h"
#include "../core/Logger.h"
#include "../core/FontHelper.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace ui {

using core::Logger;

PauseMenu::PauseMenu(scene::SceneManager* manager)
    : sceneManager_(manager)
    , visible_(false)
    , selectedOption_(0)
    , audioLoaded_(false)
    , inputCooldown_(0.0f)
{
    initializeMenu();
    loadAudio();
}

void PauseMenu::initializeMenu() {
    // Semi-transparent overlay
    overlay_.setSize(sf::Vector2f(800.0f, 600.0f));
    overlay_.setFillColor(sf::Color(0, 0, 0, 150));
    overlay_.setPosition(sf::Vector2f(0.0f, 0.0f));
    
    // Menu background
    menuBackground_.setSize(sf::Vector2f(400.0f, 200.0f));
    menuBackground_.setFillColor(sf::Color(30, 30, 30, 220));
    menuBackground_.setPosition(sf::Vector2f(200.0f, 200.0f));
    
    // Load font
    if (!core::loadBestFont(font_)) {
        Logger::instance().error("[PauseMenu] Failed to load font");
    }
    
    // Initialize menu texts
    menuTexts_.resize(static_cast<int>(Option::OptionCount));
    for (int i = 0; i < static_cast<int>(Option::OptionCount); ++i) {
        menuTexts_[i] = std::make_unique<sf::Text>(font_);
        menuTexts_[i]->setCharacterSize(32);
        menuTexts_[i]->setString(getOptionText(static_cast<Option>(i)));
        
        // Position texts
        float yPos = 250.0f + i * 60.0f;
        menuTexts_[i]->setPosition(sf::Vector2f(250.0f, yPos));
    }
    
    updateMenuSelection();
}

void PauseMenu::loadAudio() {
    try {
        hoverBuffer_ = std::make_shared<sf::SoundBuffer>();
        confirmBuffer_ = std::make_shared<sf::SoundBuffer>();
        
        if (hoverBuffer_->loadFromFile("assets/sounds/hover_select.wav") &&
            confirmBuffer_->loadFromFile("assets/sounds/confirm.wav")) {
            
            hoverSound_ = std::make_unique<sf::Sound>(*hoverBuffer_);
            confirmSound_ = std::make_unique<sf::Sound>(*confirmBuffer_);
            audioLoaded_ = true;
            Logger::instance().info("[PauseMenu] Audio loaded successfully");
        }
    } catch (const std::exception& e) {
        Logger::instance().error("[PauseMenu] Failed to load audio: " + std::string(e.what()));
    }
}

void PauseMenu::handleInput(input::InputManager& inputManager) {
    if (!visible_ || inputCooldown_ > 0.0f) return;
    
    using input::Action;
    
    // Navigation
    if (inputManager.isActionPressed(Action::MoveUp)) {
        selectedOption_ = (selectedOption_ - 1 + static_cast<int>(Option::OptionCount)) % static_cast<int>(Option::OptionCount);
        playHoverSound();
        updateMenuSelection();
        inputCooldown_ = INPUT_DELAY;
    }
    else if (inputManager.isActionPressed(Action::MoveDown)) {
        selectedOption_ = (selectedOption_ + 1) % static_cast<int>(Option::OptionCount);
        playHoverSound();
        updateMenuSelection();
        inputCooldown_ = INPUT_DELAY;
    }
    
    // Selection
    if (inputManager.isActionPressed(Action::Confirm)) {
        playConfirmSound();
        executeSelectedOption();
        inputCooldown_ = INPUT_DELAY;
    }
    
    // Close menu with ESC
    if (inputManager.isActionPressed(Action::Cancel)) {
        hide();
        inputCooldown_ = INPUT_DELAY;
    }
}

void PauseMenu::update(float deltaTime) {
    if (inputCooldown_ > 0.0f) {
        inputCooldown_ -= deltaTime;
    }
}

void PauseMenu::render(sf::RenderWindow& window) {
    if (!visible_) return;
    
    // Render overlay
    window.draw(overlay_);
    
    // Render menu background
    window.draw(menuBackground_);
    
    // Render menu texts
    for (const auto& text : menuTexts_) {
        if (text) {
            window.draw(*text);
        }
    }
}

void PauseMenu::show() {
    visible_ = true;
    Logger::instance().info("[PauseMenu] Menu shown");
}

void PauseMenu::hide() {
    visible_ = false;
    Logger::instance().info("[PauseMenu] Menu hidden");
}

void PauseMenu::toggle() {
    if (visible_) {
        hide();
    } else {
        show();
    }
}

void PauseMenu::updateMenuSelection() {
    for (int i = 0; i < static_cast<int>(Option::OptionCount); ++i) {
        if (menuTexts_[i]) {
            if (i == selectedOption_) {
                menuTexts_[i]->setFillColor(sf::Color::Yellow);
                menuTexts_[i]->setStyle(sf::Text::Bold);
            } else {
                menuTexts_[i]->setFillColor(sf::Color::White);
                menuTexts_[i]->setStyle(sf::Text::Regular);
            }
        }
    }
}

void PauseMenu::executeSelectedOption() {
    switch (static_cast<Option>(selectedOption_)) {
        case Option::Resume:
            hide();
            break;
            
        case Option::MainMenu:
            hide();
            if (sceneManager_) {
                // Return to main menu
                auto menuScene = std::make_unique<scene::MenuScene>(sceneManager_);
                sceneManager_->replace(std::move(menuScene));
                Logger::instance().info("[PauseMenu] Returning to main menu");
            }
            break;
            
        default:
            break;
    }
}

void PauseMenu::playHoverSound() {
    if (audioLoaded_ && hoverSound_) {
        hoverSound_->play();
    }
}

void PauseMenu::playConfirmSound() {
    if (audioLoaded_ && confirmSound_) {
        confirmSound_->play();
    }
}

std::string PauseMenu::getOptionText(Option option) const {
    switch (option) {
        case Option::Resume: return "Reanudar";
        case Option::MainMenu: return "Volver al Menu Principal";
        default: return "Unknown";
    }
}

} // namespace ui
