---
applyTo: 'src/scene/**'
---

# Instrucciones para el Módulo de Escenas

## Objetivo
Diseñar un sistema modular de escenas para separar la lógica del juego del núcleo (`Game`).  
Este módulo debe permitir tener distintas pantallas (ejemplo: menú principal, juego, pausa) y manejar la transición entre ellas de forma ordenada.

## Estructura esperada
- `src/scene/Scene.h` → Clase abstracta base (interfaz).
- `src/scene/SceneManager.h / .cpp` → Gestor de escenas (pila interna).
- `src/scene/MenuScene.h / .cpp` → Ejemplo simple de escena.
- `src/scene/PlayScene.h / .cpp` → Ejemplo simple de escena jugable.

## Reglas de diseño
1. **Interfaz `Scene`**
   - Métodos virtuales puros: `handleEvent(sf::Event&)`, `update(float dt)`, `render(sf::RenderWindow&)`.
   - Métodos opcionales: `onEnter()`, `onExit()` (para inicializar o limpiar recursos).

2. **`SceneManager`**
   - Usa una `std::stack<std::unique_ptr<Scene>>`.
   - Métodos:
     - `push(std::unique_ptr<Scene>)`
     - `pop()`
     - `replace(std::unique_ptr<Scene>)`
     - `current()` → devuelve puntero a la escena activa.
   - Responsable de delegar `update`, `render` y `handleEvent` a la escena actual.

3. **Escenas concretas**
   - `MenuScene`: Renderiza un texto con "Presione Enter para jugar". Si detecta `Enter`, hace `replace(PlayScene)`.
   - `PlayScene`: Renderiza un rectángulo o sprite de prueba. Si detecta `Escape`, hace `replace(MenuScene)`.

4. **Integración con `Game`**
   - El bucle principal (`Game::run`) debe delegar en `SceneManager`:
     - Eventos → `currentScene->handleEvent(e)`
     - Update → `currentScene->update(dt)`
     - Render → `currentScene->render(window)`

5. **Logging y Debug**
   - Todas las transiciones de escenas deben registrar logs con `core::Logger`.

## Buenas prácticas
- Mantener escenas pequeñas y autocontenidas.
- No cargar assets directamente en `Scene`; usar `AssetManager`.
- Evitar lógica de juego en `Game`; todo debe pasar por escenas.

## Entregables
- Implementar `Scene` base.
- Implementar `SceneManager`.
- Implementar `MenuScene` y `PlayScene` como ejemplos.
- Integrar con `Game` para que se pueda iniciar en `MenuScene`.
- Documentar en un archivo `docs/scene-status.md` el estado del módulo.
