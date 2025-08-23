# Estado del módulo `input`

Resumen corto
- Ubicación: `src/input`.
- Objetivo: centralizar y mapear entradas (teclado, futuro gamepad) a acciones del juego y ofrecer consultas por acción para las escenas.
- Estado general: Implementado como Proof-of-Concept; compilable y integrado con `Game` y `PlayScene`.

Checklist rápido
- [x] `Action` enum definido (`MoveUp/MoveDown/MoveLeft/MoveRight/Confirm/Cancel/Pause`).
- [x] `InputManager` (singleton) con API pública: `getInstance()`, `bindKey`, `isActionPressed`, `isActionJustPressed`, `isActionReleased`, `update(const sf::Event&)`.
- [x] Bindings por defecto: WASD, Enter, Escape, P.
- [x] Logging de bindings usando `core::Logger` (mensajes tipo: "[Input] Acción MoveUp asignada a tecla W").
- [x] Integración: `Game::processEvents()` reenvía eventos a `InputManager`; `PlayScene` consulta `InputManager::getInstance()` para movimiento.

Archivos en `src/input`
- `Action.h` — enum class `input::Action` (scoped enum con tipo subyacente).
- `InputManager.h` — declaración de la clase (singleton, membresías privadas: `keyBindings_`, `currentKeys_`, `previousKeys_`, `mutex_`).
- `InputManager.cpp` — implementación: bindings por defecto, `bindKey` con logging, `update` basado en eventos SFML 3, y consultas que comparan `currentKeys_`/`previousKeys_`.

Diseño y contrato
- Patrón: Singleton (`InputManager::getInstance()`), pensado para compartirse entre escenas.
- Contrato mínimo:
  - Inputs: `sf::Event` (reenviado por `Game::processEvents()`).
  - Outputs: consultas booleanas por `Action` y notificaciones de transición (just pressed / released).
  - Error modes: si no hay binding para una acción, las consultas devuelven `false`.
  - Thread-safety: acceso protegido por `std::mutex` en la implementación actual.

Detalles importantes de implementación
- Bindings por defecto inicializados en el constructor (`W/S/A/D`, `Enter`, `Escape`, `P`).
- `update(const sf::Event&)` procesa `KeyPressed` y `KeyReleased` (API SFML 3: `event.is<...>()` + `getIf<...>()`) y modifica `currentKeys_`.
- Transiciones (`isActionJustPressed`, `isActionReleased`) se calculan comparando `previousKeys_` y `currentKeys_`.
- Al final de `update` se copia `previousKeys_ = currentKeys_` (semántica actual: `previousKeys_` refleja el estado al final de la llamada anterior).
- `bindKey` registra el binding con `core::Logger::instance().info(...)` y usa helpers para convertir `Action` y `Key` a cadenas legibles.

Integración con el resto del proyecto
- `Game`:
  - `processEvents()` ahora llama a `input::InputManager::getInstance().update(event)` para mantener el estado sincronizado con los eventos SFML.
- `PlayScene`:
  - Migrado a consultar acciones mediante `InputManager::getInstance().isActionPressed(Action::...)` en lugar de `sf::Keyboard::isKeyPressed`.
  - El rectángulo del jugador responde a WASD por defecto.
- CMake:
  - `CMakeLists.txt` fue actualizado para incluir `src/input/InputManager.cpp` en el target del ejecutable, evitando símbolos no resueltos.

Pruebas y verificación
- Build: el proyecto compila y genera `build/Release/AbyssalStation.exe` tras incluir los fuentes de `input` en CMake.
- Runtime: al ejecutar el binario localmente, `PlayScene` responde a WASD; los bindings se registran en el log al iniciarse (ejemplo: "[Input] Acción MoveUp asignada a tecla W").
- Tests: no hay tests unitarios automáticos para `InputManager` en esta iteración.

Limitaciones y riesgos conocidos
- No hay soporte de gamepad aún; la clase está preparada para extensión pero faltan mapeos y backends.
- `keyToString` cubre teclas comunes, pero la representación de teclas menos usadas cae a un entero — podría mejorarse.
- `previousKeys_ = currentKeys_` copia al final de `update`; si se desea detectar una transición en la misma iteración que llega el evento, la semántica podría invertirse o exponerse `beginFrame()/endFrame()`.
- Aunque hay un `mutex_`, el diseño actual asume que `update` y las consultas se llaman desde el hilo principal; concurrencia real requiere más pruebas.

Próximos pasos recomendados (opcionalmente implementables ahora)
- Añadir tests unitarios para `InputManager` (happy path + transiciones + rebinds).
- Añadir soporte básico de gamepad (mapear ejes/botones a `Action`).
- Mejorar la serialización de bindings (guardar/cargar configuración del usuario).
- Exponer una opción de runtime para activar logs de movimiento (por ahora sólo se registran cambios de binding).
- Documentar API de `input` en `docs/input.md` con ejemplos de uso desde escenas.

Resumen corto
- El módulo `input` ya está integrado y funcional como POC: bindings por defecto, consultas por acción y logging de bindings. Falta cobertura de tests y soporte de gamepads, y se pueden pulir detalles de representación y API para frame boundaries.

Fin del estado del módulo `input`.
