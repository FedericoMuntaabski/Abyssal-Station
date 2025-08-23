# Estado actual del módulo `scene` (actualizado)

Resumen corto
- Objetivo: Documentar el estado actual de `src/scene` tras las últimas iteraciones: gestor de escenas, integración con el subsistema de UI/menus (`UIManager` y menús concretos), mejoras en `PlayScene` para soportar interacción con objetos, y nuevas utilidades para persistencia (`SaveIntegration`).
- Estado general: Implementación funcional y probada localmente. `MenuScene`, `PlayScene`, `DebugPickupScene`, `SceneManager` y `SaveIntegration` operan correctamente a nivel API. `PlayScene` detecta items cercanos, muestra la indicación de interacción y usa `ItemManager` para ejecutar efectos.

Checklist de adaptación
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Documentar `MenuScene`, `PlayScene` y `DebugPickupScene` con integración a `UIManager` y `InputManager` actualizado.
- [x] Documentar `SaveIntegration` y su relación con `core::GameState`, `SaveManager` y `ConfigManager`.

Archivos relevantes (actual)
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp`
- `src/scene/DebugPickupScene.h` / `src/scene/DebugPickupScene.cpp` (escena auxiliar para probar pickups)
- `src/scene/SaveIntegration.h` / `src/scene/SaveIntegration.cpp` (helpers para guardar/restaurar `core::GameState`)

Visión general
El módulo `scene` mantiene la abstracción de pantallas con una pila LIFO. En esta iteración, además de reforzar la integración con el sistema de UI, se añadió `SaveIntegration` —un conjunto de helpers para mapear entre el estado en tiempo de ejecución (managers) y `core::GameState` serializable— y se introdujeron cambios menores para facilitar la restauración de estado.

1) `Scene` (interfaz)
- Ubicación: `src/scene/Scene.h`.
- Contrato resumido:
  - virtual ~Scene() = default;
  - virtual void handleEvent(sf::Event& event) = 0;
  - virtual void update(float dt) = 0;
  - virtual void render(sf::RenderWindow& window) = 0;
  - virtual void onEnter() {}
  - virtual void onExit() {}

2) `SceneManager`
- Ubicación: `src/scene/SceneManager.{h,cpp}`.
- Comportamiento: mantiene la pila de escenas (`std::stack<std::unique_ptr<Scene>>`) y delega `handleEvent`, `update` y `render` a la escena actual. Todas las transiciones (push/pop/replace) registran trazas en `core::Logger`.

3) `MenuScene`
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento:
  - En `onEnter()` crea o reutiliza un `UIManager` y empuja el `MainMenu`.
  - Durante el loop, `MenuScene` delega eventos y render al `UIManager`.
  - Usa `core::FontHelper` para cargar fuentes y fallbacks.

4) `PlayScene` (novedades y flujo)
- Ubicación: `src/scene/PlayScene.*`.
- Componentes inicializados en `onEnter()` (resumen):
  - `entities::EntityManager`
  - `collisions::CollisionManager`
  - `collisions::CollisionSystem`
  - `ai::EnemyManager`
  - `gameplay::ItemManager` / `PuzzleManager`
  - `UIManager` (para menús en juego)

- Flujo principal en `update(dt)` (resumen):
  1. Si hay menús activos, `UIManager` consume eventos y la UI bloquea la lógica de juego.
  2. Si no, `PlayScene` procesa input (via `InputManager`), planifica IA, valida colisiones y aplica `commitMove` en entidades.
  3. Detección e interacción con items: `PlayScene` detecta items cercanos y muestra una indicación; al pulsar `Action::Interact` se delega a `ItemManager`.
  4. `DebugPickupScene` sirve como caso de prueba para spawn/colección de items.

- Nota: la indicación de interacción está renderizada actualmente por `PlayScene`. Recomendación: migrarla a `UIManager` para mantener overlays coherentes.

5) `DebugPickupScene`
- Escena reducida para probar pickups y mostrar flujos de interacción sin la lógica completa de `PlayScene`.

6) `SaveIntegration` (nuevas utilidades)
- Ubicación: `src/scene/SaveIntegration.h` / `src/scene/SaveIntegration.cpp`.
- Propósito: convertir el estado en tiempo de ejecución a `core::GameState` y viceversa.
- Funciones principales:
  - `scene::gatherGameState(EntityManager&, ItemManager&, PuzzleManager&) -> core::GameState` — construye un snapshot serializable.
  - `scene::applyGameState(const core::GameState&, EntityManager&, ItemManager&, PuzzleManager&)` — aplica un snapshot al estado en memoria.
- Dependencias: hace uso de los accesores añadidos a los managers (`allEntities()`, `allItems()`, `allPuzzles()`).
- Notas operativas:
  - Cuando faltan setters públicos para restaurar campos (por ejemplo salud, flags de item o puzzles), `applyGameState` realiza acciones de "mejor esfuerzo" (ej. llamar a `Item::interact()` para marcar un item como recogido). Esto funciona para demos, pero se recomienda exponer setters directos en las entidades para restauraciones deterministas.

Integración con `Game`, `ConfigManager` y `SaveManager`
- `Game::run()` crea el `SceneManager`, carga la configuración inicial mediante `core::ConfigManager::loadConfig()` y aplica ajustes a la UI/audio (`applyConfig(UIManager&)`) antes de empujar `MenuScene`.
- Guardado y carga de partidas:
  - `SaveIntegration` produce/consume `core::GameState`.
  - `core::SaveManager` (en `src/core`) serializa `GameState` a `saves/*.json` y verifica existencia antes de load.
  - Las escenas o el UI deben invocar `scene::gatherGameState(...)` + `SaveManager::saveGame(...)` para persistir, y `SaveManager::loadGame(...)` + `scene::applyGameState(...)` para restaurar.

Logging y debugging
- Todas las transiciones y eventos relevantes usan `core::Logger` (entradas/salidas de escena, guardados/cargas, errores y fallbacks).
- Mensajes clave:
  - `[config] Usando valores por defecto; no se encontró: <ruta>` — cuando `config/config.json` no existe.
  - `[save] Archivo no encontrado: <ruta>` — intento de cargar un save inexistente.

Buenas prácticas y recomendaciones
- Mantener escenas pequeñas y responsables de flujo/autoridad (la lógica de juego debe residir en `PlayScene` y managers).
- No cargar assets directamente en escenas; usar `AssetManager`.
- Prioritario: exponer setters para restauración exacta (`Player::setHealth`, `Item::setCollected(bool)`, `Puzzle::forceComplete()`) y actualizar `SaveIntegration` para usar esos setters en lugar de heurísticas.
- Añadir wiring UI: entradas en `MenuScene`/`OptionsMenu` para guardar/cargar partidas y editar/configurar `ConfigManager`.

Próximos pasos (pendientes)
- Añadir setters deterministas en entidades para que `applyGameState` pueda restaurar exactamente el estado serializado.
- Añadir botones/entradas en menus para Save/Load usando `SaveManager` + `SaveIntegration`.
- Crear tests de integración que verifiquen round-trip `gather -> save -> load -> apply`.

Estado del build y verificación
- El módulo compila y las utilidades de `SaveIntegration` están incluidas en el proyecto. Se realizaron pruebas API-level de guardado y carga; la restauración completa depende de exponer API adicionales en entidades.

Fin del `scene-status`.
