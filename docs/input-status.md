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
- `Action.h` — enum class `input::Action`.
- `InputManager.h` — declaración del singleton (`keyBindings_` ahora mapea `Action` a `std::vector<sf::Keyboard::Key>`, `currentKeys_`, `previousKeys_`, `mutex_`).
 - `InputManager.h` — declaración del singleton (`keyBindings_` ahora mapea `Action` a `std::vector<sf::Keyboard::Key>`, `currentKeys_`, `previousKeys_`, `mutex_`). Contiene además helpers para obtener el nombre legible de un binding y APIs de remapeo.
- `InputManager.cpp` — implementación: bindings por defecto, `bindKey` aporta logging y append de teclas, `update` actualiza `currentKeys_` por evento y `endFrame()` copia `currentKeys_` a `previousKeys_` una vez por frame; las consultas iteran sobre todos los bindings.
 - `InputManager.cpp` — implementación: bindings por defecto (WASD + arrow keys; Enter = Confirm; Escape = Cancel; P = Pause; Left Mouse = Confirm; E = Interact), `bindKey` y `bindMouse` aportan logging y append de bindings, `update` actualiza `currentKeys_` y `currentMouseButtons_` por evento y registra el último evento crudo para remapping, `endFrame()` copia el estado actual a previo una vez por frame; las consultas iteran sobre todos los bindings (teclas y botones). `getBindingName` y `rebind*` son utilidades públicas empleadas por `OptionsMenu` y por escenas para mostrar controles.
 - `InputManager.h` — declaración del singleton (`keyBindings_` ahora mapea `Action` a `std::vector<sf::Keyboard::Key>`, soporte equivalente para `mouseBindings_`, `currentKeys_`, `currentMouseButtons_`, `previousKeys_`, `previousMouseButtons_`, y `mutex_`).
 - `InputManager.cpp` — implementación: bindings por defecto (WASD + arrow keys; Enter = Confirm; Escape = Cancel; P = Pause; Left Mouse = Confirm), `bindKey` y `bindMouse` aportan logging y append de bindings, `update` actualiza `currentKeys_` y `currentMouseButtons_` por evento y registra el último evento crudo para remapping, `endFrame()` copia el estado actual a previo una vez por frame; las consultas iteran sobre todos los bindings (teclas y botones).

Diseño y contrato
- Patrón: Singleton (`InputManager::getInstance()`), pensado para compartirse entre escenas y subsistemas.
- Contrato mínimo:
  - Inputs: `sf::Event` (reenviado por `Game::processEvents()`).
  - Outputs: consultas booleanas por `Action` y notificaciones de transición (just pressed / released).
  - Error modes: si no hay binding para una acción, las consultas devuelven `false`.
  - Thread-safety: accesos protegidos por `std::mutex` en consultas y actualizaciones; diseño pensado para uso en hilo principal.

Adición importante:
- `Action::Interact` está pensado para ser consultado con `isActionJustPressed()` desde la lógica de juego (p.ej. `PlayScene`) para disparar interacciones puntuales. La UI puede preguntar `getBindingName(Action::Interact)` para pintar una etiqueta o un ícono con la tecla actualmente asignada.

Detalles importantes de implementación
- Multi-bindings: `bindKey(Action, Key)` añade la tecla al vector de bindings de la acción. Esto permite atajos múltiples (p.ej. W y Up para MoveUp).
- Per-frame edge detection: `update(event)` ya no copia `previousKeys_` inmediatamente; en su lugar `endFrame()` realiza la copia una vez por frame. `Game::run()` llama a `endFrame()` después de `processEvents()`. Esto arregla la semántica de `isActionJustPressed()` (antes había un bug que lo hacía inefectivo cuando `update` era llamado por cada evento).
 - Per-frame edge detection: `update(event)` no copia `previousKeys_` inmediatamente; `endFrame()` realiza la copia una vez por frame. Nota: la implementación en este repo toma un snapshot del estado previo llamando a `endFrame()` justo antes de procesar los eventos del frame en `Game::run()` (esto evita que el primer evento del frame sobrescriba la semántica de "just pressed" durante el procesamiento de eventos y actualizaciones por frame).
- Las consultas `isActionPressed/JustPressed/Released` prueban todos los bindings asociados a la acción y devuelven `true` si cualquiera cumple la condición.
- `keyToString` y logging siguen presentes para trazabilidad (ej. binding por defecto registrado en arranque).

