---
applyTo: 'src/input/**'
---

# Módulo: InputManager
Objetivo: centralizar y mapear eventos de entrada (teclado, ratón, mando) a acciones del juego.
Lenguaje: C++
Librería: SFML

## Estructura de archivos
- src/input/InputManager.h
- src/input/InputManager.cpp
- src/input/Action.h (definición enum de acciones)

## Requisitos
1. Definir un `enum class Action` con acciones básicas:
   - MoveUp, MoveDown, MoveLeft, MoveRight
   - Confirm, Cancel, Pause
2. `InputManager` debe:
   - Mapear teclas a acciones (ej: W → MoveUp).
   - Permitir `bindKey(Action, sf::Keyboard::Key)`.
   - Proveer consultas:
     - `bool isActionPressed(Action)`
     - `bool isActionJustPressed(Action)`
     - `bool isActionReleased(Action)`
   - Actualizar su estado con `update(const sf::Event&)`.

## Uso esperado
- En escenas (`MenuScene`, `PlayScene`), se consulta a `InputManager` en lugar de leer teclas crudas.
- Facilitar cambios rápidos de mapeo sin tocar el código de la escena.

## Diseño
- Patrón Singleton o inyectar instancia en `Game` para compartirlo.
- Guardar un mapa `std::unordered_map<Action, sf::Keyboard::Key>`.
- Guardar estados anteriores y actuales de teclas para detectar transiciones.

## Ejemplo de integración
```cpp
if (inputManager.isActionPressed(Action::MoveUp)) {
    player.move(0, -speed * dt);
}

# Instrucciones para Copilot

Generar enum Action y clase InputManager con la API definida.

Sugerir bindings por defecto (WASD + Enter + Escape).

Proponer tests simples: imprimir acciones activadas en consola.

Mantener consistencia con Logger para debug (ej: loggear cambios de bindings).

Preparar la clase para posible extensión a gamepads.