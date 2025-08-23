#include "OptionsMenu.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace ui {

using input::InputManager;

OptionsMenu::OptionsMenu(scene::SceneManager* manager)
    : Menu("OptionsMenu"), m_manager(manager) {
    m_options = {"Volume", "Controls", "Back"};
}

void OptionsMenu::onEnter() {
    m_selected = 0;
}

void OptionsMenu::onExit() {}

int OptionsMenu::volume() const noexcept { return m_volume; }

void OptionsMenu::setVolume(int vol) {
    m_volume = std::clamp(vol, 0, 100);
    applyVolume();
}

void OptionsMenu::applyVolume() {
    core::Logger::instance().info(std::string("Volume set to ") + std::to_string(m_volume));
    // TODO: integrate with audio system
}

void OptionsMenu::handleInput() {
    auto& im = InputManager::getInstance();

    if (m_waitingForRemap) {
        // Don't process normal input while waiting for remap
        return;
    }

    if (m_inControlsMode) {
        // In controls mode, navigate through actions
        if (im.isActionJustPressed(input::Action::MoveUp)) {
            if (m_selectedAction == 0) m_selectedAction = m_actionsToShow.size() - 1;
            else --m_selectedAction;
        }

        if (im.isActionJustPressed(input::Action::MoveDown)) {
            m_selectedAction = (m_selectedAction + 1) % m_actionsToShow.size();
        }

        if (im.isActionJustPressed(input::Action::Confirm)) {
            // Start remapping the selected action
            m_waitingForRemap = true;
            m_actionToRemap = m_actionsToShow[m_selectedAction];
            core::Logger::instance().info("Starting remap for selected action");
        }

        if (im.isActionJustPressed(input::Action::Cancel)) {
            // Exit controls mode
            m_inControlsMode = false;
            m_selected = 1; // Back to "Controls" option
        }
    } else {
        // Normal menu navigation
        if (im.isActionJustPressed(input::Action::MoveUp)) {
            if (m_selected == 0) m_selected = m_options.size() - 1;
            else --m_selected;
        }

        if (im.isActionJustPressed(input::Action::MoveDown)) {
            m_selected = (m_selected + 1) % m_options.size();
        }

        // Adjust volume with left/right when Volume option selected
        if (m_selected == 0) {
            if (im.isActionJustPressed(input::Action::MoveLeft)) {
                setVolume(m_volume - 5);
            }
            if (im.isActionJustPressed(input::Action::MoveRight)) {
                setVolume(m_volume + 5);
            }
        }

        if (im.isActionJustPressed(input::Action::Confirm)) {
            const auto& choice = m_options[m_selected];
            core::Logger::instance().info("OptionsMenu selected: " + choice);
            if (choice == "Back") {
                deactivate();
            } else if (choice == "Controls") {
                // Enter controls mode
                m_inControlsMode = true;
                m_selectedAction = 0;
            }
        }

        if (im.isActionJustPressed(input::Action::Cancel)) {
            deactivate();
        }
    }
}

void OptionsMenu::update(float dt) {
    (void)dt;
    if (m_waitingForRemap) {
        auto& im = InputManager::getInstance();
        auto lastKey = im.getLastKeyEvent();
        if (lastKey.first) {
            im.rebindKeys(m_actionToRemap, {lastKey.second});
            core::Logger::instance().info(std::string("Rebound action to key ") + std::to_string(static_cast<int>(lastKey.second)));
            
            // Immediately save bindings after remapping
            im.saveBindings("config/input_bindings.json");
            
            m_waitingForRemap = false;
            im.clearLastEvents();
        } else {
            auto lastMouse = im.getLastMouseButtonEvent();
            if (lastMouse.first) {
                im.rebindMouse(m_actionToRemap, {lastMouse.second});
                core::Logger::instance().info(std::string("Rebound action to mouse button ") + std::to_string(static_cast<int>(lastMouse.second)));
                
                // Immediately save bindings after remapping
                im.saveBindings("config/input_bindings.json");
                
                m_waitingForRemap = false;
                im.clearLastEvents();
            }
        }
    }
}

