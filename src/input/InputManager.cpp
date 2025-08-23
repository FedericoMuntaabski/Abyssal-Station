#include "InputManager.h"
#include <core/Logger.h>
#include <sstream>

namespace input {

static const char* actionToString(Action a) {
    switch (a) {
        case Action::MoveUp: return "MoveUp";
        case Action::MoveDown: return "MoveDown";
        case Action::MoveLeft: return "MoveLeft";
        case Action::MoveRight: return "MoveRight";
        case Action::Confirm: return "Confirm";
        case Action::Cancel: return "Cancel";
        case Action::Pause: return "Pause";
        default: return "Unknown";
    }
}

static std::string keyToString(sf::Keyboard::Key k) {
    using K = sf::Keyboard::Key;
    switch (k) {
        case K::A: return "A";
        case K::B: return "B";
        case K::C: return "C";
        case K::D: return "D";
        case K::E: return "E";
        case K::F: return "F";
        case K::G: return "G";
        case K::H: return "H";
        case K::I: return "I";
        case K::J: return "J";
        case K::K: return "K";
        case K::L: return "L";
        case K::M: return "M";
        case K::N: return "N";
        case K::O: return "O";
        case K::P: return "P";
        case K::Q: return "Q";
        case K::R: return "R";
        case K::S: return "S";
        case K::T: return "T";
        case K::U: return "U";
        case K::V: return "V";
        case K::W: return "W";
        case K::X: return "X";
        case K::Y: return "Y";
        case K::Z: return "Z";
        case K::Enter: return "Enter";
        case K::Escape: return "Escape";
        case K::Space: return "Space";
        case K::Left: return "Left";
        case K::Right: return "Right";
        case K::Up: return "Up";
        case K::Down: return "Down";
        default:
            return std::to_string(static_cast<int>(k));
    }
}

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager() {
    // Default bindings: WASD, Enter, Escape, P
    bindKey(Action::MoveUp, sf::Keyboard::Key::W);
    bindKey(Action::MoveDown, sf::Keyboard::Key::S);
    bindKey(Action::MoveLeft, sf::Keyboard::Key::A);
    bindKey(Action::MoveRight, sf::Keyboard::Key::D);
    bindKey(Action::Confirm, sf::Keyboard::Key::Enter);
    bindKey(Action::Cancel, sf::Keyboard::Key::Escape);
    bindKey(Action::Pause, sf::Keyboard::Key::P);

    core::Logger::instance().info("InputManager initialized with default bindings.");
}

void InputManager::bindKey(Action action, sf::Keyboard::Key key) {
    std::lock_guard<std::mutex> lock(mutex_);
    keyBindings_[action] = key;

    std::ostringstream oss;
    oss << "[Input] Acción " << actionToString(action) << " asignada a tecla " << keyToString(key);
    core::Logger::instance().info(oss.str());
}

void InputManager::update(const sf::Event& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Update currentKeys_ based on key press/release events (SFML 3 event API)
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            currentKeys_[kp->code] = true;
        }
    } else if (event.is<sf::Event::KeyReleased>()) {
        if (auto kr = event.getIf<sf::Event::KeyReleased>()) {
            currentKeys_[kr->code] = false;
        }
    }

    // Note: transition detection is implemented in the query methods by
    // comparing previousKeys_ and currentKeys_. At the end of each update,
    // copy current state to previous state for the next frame.
    previousKeys_ = currentKeys_;
}

bool InputManager::isActionPressed(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    sf::Keyboard::Key k = it->second;
    auto it2 = currentKeys_.find(k);
    return it2 != currentKeys_.end() && it2->second;
}

bool InputManager::isActionJustPressed(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    sf::Keyboard::Key k = it->second;
    bool curr = false;
    bool prev = false;
    auto itCurr = currentKeys_.find(k);
    if (itCurr != currentKeys_.end()) curr = itCurr->second;
    auto itPrev = previousKeys_.find(k);
    if (itPrev != previousKeys_.end()) prev = itPrev->second;
    return curr && !prev;
}

bool InputManager::isActionReleased(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    sf::Keyboard::Key k = it->second;
    bool curr = false;
    bool prev = false;
    auto itCurr = currentKeys_.find(k);
    if (itCurr != currentKeys_.end()) curr = itCurr->second;
    auto itPrev = previousKeys_.find(k);
    if (itPrev != previousKeys_.end()) prev = itPrev->second;
    return !curr && prev;
}

} // namespace input
