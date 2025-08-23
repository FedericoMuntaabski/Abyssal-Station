#ifndef ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H
#define ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H

#include "Action.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <unordered_map>
#include <mutex>

namespace input {

// InputManager centraliza el mapeo de teclas a acciones y su estado.
// Diseñado para usarse como singleton compartido por las escenas.
// TODO: Extender para soportar joysticks/gamepads (mapear ejes y botones a Action).
class InputManager {
public:
    // Obtiene la instancia singleton (thread-safe en C++11+).
    static InputManager& getInstance();

    // Asocia una acción a una tecla (sobrescribe binding previo).
    void bindKey(Action action, sf::Keyboard::Key key);

    // Consultas de estado por acción (usa los bindings actuales).
    bool isActionPressed(Action action) const;
    bool isActionJustPressed(Action action) const;
    bool isActionReleased(Action action) const;

    // Actualiza el estado interno a partir de un evento SFML.
    // Debe llamarse desde el bucle de eventos (por ejemplo, desde Game).
    void update(const sf::Event& event);

private:
    InputManager();
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Mapeo acción -> tecla
    std::unordered_map<Action, sf::Keyboard::Key> keyBindings_;

    // Estado actual y previo de teclas (true = pressed)
    std::unordered_map<sf::Keyboard::Key, bool> currentKeys_;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeys_;

    // Protege accesos concurrentes si se accede desde varios hilos.
    mutable std::mutex mutex_;
};

} // namespace input

#endif // ABYSSAL_STATION_SRC_INPUT_INPUTMANAGER_H
