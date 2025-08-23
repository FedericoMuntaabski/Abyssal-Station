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
    // Also bind arrow keys
    bindKey(Action::MoveUp, sf::Keyboard::Key::Up);
    bindKey(Action::MoveDown, sf::Keyboard::Key::Down);
    bindKey(Action::MoveLeft, sf::Keyboard::Key::Left);
    bindKey(Action::MoveRight, sf::Keyboard::Key::Right);
    bindKey(Action::Confirm, sf::Keyboard::Key::Enter);
    bindKey(Action::Cancel, sf::Keyboard::Key::Escape);
    bindKey(Action::Interact, sf::Keyboard::Key::E);
    bindKey(Action::Pause, sf::Keyboard::Key::P);

    // Default mouse bindings: left click -> Confirm
    bindMouse(Action::Confirm, sf::Mouse::Button::Left);

    core::Logger::instance().info("InputManager initialized with default bindings.");
}

void InputManager::bindKey(Action action, sf::Keyboard::Key key) {
    std::lock_guard<std::mutex> lock(mutex_);
    keyBindings_[action].push_back(key);

    std::ostringstream oss;
    oss << "[Input] Acción " << actionToString(action) << " asignada a tecla " << keyToString(key);
    core::Logger::instance().info(oss.str());
}

void InputManager::bindMouse(Action action, sf::Mouse::Button button) {
    std::lock_guard<std::mutex> lock(mutex_);
    mouseBindings_[action].push_back(button);

    std::ostringstream oss;
    oss << "[Input] Acción " << actionToString(action) << " asignada a botón ratón " << static_cast<int>(button);
    core::Logger::instance().info(oss.str());
}

void InputManager::update(const sf::Event& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Update currentKeys_ based on key press/release events (SFML 3 event API)
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            currentKeys_[kp->code] = true;
            // record last key for remapping UI
            lastKeyEvent_ = kp->code;
            hasLastKey_ = true;
        }
    } else if (event.is<sf::Event::KeyReleased>()) {
        if (auto kr = event.getIf<sf::Event::KeyReleased>()) {
            currentKeys_[kr->code] = false;
        }
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            currentMouseButtons_[mbp->button] = true;
            // record last mouse button for remapping UI
            lastMouseEvent_ = mbp->button;
            hasLastMouse_ = true;
        }
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        if (auto mbr = event.getIf<sf::Event::MouseButtonReleased>()) {
            currentMouseButtons_[mbr->button] = false;
        }
    }
}

// Copy current state to previous state once per frame.
void InputManager::endFrame() {
    std::lock_guard<std::mutex> lock(mutex_);
    previousKeys_ = currentKeys_;
    // Also copy mouse button states
    previousMouseButtons_ = currentMouseButtons_;
}

bool InputManager::isActionPressed(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    for (auto k : it->second) {
        auto it2 = currentKeys_.find(k);
        if (it2 != currentKeys_.end() && it2->second) return true;
    }
    // Check mouse bindings
    auto mit = mouseBindings_.find(action);
    if (mit != mouseBindings_.end()) {
        for (auto b : mit->second) {
            auto itb = currentMouseButtons_.find(b);
            if (itb != currentMouseButtons_.end() && itb->second) return true;
        }
    }
    return false;
}

bool InputManager::isActionJustPressed(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    for (auto k : it->second) {
        bool curr = false;
        bool prev = false;
        auto itCurr = currentKeys_.find(k);
        if (itCurr != currentKeys_.end()) curr = itCurr->second;
        auto itPrev = previousKeys_.find(k);
        if (itPrev != previousKeys_.end()) prev = itPrev->second;
        if (curr && !prev) return true;
    }
    // Mouse buttons
    auto mit = mouseBindings_.find(action);
    if (mit != mouseBindings_.end()) {
        for (auto b : mit->second) {
            bool curr = false;
            bool prev = false;
            auto itCurr = currentMouseButtons_.find(b);
            if (itCurr != currentMouseButtons_.end()) curr = itCurr->second;
            auto itPrev = previousMouseButtons_.find(b);
            if (itPrev != previousMouseButtons_.end()) prev = itPrev->second;
            if (curr && !prev) return true;
        }
    }
    return false;
}

bool InputManager::isActionReleased(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return false;
    for (auto k : it->second) {
        bool curr = false;
        bool prev = false;
        auto itCurr = currentKeys_.find(k);
        if (itCurr != currentKeys_.end()) curr = itCurr->second;
        auto itPrev = previousKeys_.find(k);
        if (itPrev != previousKeys_.end()) prev = itPrev->second;
        if (!curr && prev) return true;
    }
    // Mouse buttons
    auto mit = mouseBindings_.find(action);
    if (mit != mouseBindings_.end()) {
        for (auto b : mit->second) {
            bool curr = false;
            bool prev = false;
            auto itCurr = currentMouseButtons_.find(b);
            if (itCurr != currentMouseButtons_.end()) curr = itCurr->second;
            auto itPrev = previousMouseButtons_.find(b);
            if (itPrev != previousMouseButtons_.end()) prev = itPrev->second;
            if (!curr && prev) return true;
        }
    }
    return false;
}

// Rebind (replace) all keys for an action
void InputManager::rebindKeys(Action action, const std::vector<sf::Keyboard::Key>& keys) {
    std::lock_guard<std::mutex> lock(mutex_);
    keyBindings_[action] = keys;
}

// Rebind (replace) all mouse buttons for an action
void InputManager::rebindMouse(Action action, const std::vector<sf::Mouse::Button>& buttons) {
    std::lock_guard<std::mutex> lock(mutex_);
    mouseBindings_[action] = buttons;
}

std::pair<bool, sf::Keyboard::Key> InputManager::getLastKeyEvent() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {hasLastKey_, lastKeyEvent_};
}

std::pair<bool, sf::Mouse::Button> InputManager::getLastMouseButtonEvent() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {hasLastMouse_, lastMouseEvent_};
}

void InputManager::clearLastEvents() {
    std::lock_guard<std::mutex> lock(mutex_);
    hasLastKey_ = false;
    hasLastMouse_ = false;
}

std::vector<sf::Keyboard::Key> InputManager::getKeyBindings(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = keyBindings_.find(action);
    if (it == keyBindings_.end()) return {};
    return it->second;
}

std::vector<sf::Mouse::Button> InputManager::getMouseBindings(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = mouseBindings_.find(action);
    if (it == mouseBindings_.end()) return {};
    return it->second;
}

static std::string mouseButtonToString(sf::Mouse::Button b) {
    switch (b) {
        case sf::Mouse::Button::Left: return "Mouse Left";
        case sf::Mouse::Button::Right: return "Mouse Right";
        case sf::Mouse::Button::Middle: return "Mouse Middle";
        default: return "Mouse" + std::to_string(static_cast<int>(b));
    }
}

std::string InputManager::getBindingName(Action action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto kIt = keyBindings_.find(action);
    if (kIt != keyBindings_.end() && !kIt->second.empty()) {
        return keyToString(kIt->second.front());
    }
    auto mIt = mouseBindings_.find(action);
    if (mIt != mouseBindings_.end() && !mIt->second.empty()) {
        return mouseButtonToString(mIt->second.front());
    }
    return std::string("Unbound");
}

} // namespace input
