#ifndef ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H
#define ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H

#include "Action.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>

namespace input {

// InputManager centraliza el mapeo de teclas a acciones y su estado.
// Diseñado para usarse como singleton compartido por las escenas.
class InputManager {
public:
    // Obtiene la instancia singleton (thread-safe en C++11+).
    static InputManager& getInstance();

    // Asocia una acción a una tecla (sobrescribe binding previo).
    void bindKey(Action action, sf::Keyboard::Key key);
    // Asocia una acción a un botón del ratón.
    void bindMouse(Action action, sf::Mouse::Button button);
    // Rebind (replace) all keys for an action
    void rebindKeys(Action action, const std::vector<sf::Keyboard::Key>& keys);
    // Rebind (replace) all mouse buttons for an action
    void rebindMouse(Action action, const std::vector<sf::Mouse::Button>& buttons);

    // Helpers to read the last raw input event captured (for remapping UI)
    // Returns optional-like indicator: pair<bool, Key> where bool indicates presence.
    std::pair<bool, sf::Keyboard::Key> getLastKeyEvent() const;
    std::pair<bool, sf::Mouse::Button> getLastMouseButtonEvent() const;
    // Clear last recorded raw events after consuming them
    void clearLastEvents();

    // Expose current bindings for UI display
    std::vector<sf::Keyboard::Key> getKeyBindings(Action action) const;
    std::vector<sf::Mouse::Button> getMouseBindings(Action action) const;
    // Human readable name for the first binding of an action (e.g. "W", "Left", "Mouse Left")
    std::string getBindingName(Action action) const;

    // Serialization methods for saving/loading bindings configuration
    bool saveBindings(const std::string& configPath) const;
    bool loadBindings(const std::string& configPath);
    // Export/import bindings as JSON string (useful for testing and UI)
    std::string exportBindingsToJson() const;
    bool importBindingsFromJson(const std::string& jsonString);

    // Input history and analysis methods
    void enableInputHistory(bool enable = true);
    bool isInputHistoryEnabled() const;
    std::vector<std::pair<Action, float>> getInputHistory(float timePeriod = 5.0f) const;
    void clearInputHistory();
    // Get action usage statistics (useful for UI tutorials and hints)
    std::map<Action, int> getActionUsageCount() const;

    // Performance optimization methods
    void enableActionCaching(bool enable = true);
    bool isActionCachingEnabled() const;
    void precomputeActionStates(); // Call once per frame for optimization
    void invalidateActionCache(); // Call when bindings change

    // Consultas de estado por acción (usa los bindings actuales).
    bool isActionPressed(Action action) const;
    bool isActionJustPressed(Action action) const;
    bool isActionReleased(Action action) const;

    // Actualiza el estado interno a partir de un evento SFML.
    // Debe llamarse desde el bucle de eventos (por ejemplo, desde Game).
    void update(const sf::Event& event);

    // Update method with delta time for input history tracking
    void update(const sf::Event& event, float deltaTime);

    // Call once per frame after processing all events to advance the input frame
    void endFrame();

private:
    InputManager();
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Mapeo acción -> lista de teclas (soporta múltiples bindings por acción)
    std::unordered_map<Action, std::vector<sf::Keyboard::Key>> keyBindings_;

    // Estado actual y previo de teclas (true = pressed)
    std::unordered_map<sf::Keyboard::Key, bool> currentKeys_;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeys_;
    // Estado actual y previo de botones de ratón
    std::unordered_map<sf::Mouse::Button, bool> currentMouseButtons_;
    std::unordered_map<sf::Mouse::Button, bool> previousMouseButtons_;
    // Mapeo acción -> lista de botones de ratón
    std::unordered_map<Action, std::vector<sf::Mouse::Button>> mouseBindings_;
    // Store last raw events captured during update() to help remapping UI
    sf::Keyboard::Key lastKeyEvent_{sf::Keyboard::Key::Unknown};
    bool hasLastKey_{false};
    sf::Mouse::Button lastMouseEvent_{sf::Mouse::Button::Left};
    bool hasLastMouse_{false};

    // Input history system
    struct InputHistoryEntry {
        Action action;
        float timestamp;
    };
    bool inputHistoryEnabled_{false};
    std::vector<InputHistoryEntry> inputHistory_;
    std::map<Action, int> actionUsageCount_;
    float gameTime_{0.0f}; // Track game time for history timestamps

    // Performance optimization - action state caching
    bool actionCachingEnabled_{false};
    std::unordered_map<Action, bool> cachedActionStates_;
    std::unordered_map<Action, bool> cachedJustPressedStates_;
    std::unordered_map<Action, bool> cachedReleasedStates_;
    bool cacheValid_{false};

    // Helper methods for input history
    void recordInputHistory(sf::Keyboard::Key key);
    void recordInputHistory(sf::Mouse::Button button);

    // Protege accesos concurrentes si se accede desde varios hilos.
    mutable std::mutex mutex_;
};

} // namespace input

#endif // ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H
