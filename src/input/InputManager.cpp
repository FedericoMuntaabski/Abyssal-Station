#include "InputManager.h"
#include <core/Logger.h>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

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
        case Action::Interact: return "Interact";
        case Action::Run: return "Run";
        case Action::Attack: return "Attack";
        case Action::ToggleFlashlight: return "ToggleFlashlight";
        case Action::Crouch: return "Crouch";
        case Action::HotbarSlot1: return "HotbarSlot1";
        case Action::HotbarSlot2: return "HotbarSlot2";
        case Action::HotbarSlot3: return "HotbarSlot3";
        case Action::HotbarSlot4: return "HotbarSlot4";
        case Action::OpenInventory: return "OpenInventory";
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
    bindKey(Action::Run, sf::Keyboard::Key::LShift);
    
    // NEW: Survival and combat bindings
    bindKey(Action::Attack, sf::Keyboard::Key::Space);
    bindKey(Action::ToggleFlashlight, sf::Keyboard::Key::F);
    bindKey(Action::Crouch, sf::Keyboard::Key::LControl);
    bindKey(Action::HotbarSlot1, sf::Keyboard::Key::Num1);
    bindKey(Action::HotbarSlot2, sf::Keyboard::Key::Num2);
    bindKey(Action::HotbarSlot3, sf::Keyboard::Key::Num3);
    bindKey(Action::HotbarSlot4, sf::Keyboard::Key::Num4);
    bindKey(Action::OpenInventory, sf::Keyboard::Key::Tab);

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
    update(event, 0.0f); // Call overloaded version with no delta time
}

void InputManager::update(const sf::Event& event, float deltaTime) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Update game time for history tracking
    gameTime_ += deltaTime;

    // Update currentKeys_ based on key press/release events (SFML 3 event API)
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            currentKeys_[kp->code] = true;
            // record last key for remapping UI
            lastKeyEvent_ = kp->code;
            hasLastKey_ = true;
            
            // Record input history if enabled
            if (inputHistoryEnabled_) {
                recordInputHistory(kp->code);
            }
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
            
            // Record input history if enabled
            if (inputHistoryEnabled_) {
                recordInputHistory(mbp->button);
            }
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
    
    // Use cache if available and valid
    if (actionCachingEnabled_ && cacheValid_) {
        auto it = cachedActionStates_.find(action);
        if (it != cachedActionStates_.end()) {
            return it->second;
        }
    }
    
    // Fallback to normal computation
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
    
    // Use cache if available and valid
    if (actionCachingEnabled_ && cacheValid_) {
        auto it = cachedJustPressedStates_.find(action);
        if (it != cachedJustPressedStates_.end()) {
            return it->second;
        }
    }
    
    // Fallback to normal computation
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
    
    // Use cache if available and valid
    if (actionCachingEnabled_ && cacheValid_) {
        auto it = cachedReleasedStates_.find(action);
        if (it != cachedReleasedStates_.end()) {
            return it->second;
        }
    }
    
    // Fallback to normal computation
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
    // Invalidate cache when bindings change
    cacheValid_ = false;
}

// Rebind (replace) all mouse buttons for an action
void InputManager::rebindMouse(Action action, const std::vector<sf::Mouse::Button>& buttons) {
    std::lock_guard<std::mutex> lock(mutex_);
    mouseBindings_[action] = buttons;
    // Invalidate cache when bindings change
    cacheValid_ = false;
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

// Serialization methods implementation
bool InputManager::saveBindings(const std::string& configPath) const {
    try {
        std::string jsonData = exportBindingsToJson();
        std::ofstream file(configPath);
        if (!file.is_open()) {
            core::Logger::instance().error("Failed to open config file for writing: " + configPath);
            return false;
        }
        file << jsonData;
        file.close();
        core::Logger::instance().info("Input bindings saved to: " + configPath);
        return true;
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error saving input bindings: " + std::string(e.what()));
        return false;
    }
}

bool InputManager::loadBindings(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            core::Logger::instance().warning("Config file not found, using default bindings: " + configPath);
            return false;
        }
        
        std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        bool result = importBindingsFromJson(jsonData);
        if (result) {
            core::Logger::instance().info("Input bindings loaded from: " + configPath);
        }
        return result;
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error loading input bindings: " + std::string(e.what()));
        return false;
    }
}