Integración con el resto del proyecto
- `Game`:
  - `processEvents()` llama a `input::InputManager::getInstance().update(event)` por cada evento.
  - `run()` llama a `input::InputManager::getInstance().endFrame()` una vez por fotograma, después de procesar eventos.
- `Scene`s y UI:
  - `PlayScene` y los menús (`MainMenu`, `PauseMenu`, `OptionsMenu`) usan `InputManager::getInstance()` para navegar/select y para controles de juego.
  - Menús dependen de la semántica `isActionJustPressed()` para navegación por opción (up/down/confirm). El ajuste `endFrame()` fue necesario para que la navegación funcione correctamente.
  - `PlayScene` usa `isActionJustPressed(Action::Interact)` para disparar la interacción del jugador con objetos cercanos; al activarse, `ItemManager::interactWithItem()` centraliza la lógica (UI toast, eliminación del objeto o avance de puzzle).
  - La UI (toasts y la indicación en pantalla para recoger objetos) consulta `getBindingName(Action::Interact)` para mostrar la tecla/ícono correcto y permite remapeo desde `OptionsMenu`.
  - Menús dependen de la semántica `isActionJustPressed()` para navegación por opción (up/down/confirm). El ajuste `endFrame()` fue necesario para que la navegación funcione correctamente.
 - `Scene`s y UI:
  - `PlayScene` y los menús (`MainMenu`, `PauseMenu`, `OptionsMenu`) usan `InputManager::getInstance()` para navegar/select y para controles de juego.
  - Menús dependen de la semántica `isActionJustPressed()` para navegación por opción (up/down/confirm). El ajuste de snapshot en `endFrame()` fue necesario para que la navegación funcione correctamente.
  - `OptionsMenu` ahora puede mostrar y remapear bindings usando las nuevas APIs (`getBindingName`, `rebindKeys`, `rebindMouse`, etc.).
- CMake:
  - `CMakeLists.txt` incluye `src/input/InputManager.cpp` en el target del ejecutable.

Pruebas y verificación
- Build: el proyecto compila y genera `build/Release/AbyssalStation.exe`.
- Runtime: verificado que la navegación de menús responde con W/S (o los bindings alternativos) y que `isActionJustPressed()` funciona de forma fiable tras introducir `endFrame()`.
 - Runtime: verificado que la navegación de menús responde con W/S (o los bindings alternativos), que `isActionJustPressed()` funciona de forma fiable tras introducir `endFrame()`, y que `Action::Interact` responde a la tecla `E` por defecto para recoger/activar objetos en `PlayScene`.
- Tests: no tests unitarios añadidos en esta iteración; recomendado añadir tests para transiciones y rebinds.
 - Tests: no tests unitarios añadidos en esta iteración; recomendado añadir tests para transitions, multi-bindings, and remap APIs.

Limitaciones y riesgos conocidos
- No hay soporte de gamepad aún; la clase está preparada para extensión.
- `keyToString` cubre teclas comunes; podría ampliarse para mejores etiquetas.
- Mutex protege el estado pero el diseño asume llamadas desde el hilo principal; si se utiliza desde múltiples hilos hay que validar concurrencia y añadir pruebas.

Próximos pasos recomendados
- Añadir tests unitarios para `InputManager` (transición, multi-bindings, rebind).
- Añadir API para serializar bindings (guardar/cargar configuración del jugador).
- Agregar soporte básico de gamepad (mapear ejes/botones a `Action`).
- Proveer una vista en tiempo de ejecución que muestre bindings actuales (útil para `OptionsMenu`).
 - Añadir tests unitarios para `InputManager` (transición, multi-bindings, rebind).
 - Añadir API para serializar bindings (guardar/cargar configuración del jugador).
 - Agregar soporte básico de gamepad (mapear ejes/botones a `Action`).
 - Proveer una vista en tiempo de ejecución que muestre bindings actuales y permita remap por acción (mejorar `OptionsMenu`).

Resumen corto
- El módulo `input` está integrado y mejorado: soporta multi-bindings, detección per-frame correcta mediante `endFrame()`, y está listo para que la UI y las escenas realicen navegación y control de juego de forma robusta.

Fin del estado del módulo `input`.
