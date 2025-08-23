# Estado actual del Core (consolidado y actualizado con cambios recientes)

Resumen corto
- Rama principal de trabajo: `feature/save_config` (actual) — integra el sistema de escenas, el subsistema de input, la capa UI/menus y el nuevo módulo de guardado/configuración.
- Objetivo: documentar el estado del core tras añadir `ConfigManager`, `SaveManager`, `GameState` (serialización JSON con `nlohmann::json`) y las integraciones posteriores con `InputManager` y `OptionsMenu` (persistencia de bindings y remapeo en runtime).

Checklist de actualizaciones aplicadas
- [x] Integración del sistema de escenas y UI/menus.
- [x] `InputManager` con multi-bindings, remapeo en runtime y serialización (save/load/export/import).
- [x] Nuevos módulos: `ConfigManager` y `SaveManager` (serialización JSON con `nlohmann::json`).
- [x] `GameState` serializable y helpers `scene::gatherGameState` / `scene::applyGameState`.
- [x] Logs y fallbacks explícitos para config/save (mensajes `[config]` y `[save]`).
- [x] Integración: `Game` carga bindings al iniciar y guarda bindings al cerrar (persistencia en `config/input_bindings.json`).
- [x] Pruebas unitarias añadidas (Google Test) y suite de tests para `InputManager`.

Archivos revisados / añadidos
- `src/core/ConfigManager.h` / `src/core/ConfigManager.cpp` — lectura/escritura de `config/config.json`, defaults y `applyConfig(UIManager&)`.
- `src/core/SaveManager.h` / `src/core/SaveManager.cpp` — guardado y carga de `saves/*.json` con logs `[save]` / `[load]`.
- `src/core/GameState.h` — estructura serializable para jugadores, items y puzzles (con `to_json`/`from_json`).
- `src/scene/SaveIntegration.h` / `src/scene/SaveIntegration.cpp` — helpers para mapear entre managers (EntityManager/ItemManager/PuzzleManager) y `GameState`.
- `src/core/Game.cpp` — ahora intenta cargar bindings desde `config/input_bindings.json` al iniciar y guarda bindings al cerrar; se integra con `InputManager`.
- `src/input/InputManager.h` / `src/input/InputManager.cpp` — extensiones: serialización (save/load/export/import), historial de inputs, caching de estados y APIs para remapeo en runtime.
- `src/ui/OptionsMenu.h` / `src/ui/OptionsMenu.cpp` — muestra bindings actuales, permite remapear con captura de eventos y guarda cambios inmediatamente.
- Modificaciones en managers: `EntityManager::allEntities()`, `ItemManager::allItems()`, `PuzzleManager::allPuzzles()` (accesores no propietarios para integración/serialización).
- `build.ps1` actualizado para instalar dependencias vía vcpkg (`nlohmann-json`, `gtest`) si falta.
- Nuevos tests en `tests/` (Google Test): `InputManager` tests y runner.
- `config/config.json`, `config/input_bindings.json` (persisted), y `saves/example_save.json` añadidos como ejemplos.

Visión general del core (resumen actualizado)

1) Game / Scenes / UI
- `Game` mantiene el bucle principal y el `SceneManager`.
- `MenuScene` y `PlayScene` mantienen su `UIManager` y usan menús (`MainMenu`, `PauseMenu`, `OptionsMenu`).
- `OptionsMenu` soporta ajuste de volumen y remapeo de teclas (usa `InputManager::rebindKeys` / `rebindMouse`) y muestra los bindings actuales en tiempo real.

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
- Nota: la restauración usa las APIs públicas existentes. Para restauraciones exactas se recomienda exponer setters directos (`Player::setHealth`, `Item::setCollected`, `Puzzle::forceComplete`).

4) APIs de entrada, persistencia y remapeo
- `InputManager` ahora soporta:
  - Serialización de bindings (`saveBindings`, `loadBindings`, `exportBindingsToJson`, `importBindingsFromJson`).
  - Registro del último evento para remapeo en runtime (`getLastKeyEvent`, `getLastMouseButtonEvent`).
  - Historial de inputs y conteo de uso por acción (opcional, habilitable).
  - Cache de estados por frame (precompute) para optimizar consultas repetidas.
- Integración práctica:
  - `Game` carga `config/input_bindings.json` al iniciar si existe y guarda bindings al cerrar.
  - `OptionsMenu` permite remapear y persiste inmediatamente los cambios a `config/input_bindings.json`.

5) Logs y fallbacks
- Mensajes añadidos y estandarizados:
  - Config: `[config] Usando valores por defecto; no se encontró: <ruta>` y `[config] Error al cargar config: <ex>`.
  - Save: `[save] Archivo no encontrado: <ruta>`, `[save] Saved game to <ruta>`, `[load] Loaded game from <ruta>`, `[load] Error al cargar save: <ex>`.
  - Input: logs de bindings por defecto y mensajes en load/save de bindings.
- En todos los casos de error de lectura se aplica un fallback seguro (defaults o no modificar estado) y se registra.

6) Build / dependencias
- `build.ps1` ahora instala `nlohmann-json` y `gtest` vía vcpkg si no están presentes.
- `CMakeLists.txt` fue actualizado para compilar los nuevos sources y para exponer la suite de tests (Google Test).
- El proyecto compila y se generó el ejecutable en mi verificación local: `build/Release/AbyssalStation.exe`.

Archivos añadidos de ejemplo
- `config/config.json` — ejemplo con `version` y campos de audio/bindings/resolution/notificationDuration.
- `config/input_bindings.json` — fichero de persistencia de bindings usado por `Game` y `OptionsMenu`.
- `saves/example_save.json` — ejemplo de save con `version`, `meta` y arrays de players/items/puzzles.

Calidad y pruebas rápidas
- Build: OK (se ejecutó `build.ps1`, se instalaron dependencias si era necesario y el proyecto compila).
- Tests: se añadió una suite de tests (Google Test) centrada en `InputManager` y la serialización de bindings; los tests fueron ejecutados localmente y pasaron en la verificación.
- Smoke test manual: menú y escenas funcionan; la navegación y el remapeo via `OptionsMenu` han sido verificados; la restauración exacta de estado (salud/items/puzzles) requiere exponer setters directos para ser totalmente determinista.

Recomendaciones y próximos pasos
- Exponer setters públicos en entidades para restauración exacta: `Player::setHealth`, `Item::setCollected`, `Puzzle::forceComplete`.
- Integrar `OptionsMenu::applyVolume()` con un `AudioManager` central y persistir la preferencia vía `ConfigManager`.
- Añadir pruebas de integración que cubran el flujo: `OptionsMenu` → `InputManager` → `ConfigManager` (guardar/reaplicar bindings) y `SaveManager` round-trip.
- Considerar empaquetado/distribución si se requiere un artefacto instalable (ZIP/installer).

Resumen corto
- El core incluye ahora un módulo de config/save bien integrado (JSON/nlohmann), integración de persistencia y remapeo de bindings en `InputManager`/`OptionsMenu`, tests unitarios para input/serialización, y logs/fallbacks robustos; la compilación está verificada.

Fin del informe.