std::string InputManager::exportBindingsToJson() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    nlohmann::json j;
    
    // Export key bindings
    nlohmann::json keyBindings;
    for (const auto& [action, keys] : keyBindings_) {
        nlohmann::json actionKeys;
        for (const auto& key : keys) {
            actionKeys.push_back(static_cast<int>(key));
        }
        keyBindings[std::to_string(static_cast<int>(action))] = actionKeys;
    }
    j["keyBindings"] = keyBindings;
    
    // Export mouse bindings
    nlohmann::json mouseBindings;
    for (const auto& [action, buttons] : mouseBindings_) {
        nlohmann::json actionButtons;
        for (const auto& button : buttons) {
            actionButtons.push_back(static_cast<int>(button));
        }
        mouseBindings[std::to_string(static_cast<int>(action))] = actionButtons;
    }
    j["mouseBindings"] = mouseBindings;
    
    return j.dump(4);
}

bool InputManager::importBindingsFromJson(const std::string& jsonString) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto j = nlohmann::json::parse(jsonString);
        
        // Clear existing bindings
        keyBindings_.clear();
        mouseBindings_.clear();
        
        // Import key bindings
        if (j.contains("keyBindings")) {
            for (const auto& [actionStr, keys] : j["keyBindings"].items()) {
                int actionInt = std::stoi(actionStr);
                Action action = static_cast<Action>(actionInt);
                
                std::vector<sf::Keyboard::Key> keyList;
                for (const auto& keyInt : keys) {
                    keyList.push_back(static_cast<sf::Keyboard::Key>(keyInt.get<int>()));
                }
                keyBindings_[action] = keyList;
            }
        }
        
        // Import mouse bindings
        if (j.contains("mouseBindings")) {
            for (const auto& [actionStr, buttons] : j["mouseBindings"].items()) {
                int actionInt = std::stoi(actionStr);
                Action action = static_cast<Action>(actionInt);
                
                std::vector<sf::Mouse::Button> buttonList;
                for (const auto& buttonInt : buttons) {
                    buttonList.push_back(static_cast<sf::Mouse::Button>(buttonInt.get<int>()));
                }
                mouseBindings_[action] = buttonList;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        core::Logger::instance().error("Error parsing input bindings JSON: " + std::string(e.what()));
        return false;
    }
}

// Input history system implementation
void InputManager::enableInputHistory(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    inputHistoryEnabled_ = enable;
    if (!enable) {
        inputHistory_.clear();
    }
}

bool InputManager::isInputHistoryEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return inputHistoryEnabled_;
}

std::vector<std::pair<input::Action, float>> InputManager::getInputHistory(float timePeriod) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::pair<Action, float>> result;
    float cutoffTime = gameTime_ - timePeriod;
    
    for (const auto& entry : inputHistory_) {
        if (entry.timestamp >= cutoffTime) {
            result.emplace_back(entry.action, entry.timestamp);
        }
    }
    
    return result;
}

void InputManager::clearInputHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    inputHistory_.clear();
    actionUsageCount_.clear();
}

std::map<input::Action, int> InputManager::getActionUsageCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return actionUsageCount_;
}

// Private helper methods for input history
void InputManager::recordInputHistory(sf::Keyboard::Key key) {
    // Find which action this key is bound to
    for (const auto& [action, keys] : keyBindings_) {
        for (const auto& boundKey : keys) {
            if (boundKey == key) {
                inputHistory_.push_back({action, gameTime_});
                actionUsageCount_[action]++;
                // Limit history size to prevent memory growth
                if (inputHistory_.size() > 1000) {
                    inputHistory_.erase(inputHistory_.begin(), inputHistory_.begin() + 100);
                }
                return;
            }
        }
    }
}

void InputManager::recordInputHistory(sf::Mouse::Button button) {
    // Find which action this button is bound to
    for (const auto& [action, buttons] : mouseBindings_) {
        for (const auto& boundButton : buttons) {
            if (boundButton == button) {
                inputHistory_.push_back({action, gameTime_});
                actionUsageCount_[action]++;
                // Limit history size to prevent memory growth
                if (inputHistory_.size() > 1000) {
                    inputHistory_.erase(inputHistory_.begin(), inputHistory_.begin() + 100);
                }
                return;
            }
        }
    }
}

