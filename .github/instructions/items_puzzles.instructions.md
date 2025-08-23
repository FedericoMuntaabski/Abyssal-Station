---
applyTo: "src/gameplay/**"
---

# Módulo: Objetos / Items / Puzzles

## Objetivo
Implementar un sistema modular de objetos interactivos y puzzles cooperativos dentro del juego:
- Objetos que el jugador puede recoger o activar.
- Puzzles que requieren interacción de uno o varios jugadores.
- Integración con **Entities**, **Collisions**, **Scenes** y **UI**.

## Estructura de archivos
- `src/gameplay/Item.h / Item.cpp` → Clase base para cualquier objeto interactivo.
- `src/gameplay/Puzzle.h / Puzzle.cpp` → Clase base para puzzles y mecanismos.
- `src/gameplay/ItemManager.h / ItemManager.cpp` → Contenedor de items activos.
- `src/gameplay/PuzzleManager.h / PuzzleManager.cpp` → Contenedor de puzzles activos.

## Clases principales

### Item
- Atributos: id, posición (`sf::Vector2f`), tipo (enum: Key, Tool, Collectible).
- Métodos: `interact(Player&)`, `update(float dt)`, `render(sf::RenderWindow&)`.
- Estado: disponible o recogido.

### Puzzle
- Atributos: id, lista de pasos/objetivos, estado (Locked, Active, Completed).
- Métodos: `update(float dt)`, `render(sf::RenderWindow&)`, `checkCompletion()`.
- Integración con Items: algunos puzzles requieren objetos específicos.

### Managers
- `ItemManager`: agregar, eliminar, actualizar y renderizar items.
- `PuzzleManager`: registrar puzzles, actualizar su estado y notificar completion.

## Reglas y consideraciones
- Los objetos deben usar Collisions para detectar interacción con el jugador.
- Los puzzles deben poder ser activados por múltiples jugadores si el juego es multijugador.
- Registrar eventos importantes con `Logger` (ej: item recogido, puzzle completado).

## Ejemplo de flujo
1. Jugador se acerca a un objeto “Key”.
2. `CollisionManager` detecta colisión.
3. `interact()` recoge el objeto, actualiza inventario.
4. Jugador llega al puzzle que requiere esa Key.
5. `checkCompletion()` actualiza estado a Completed y loggea el evento.
6. UIManager refleja el cambio en pantalla (opcional).

## Instrucciones para Copilot
- Crear clases modulares `Item`, `Puzzle`, `ItemManager`, `PuzzleManager`.
- Implementar métodos `interact`, `update`, `render`.
- Preparar el sistema para soporte multijugador (múltiples jugadores interactuando).
- Incluir logs de eventos importantes con `Logger`.
- Mostrar ejemplos de integración con PlayScene y EntityManager.
