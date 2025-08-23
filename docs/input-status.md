# Estado del módulo `input`

Resumen corto
- Ubicación: `src/input`.
- Objetivo: centralizar y mapear entradas (teclado, futuro gamepad) a acciones del juego y ofrecer consultas por acción para las escenas.
- Estado general: Implementado y mejorado en esta iteración para soportar detección per-frame correcta y multi-bindings; integrado con `Game`, `Scene`s y el nuevo módulo de UI/menus.

Checklist rápido
- [x] `Action` enum definido (`MoveUp/MoveDown/MoveLeft/MoveRight/Confirm/Cancel/Pause`).
- [x] `InputManager` (singleton) con API pública: `getInstance()`, `bindKey`, `isActionPressed`, `isActionJustPressed`, `isActionReleased`, `update(const sf::Event&)`, `endFrame()`.
- [x] Bindings por defecto: WASD, Enter, Escape, P (se registran por `core::Logger`).
 - [x] Nuevo `Action::Interact` añadido y enlazado por defecto a la tecla `E` (se usa en `PlayScene` para recoger/activar objetos).
- [x] `bindKey` permite múltiples bindings por acción (se añaden al vector de teclas en lugar de reemplazar).
- [x] Integración: `Game::processEvents()` reenvía eventos a `InputManager`; `Game::run()` llama a `InputManager::endFrame()` una vez por frame.
 - [x] API de consulta y utilidades expuestas: `getBindingName(Action)` devuelve una etiqueta legible para UI (p. ej. "E" o "Left Ctrl") y es usada por `PlayScene`/`UIManager` para mostrar la indicación de interacción.
 - [x] `bindKey` permite múltiples bindings por acción (se añaden al vector de teclas en lugar de reemplazar).
 - [x] Soporte de mouse: `InputManager` ahora maneja botones de ratón (press/release) y puede mapear acciones a botones de mouse.
 - [x] Exposición de APIs de remapeo y consulta: `rebindKeys`, `rebindMouse`, `getLastKeyEvent`, `getLastMouseButtonEvent`, `getBindingName`, `clearLastEvents`.
 - [x] Integración: `Game::processEvents()` reenvía eventos a `InputManager`; `Game::run()` llama a `InputManager::endFrame()` una vez por frame (el código actual toma un snapshot del estado previo ANTES de procesar los eventos del frame para corregir la semántica de "just pressed").

Archivos en `src/input`
# Estado del módulo `input`

Resumen corto
- Ubicación: `src/input`.
- Objetivo: centralizar y mapear entradas (teclado, ratón y futuro gamepad) a acciones del juego y ofrecer consultas por acción para las escenas y la UI.
- Estado general: Implementado y mejorado en esta iteración. Se añadieron serialización de bindings, interfaz de remapeo en tiempo de ejecución, pruebas unitarias, historial de inputs y cache de estados para optimización.

Checklist rápido (status)
- [x] `Action` enum definido (incluye: MoveUp/MoveDown/MoveLeft/MoveRight/Confirm/Cancel/Interact/Pause).
- [x] `InputManager` (singleton) con API pública: `getInstance()`, `bindKey`, `bindMouse`, `rebindKeys`, `rebindMouse`, `isActionPressed`, `isActionJustPressed`, `isActionReleased`, `update(const sf::Event&)`, `endFrame()`.
- [x] Bindings por defecto: WASD + flechas, Enter (Confirm), Escape (Cancel), P (Pause), E (Interact); Left Mouse = Confirm.
- [x] Multi-bindings por acción (vector de teclas/botones).
- [x] Serialización JSON de bindings: `saveBindings`, `loadBindings`, `exportBindingsToJson`, `importBindingsFromJson`.
- [x] UI runtime: `OptionsMenu` muestra bindings y permite remapear acciones con feedback inmediato.
- [x] Registro del último evento crudo y APIs para consumirlo (`getLastKeyEvent`, `getLastMouseButtonEvent`, `clearLastEvents`) para facilitar remapeo.
- [x] Pruebas unitarias añadidas (Google Test): cobertura para detección de entradas, multi-bindings, serialización, edge-cases y remapeo.
- [x] Mejoras adicionales:
  - Input history (registro de eventos con timestamps y conteo de uso por acción).
  - Action caching (precomputar estados por frame para optimización de consultas frecuentes).