void OptionsMenu::render(sf::RenderWindow& window) {
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    if (m_inControlsMode) {
        // Render controls mode
        sf::Text title(font, "Controls Configuration", 24);
        title.setFillColor(sf::Color::White);
        title.setPosition(sf::Vector2f(110.f, 80.f));
        window.draw(title);

        auto& im = InputManager::getInstance();
        for (std::size_t i = 0; i < m_actionsToShow.size(); ++i) {
            const auto act = m_actionsToShow[i];
            std::string name;
            switch (act) {
                case input::Action::MoveUp: name = "Move Up"; break;
                case input::Action::MoveDown: name = "Move Down"; break;
                case input::Action::MoveLeft: name = "Move Left"; break;
                case input::Action::MoveRight: name = "Move Right"; break;
                case input::Action::Confirm: name = "Confirm"; break;
                case input::Action::Cancel: name = "Cancel"; break;
                case input::Action::Interact: name = "Interact"; break;
                case input::Action::Pause: name = "Pause"; break;
                default: name = "Unknown"; break;
            }
            
            std::string binding = im.getBindingName(act);
            
            // Show all bindings for this action
            auto keyBindings = im.getKeyBindings(act);
            auto mouseBindings = im.getMouseBindings(act);
            std::string allBindings;
            for (size_t kb = 0; kb < keyBindings.size(); ++kb) {
                if (!allBindings.empty()) allBindings += ", ";
                allBindings += im.getBindingName(act); // This gets the first binding name
            }
            for (const auto& mb : mouseBindings) {
                if (!allBindings.empty()) allBindings += ", ";
                switch (mb) {
                    case sf::Mouse::Button::Left: allBindings += "Mouse Left"; break;
                    case sf::Mouse::Button::Right: allBindings += "Mouse Right"; break;
                    case sf::Mouse::Button::Middle: allBindings += "Mouse Middle"; break;
                    default: allBindings += "Mouse" + std::to_string(static_cast<int>(mb)); break;
                }
            }
            if (allBindings.empty()) allBindings = "Unbound";

            sf::Text actionText(font, name + ": " + allBindings, 18);
            actionText.setPosition(sf::Vector2f(110.f, m_startY + static_cast<float>(i) * m_spacing));
            actionText.setFillColor(i == m_selectedAction ? sf::Color::Yellow : sf::Color::White);
            window.draw(actionText);
        }

        if (m_waitingForRemap) {
            sf::Text instr(font, "Press a key or click a mouse button to rebind...", 18);
            instr.setFillColor(sf::Color::Red);
            instr.setPosition(sf::Vector2f(110.f, m_startY + static_cast<float>(m_actionsToShow.size()) * m_spacing + 20.f));
            window.draw(instr);
        }

        sf::Text backInstr(font, "Press Cancel to go back", 16);
        backInstr.setFillColor(sf::Color(128, 128, 128)); // Gray color
        backInstr.setPosition(sf::Vector2f(110.f, m_startY + static_cast<float>(m_actionsToShow.size()) * m_spacing + 60.f));
        window.draw(backInstr);
    } else {
        // Normal menu mode
        for (std::size_t i = 0; i < m_options.size(); ++i) {
            std::string content;
            if (i == 0) {
                // show volume value inline
                content = m_options[i] + std::string(": ") + std::to_string(m_volume);
            } else {
                content = m_options[i];
            }
            sf::Text text(font, content, 20);
            text.setFillColor(i == m_selected ? sf::Color::Green : sf::Color::White);
            text.setPosition(sf::Vector2f(110.f, m_startY + static_cast<float>(i) * m_spacing));
            window.draw(text);
        }
    }
}

} // namespace ui
