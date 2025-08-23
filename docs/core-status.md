# Estado actual del Core (consolidado y actualizado con cambios recientes)

Resumen corto
- Rama principal de trabajo: `feature/save_config` (actual) — integra el sistema de escenas, el subsistema de input, la capa UI/menus y el nuevo módulo de guardado/configuración.
- Objetivo: documentar el estado del core tras añadir ConfigManager, SaveManager y la integración con `GameState` (serialización JSON), además de los cambios previos en UI/Input.

Checklist de actualizaciones aplicadas
- [x] Integración del sistema de escenas y UI/menus.
- [x] `InputManager` con multi-bindings y APIs de remapeo.
- [x] Nuevos módulos: `ConfigManager` y `SaveManager` (serialización JSON con `nlohmann::json`).
- [x] `GameState` serializable y helpers `scene::gatherGameState` / `scene::applyGameState`.
- [x] Logs y fallbacks explícitos para config/save (mensajes `[config]` y `[save]`).

Archivos revisados / añadidos
- `src/core/ConfigManager.h` / `src/core/ConfigManager.cpp` — lectura/escritura de `config/config.json`, defaults y `applyConfig(UIManager&)`.
- `src/core/SaveManager.h` / `src/core/SaveManager.cpp` — guardado y carga de `saves/*.json` con logs `[save]` / `[load]`.
- `src/core/GameState.h` — estructura serializable para jugadores, items y puzzles (con `to_json`/`from_json`).
- `src/scene/SaveIntegration.h` / `src/scene/SaveIntegration.cpp` — helpers para mapear entre managers (EntityManager/ItemManager/PuzzleManager) y `GameState`.
- Modificaciones en managers: `EntityManager::allEntities()`, `ItemManager::allItems()`, `PuzzleManager::allPuzzles()` (accesores no propietarios para integración/serialización).
- `build.ps1` actualizado para instalar `nlohmann-json` via vcpkg si falta.
- `config/config.json` y `saves/example_save.json` añadidos como ejemplos.

Visión general del core (resumen actualizado)

1) Game / Scenes / UI
- `Game` mantiene el bucle principal y el `SceneManager`.
- `MenuScene` y `PlayScene` mantienen su `UIManager` y usan menús (`MainMenu`, `PauseMenu`, `OptionsMenu`).
- `OptionsMenu` soporta ajuste de volumen y remapeo de teclas (usa `InputManager::rebindKeys` / `rebindMouse`).

2) Configuración y guardado (nuevo)
- `ConfigManager`:
  - Lee/escribe `config/config.json` usando `nlohmann::json`.
  - Defaults: volume=100, language="es", resolution=1920x1080, notificationDuration=3.0.
  - Si el archivo no existe o hay error → `setDefaults()` y registro: "[config] Usando valores por defecto; no se encontró: <ruta>".
  - `applyConfig(UIManager&)` aplica parámetros de UI (ej. notificationDuration) y registra la acción.
- `SaveManager`:
  - Guarda y carga `saves/<filename>` con `core::GameState`.
  - Si falta el archivo al cargar registra: "[save] Archivo no encontrado: <ruta>" y retorna false sin lanzar.
  - Logs claros en éxito/error: `[save] Saved game to ...`, `[load] Loaded game from ...`, y mensajes de error en excepciones.

3) Serialización de estado
- `GameState` contiene:
  - `players[]` (id, x, y, health), `itemsCollected[]` (ids), `puzzlesCompleted[]` (ids).
- `scene::gatherGameState` y `scene::applyGameState` implementan la extracción/restauración desde `EntityManager`, `ItemManager`, `PuzzleManager`.
- Nota: la restauración usa las APIs públicas existentes (p. ej. `Item::interact` para marcar un item como recogido cuando no existe setter directo). Recomendado: exponer setters directos (`Player::setHealth`, `Item::setCollected`, `Puzzle::forceComplete`) para una recuperación más exacta.

4) APIs de entrada y remapeo
- `InputManager` mantiene multi-bindings, expone remapeo en runtime y helpers usados por `OptionsMenu` para mostrar bindings y remapear mediante eventos crudos.

5) Logs y fallbacks
- Mensajes añadidos y estandarizados:
  - Config: `[config] Usando valores por defecto; no se encontró: <ruta>` y `[config] Error al cargar config: <ex>`.
  - Save: `[save] Archivo no encontrado: <ruta>`, `[save] Saved game to <ruta>`, `[load] Loaded game from <ruta>`, `[load] Error al cargar save: <ex>`.
- En todos los casos de error de lectura se aplica un fallback seguro (defaults o no modificar estado) y se registra.

6) Build / dependencias
- `build.ps1` ahora instala `nlohmann-json` vía vcpkg si no está presente.
- `CMakeLists.txt` fue actualizado para compilar los nuevos sources (`ConfigManager.cpp`, `SaveManager.cpp`, `SaveIntegration.cpp`, etc.).
- El proyecto compila (verificado vía `build.ps1`) y el ejecutable se genera: `build/Release/AbyssalStation.exe` (BUILD PASS).

Archivos añadidos de ejemplo
- `config/config.json` — ejemplo con `version` y campos de audio/bindings/resolution/notificationDuration.
- `saves/example_save.json` — ejemplo de save con `version`, `meta` y arrays de players/items/puzzles.

Calidad y pruebas rápidas
- Build: OK (se ejecutó `build.ps1`, se añadió la instalación de `nlohmann-json` y el proyecto compila).
- Tests: no se añadieron tests automáticos en esta iteración.
- Smoke test manual: menú y escenas funcionan; los helpers de save/config están incluidos pero la verificación runtime completa (restore exacto de salud/items/puzzles) requiere exponer métodos directos en algunas clases para ser totalmente determinista.

Recomendaciones y próximos pasos
- Exponer setters públicos para restauración exacta: `Player::setHealth`, `Item::setCollected`, `Puzzle::forceComplete`.
- Integrar `OptionsMenu::applyVolume()` con un `AudioManager` central (o con SFML audio wrappers) y persistir en `ConfigManager`.
- Persistir bindings en `ConfigManager` (campo `keyBindings`) y reaplicar con `InputManager` en `loadConfig()`.
- Añadir pruebas unitarias para serialización `GameState` (round-trip JSON) y tests para `SaveManager` y `ConfigManager`.

Resumen corto
- El core incluye ahora un módulo de config/save bien integrado (JSON/nlohmann), helpers para mapear estado y logs/fallbacks robustos; la UI y el input siguen funcionando como antes. La compilación está verificada y los ejemplos `config/config.json` y `saves/example_save.json` están incluidos en el repo para referencia.

Fin del informe.