Arquitectura y contrato
- Patrón: Singleton (`InputManager::getInstance()`), pensado para inyección en escenas/servicios si se requiere.
- Inputs: `sf::Event` (reenviado por `Game::processEvents()`).
- Outputs: consultas booleanas por `Action` (pressed / just pressed / released), nombres legibles para UI, export/import JSON.
- Errores/edge cases: consultas devuelven `false` si no hay binding; importación de JSON maneja errores y deja bindings previos en caso de fallo.
- Concurrencia: accesos protegidos por `std::mutex`. Diseño pensado para uso principal en hilo de juego.

Detalles clave de implementación
- Per-frame edge detection: `endFrame()` realiza el snapshot de `currentKeys_` -> `previousKeys_` una vez por frame; `Game::run()` llama a `endFrame()` en el punto correcto para preservar la semántica de `isActionJustPressed()` durante el procesamiento de eventos y la actualización de escenas.
- Multi-bindings: cada `Action` mantiene vectores de teclas y botones; las consultas iteran y devuelven `true` si cualquiera cumple la condición.
- Serialización: JSON (nlohmann/json) con estructura `{ "keyBindings": { actionInt: [keyInt,...] }, "mouseBindings": { ... } }`.
- UI remap: `OptionsMenu` consume `getLastKeyEvent()` / `getLastMouseButtonEvent()` para rebind y guarda inmediatamente los bindings en `config/input_bindings.json`.
- Input history: habilitable, registra eventos con `gameTime_` y mantiene contador de uso por acción (útil para tutoriales y análisis).
- Action caching: opcional, precomputar `pressed/justPressed/released` para todas las acciones una vez por frame y servir consultas desde la caché para reducir coste de búsquedas repetidas.

Integración con el resto del proyecto
- `Game`:
  - `processEvents()` reenvía eventos a `InputManager::update(event)`.
  - `run()` ejecuta `InputManager::endFrame()` por fotograma (el build actual lo hace en el punto correcto para preservar la semántica de "just pressed").
- UI / Scenes:
  - `OptionsMenu` muestra bindings y permite remapear acciones en tiempo real; guarda los cambios inmediatamente.
  - Menús usan `isActionJustPressed()` para navegar; `PlayScene` usa `Action::Interact` para activaciones puntuales.
- CMake:
  - `CMakeLists.txt` incorpora la dependencia `nlohmann_json` y añade los tests (Google Test). `src/input/InputManager.cpp` está en el target principal.

Pruebas y verificación
- Build: el proyecto compila y genera `build/Release/AbyssalStation.exe` (verificado).
- Tests: hay un conjunto de tests con Google Test que cubren detección de entradas, multi-bindings, serialización, edge-cases y remapeo; los tests en `tests/` pasan en mi verificación local.
- Runtime: verificado que la navegación de menús responde con los bindings por defecto y que `isActionJustPressed()` funciona correctamente; remapeo desde `OptionsMenu` actualiza la UI y persiste los cambios.

Limitaciones y riesgos conocidos
- No hay soporte de gamepad en esta iteración; la arquitectura está preparada para añadirlo.
- `keyToString` ofrece nombres legibles básicos; podría mejorarse para representar combinaciones o mapeos más complejos.
- Aunque hay mutex para proteger estado, el subsistema asume principalmente uso en el hilo principal; si se hace acceso concurrente real desde otros hilos se recomienda ampliar las pruebas y validar la estrategia de sincronización.

Próximos pasos recomendados
- Añadir soporte básico de gamepad (mapear ejes y botones a `Action`).
- Mejorar `keyToString` para etiquetas localizables y combinaciones (Ctrl+X, Shift+W, etc.).
- Añadir tests de integración que cubran la interacción entre `OptionsMenu`, `InputManager` y `ConfigManager` (guardar/cargar bindings).
- Exponer el historial de input en la UI para analytics/tutoría si se requiere.

Estado: Integrado — el módulo `input` ahora soporta serialización, remapeo en tiempo de ejecución, pruebas unitarias, historial de inputs y optimización por cache; está listo para la siguiente iteración (gamepad, localización, más tests).

Fin del estado del módulo `input`.