// Performance optimization implementation
void InputManager::enableActionCaching(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    actionCachingEnabled_ = enable;
    if (!enable) {
        cachedActionStates_.clear();
        cachedJustPressedStates_.clear();
        cachedReleasedStates_.clear();
        cacheValid_ = false;
    }
}

bool InputManager::isActionCachingEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return actionCachingEnabled_;
}

void InputManager::precomputeActionStates() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!actionCachingEnabled_) return;
    
    // Clear previous cache
    cachedActionStates_.clear();
    cachedJustPressedStates_.clear();
    cachedReleasedStates_.clear();
    
    // Precompute all action states
    std::vector<Action> allActions = {
        Action::MoveUp, Action::MoveDown, Action::MoveLeft, Action::MoveRight,
        Action::Confirm, Action::Cancel, Action::Interact, Action::Pause
    };
    
    for (Action action : allActions) {
        // Compute pressed state
        bool pressed = false;
        auto kIt = keyBindings_.find(action);
        if (kIt != keyBindings_.end()) {
            for (auto k : kIt->second) {
                auto it2 = currentKeys_.find(k);
                if (it2 != currentKeys_.end() && it2->second) {
                    pressed = true;
                    break;
                }
            }
        }
        if (!pressed) {
            auto mIt = mouseBindings_.find(action);
            if (mIt != mouseBindings_.end()) {
                for (auto b : mIt->second) {
                    auto itb = currentMouseButtons_.find(b);
                    if (itb != currentMouseButtons_.end() && itb->second) {
                        pressed = true;
                        break;
                    }
                }
            }
        }
        cachedActionStates_[action] = pressed;
        
        // Compute just pressed state
        bool justPressed = false;
        if (kIt != keyBindings_.end()) {
            for (auto k : kIt->second) {
                bool curr = false;
                bool prev = false;
                auto itCurr = currentKeys_.find(k);
                if (itCurr != currentKeys_.end()) curr = itCurr->second;
                auto itPrev = previousKeys_.find(k);
                if (itPrev != previousKeys_.end()) prev = itPrev->second;
                if (curr && !prev) {
                    justPressed = true;
                    break;
                }
            }
        }
        if (!justPressed) {
            auto mIt = mouseBindings_.find(action);
            if (mIt != mouseBindings_.end()) {
                for (auto b : mIt->second) {
                    bool curr = false;
                    bool prev = false;
                    auto itCurr = currentMouseButtons_.find(b);
                    if (itCurr != currentMouseButtons_.end()) curr = itCurr->second;
                    auto itPrev = previousMouseButtons_.find(b);
                    if (itPrev != previousMouseButtons_.end()) prev = itPrev->second;
                    if (curr && !prev) {
                        justPressed = true;
                        break;
                    }
                }
            }
        }
        cachedJustPressedStates_[action] = justPressed;
        
        // Compute released state
        bool released = false;
        if (kIt != keyBindings_.end()) {
            for (auto k : kIt->second) {
                bool curr = false;
                bool prev = false;
                auto itCurr = currentKeys_.find(k);
                if (itCurr != currentKeys_.end()) curr = itCurr->second;
                auto itPrev = previousKeys_.find(k);
                if (itPrev != previousKeys_.end()) prev = itPrev->second;
                if (!curr && prev) {
                    released = true;
                    break;
                }
            }
        }
        if (!released) {
            auto mIt = mouseBindings_.find(action);
            if (mIt != mouseBindings_.end()) {
                for (auto b : mIt->second) {
                    bool curr = false;
                    bool prev = false;
                    auto itCurr = currentMouseButtons_.find(b);
                    if (itCurr != currentMouseButtons_.end()) curr = itCurr->second;
                    auto itPrev = previousMouseButtons_.find(b);
                    if (itPrev != previousMouseButtons_.end()) prev = itPrev->second;
                    if (!curr && prev) {
                        released = true;
                        break;
                    }
                }
            }
        }
        cachedReleasedStates_[action] = released;
    }
    
    cacheValid_ = true;
}

void InputManager::invalidateActionCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    cacheValid_ = false;
}

} // namespace input
