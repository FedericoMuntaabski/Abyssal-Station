# Estado actual del Core (consolidado y actualizado con cambios recientes)

Resumen corto
- Rama principal de trabajo: `feature/UI` (esta rama integró cambios de UI/menus e introdujo ajustes en input y en la integración con `Game`).
- Objetivo: explicar el estado actual del core tras integrar el sistema de escenas, el subsistema de input actualizado y la integración de un módulo de UI/menus (Main/Pause/Options) que afecta a `Game` y a las escenas.

Checklist de actualizaciones aplicadas
- [x] Documentar la delegación del bucle a `SceneManager` y la integración con UI/menus.
- [x] Describir cambios en `InputManager` (detección per-frame y soporte multi-bindings).
- [x] Anotar ajustes en `Game` (llamada a `InputManager::endFrame()` y delegación de UI/menus desde escenas).

Archivos revisados / actualizados
- `src/core/Game.h` / `src/core/Game.cpp` (llamada a `InputManager::endFrame()` y flujo de escenas)
- `src/core/AssetManager.*`, `src/core/Logger.*`, `src/core/Timer.*`
- `src/input/InputManager.h` / `src/input/InputManager.cpp` (endFrame, multi-bindings)
- `src/scene/` (Scene, SceneManager, MenuScene, PlayScene) — ahora integran `UIManager` y menús
- `src/ui/` (nuevo) — `Menu`, `MainMenu`, `PauseMenu`, `OptionsMenu`, `UIManager`

Visión general del core (resumen actualizado)

1) Game
- Clase: `Game` — ventana SFML y loop principal.
- Cambios clave:
  - `Game` crea `scene::SceneManager` y empuja `MenuScene` al inicio.
  - `Game::processEvents()` reenvía eventos a `input::InputManager::getInstance().update(event)` y a la escena actual.
  - Se añadió `input::InputManager::endFrame()`; en esta rama el juego toma un snapshot del estado previo de entrada justo ANTES de procesar los eventos del frame (es decir, `endFrame()` se invoca al inicio de la iteración de frame) para preservar la semántica de "just pressed" durante el procesamiento de eventos y las actualizaciones por frame.
  - El post-build copia `assets/` al directorio del ejecutable y `Game` realiza una búsqueda robusta de assets al iniciarse.

2) Subsystema de Input (actualizado)
 - `InputManager` ahora:
  - Soporta múltiples bindings por acción (vector de `sf::Keyboard::Key`) y equivalentes para botones de ratón (`mouseBindings_`).
  - `update(event)` actualiza el estado `currentKeys_` y `currentMouseButtons_` por evento; `endFrame()` copia el estado actual a previo una vez por frame.
  - `isActionJustPressed` / `isActionPressed` / `isActionReleased` comprueban todos los bindings asociados a una acción (teclas y botones de ratón).
  - Inicializa bindings por defecto (W/A/S/D para movimiento, flechas también añadidas; Enter = Confirm; Escape = Cancel; P = Pause; Left Mouse = Confirm) y registra los bindings con `core::Logger`.
  - Expone nuevas APIs para la UI y remapeo: `rebindKeys`, `rebindMouse`, `getLastKeyEvent`, `getLastMouseButtonEvent`, `clearLastEvents`, `getBindingName`, y getters para bindings actuales. `update()` registra el último evento crudo para permitir una UX de remap simple en `OptionsMenu`.

3) UI / Menús (nueva capa integrándose con escenas)
- Nuevos ficheros en `src/ui/` y su integración:
  - `Menu` (base abstracta): interfaz para `handleInput()`, `update()`, `render()` y hooks `onEnter()`/`onExit()`.
  - `MainMenu`: Start/Options/Exit (usa `SceneManager` y `UIManager`).
  - `PauseMenu`: Resume/Options/Quit — cuando se abre, marca pausa en `UIManager` y al cerrarse la desactiva.
  - `OptionsMenu`: Volume/Controls placeholders (ajuste simple de volumen, logs).
  - `UIManager`: pila de menús, delegación `update`/`render`, `pushMenu`/`popMenu`, consultas `isMenuActive(name)` y `isAnyMenuActive()`, y flag `setPaused()`/`isPaused()`.

4) Integración de UI con escenas
- `MenuScene` crea un `UIManager` y empuja `MainMenu` en `onEnter()`.
- `PlayScene` crea su propio `UIManager` y usa `pushMenu(new PauseMenu(...))` para alternar pausa cuando el usuario presiona Escape.
- `PlayScene::update()` ahora delega primero a `m_uiManager->update(dt)` y: si `PauseMenu` está activo, la función retorna temprano (congelando la lógica de juego); si cualquier menú está activo, evita propagar entradas de movimiento al juego (prioridad de UI).

5) AssetManager, Logger y Timer
- `AssetManager` sigue gestionando texturas y sonidos; carga de fuentes sigue siendo local en `MenuScene` (recomendado mover a `AssetManager`).
- `Logger` y `Timer` mantienen su papel en el core para trazas y mediciones.

- Se añadió `src/core/FontHelper.{h,cpp}`: helper para localizar y cargar una fuente desde `assets/fonts` o rutas del sistema. `MenuScene` y `PlayScene` usan este helper como fallback para la carga de fuentes/hints. `CMakeLists.txt` fue actualizado para incluir `FontHelper.cpp` en el build.

Quality gates y estado del build
- Build: el proyecto compila correctamente y el ejecutable se genera en `build/Release/AbyssalStation.exe`.
- Tests: no se añadieron tests automáticos en esta iteración.
- Smoke test: verificado que:
  - `MainMenu` aparece al iniciar la app.
  - Navegación por menús funciona (W/S y Enter).
  - `PlayScene` pausa la lógica (movimiento y AI) cuando `PauseMenu` está activo.

Cambios aplicados (delta)
- `src/core/Game.cpp`: llamada a `input::InputManager::endFrame()` y flujo de escena inicial.
- `src/input/*`: `endFrame()`, multi-bindings y correcciones en detección de eventos "just pressed".
- `src/scene/*`: integración con `UIManager` y creación de menús desde escenas.
- `src/ui/*`: nuevos ficheros y lógica de menú.
- `CMakeLists.txt`: añadidos los nuevos ficheros `src/ui/*` para compilación.

Recomendaciones y próximos pasos
- Mover la carga de fuentes a `AssetManager` y eliminar búsquedas ad-hoc en escenas (mejora de mantenimiento).
- Cambiar `UIManager::pushMenu(Menu*)` para aceptar `std::unique_ptr<Menu>` y actualizar los call-sites para evitar owning raw pointers.
- Integrar `OptionsMenu::applyVolume()` con el sistema de audio para que los cambios afecten realmente al SFX/music.
- Mejorar la UI: overlay translúcido en pausa, centrado y estilos de texto, soporte de mouse.

Resumen corto
- El core está estable tras integrar el sistema de escenas, los cambios en el subsistema de input y la nueva capa de UI/menus. El juego compila y la interacción básica (navegación por menús, pausa que congela la lógica de la escena) está verificada.

Fin del documento.
Input subsystem (nuevo)
- `src/input/Action.h`: enum class `input::Action` con acciones básicas (MoveUp, MoveDown, MoveLeft, MoveRight, Confirm, Cancel, Pause).
- `src/input/InputManager.{h,cpp}`: singleton que mapea `Action` a `sf::Keyboard::Key`, permite `bindKey`, mantiene `currentKeys` y `previousKeys`, y expone `isActionPressed`, `isActionJustPressed`, `isActionReleased`. Inicializa bindings por defecto (WASD + Enter + Escape + P) y usa `core::Logger` para loggear cambios de bindings.

Integración y notas:
- `Game` ahora reenvía eventos a `InputManager` para que escenas puedan consultar acciones a través de `InputManager::getInstance()` en lugar de leer teclas crudas.
- `PlayScene` fue adaptado para usar `InputManager` y ya no consulta `sf::Keyboard::isKeyPressed` directamente.
- Logging: `InputManager::bindKey` registra mensajes como "[Input] Acción MoveUp asignada a tecla W" usando `core::Logger`.

2) Scene system (nuevo, integrado desde la carpeta `src/scene`)
- Componentes añadidos:
  - `src/scene/Scene.h` (interfaz abstracta de escena con hooks `onEnter()`/`onExit()`)
  - `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp` (stack de escenas: push/pop/replace/current; delegación de eventos/update/render; logging de transiciones)
  - `src/scene/MenuScene.*` y `src/scene/PlayScene.*` (implementaciones de ejemplo)
- Integración con `core`:
  - `Game` crea y utiliza `SceneManager`.
  - Las escenas usan `core::Logger` para registrar acciones y estados.
- Estado: Funcionando. MenuScene carga una fuente desde `assets/fonts/` (implementada búsqueda robusta), PlayScene dibuja un rectángulo rojo y responde a teclas de flecha y Escape.

3) AssetManager
- Clase: `AssetManager` (singleton).
- Estado: funciona para texturas y sonidos. No se agregó carga centralizada de fuentes en `AssetManager` en esta iteración; actualmente `MenuScene` realiza una búsqueda simple/robusta de fuentes en `assets/fonts`.
- Limitaciones actuales: no recursividad por defecto, sin hot-reload ni protección por mutex para cargas concurrentes.

4) Logger
- Espacio de nombres: `core::Logger` (singleton).
- Estado: Implementado y usado por `SceneManager`, `MenuScene`, `PlayScene` y `Game` para trazas de información y advertencias.

5) Timer / TimerScope
- Clases: `Timer` y `TimerScope` mantienen su funcionalidad de utilidades para medición de tiempos; siguen incluidas y usadas en el proyecto.

6) main.cpp
- `main()` crea `Game` y llama `run()`; se mantienen las defensas en caso de excepciones no capturadas.

Contratos mínimos (inputs/outputs, errores)
- `Game::run()`: bloqueante, devuelve al salir del main loop; si no hay escenas activas el juego termina con una salida limpia.
- `SceneManager::push/replace/pop`: no lanzan excepciones por diseño; en caso de operación inválida se registra mediante `Logger`.
- `AssetManager::load*From(path)`: registra errores en caso de fallos de E/S y devuelve el conjunto cargado (si corresponde).

Casos borde y riesgos
- Recursos faltantes: si faltan assets la app lo registra y continúa (no se lanzan excepciones de fallo de carga en la mayoría de los casos). MenuScene tiene manejo de ausencia de fuentes (log + no dibuja texto si no hay fuente).
- Concurrencia: Logger es thread-safe; AssetManager no está protegido por mutex (riesgo si se accede desde múltiples hilos).
- Compatibilidad SFML: Código adaptado a SFML 3 (cambios en API de eventos, Text/Font, y Keyboard enum). Requiere SFML 3 en vcpkg/conan o en el sistema.
- Paths y ejecución: el build añade una etapa para copiar `assets/` a la carpeta del ejecutable; si se ejecuta el exe directamente desde otra ubicación es posible que no se encuentren assets a menos que se ajusten rutas o se use la búsqueda robusta implementada.

Cambios aplicados (delta) — resumen técnico
- `src/core/Game.h` / `src/core/Game.cpp`
  - Integración del `scene::SceneManager` y delegación de eventos/update/render.
  - Búsqueda robusta de `assets/` cuando el ejecutable corre desde `build/Release`.
  - Soporte de FPS en el título para debug.

- `src/core/Logger.cpp`, `src/core/Timer.cpp`
  - Asegurados en la lista de fuentes del build para resolver enlaces; Logger usado ampliamente en el core y las escenas.

- `src/scene/*` (nuevos archivos)
  - `Scene.h`, `SceneManager.*`, `MenuScene.*`, `PlayScene.*`.

- `CMakeLists.txt`
  - Se añadieron las nuevas fuentes (core + scene) al objetivo y se añadió un post-build custom command para copiar la carpeta `assets/` al directorio del ejecutable.
  - `find_package(SFML ...)` ajustado para incluir Audio si no estaba previamente (necesario para `sf::Sound`).

Estado del build y pruebas rápidas (quality gates)
- Build: compilación con la ayuda `build.ps1` y CMake (Visual Studio generator) generó `build/Release/AbyssalStation.exe` con éxito.
- Lint/Typecheck: no se añadieron nuevas herramientas de linting; la compilación C++ está limpia para los cambios aplicados.
- Unit tests: no se añadieron tests automatizados en esta iteración.
- Smoke test: ejecución rápida desde `build/Release` mostró:
  - `assets/` copiado a `build/Release/assets` por el post-build step.
  - `MenuScene` logró cargar `assets/fonts/Long_Shot.ttf` y mostró el texto del menú.
  - Transiciones: Enter -> PlayScene, Escape -> MenuScene; PlayScene mueve el rectángulo con flechas.

Commits y ramas
- Cambios recientes se encuentran en la rama de trabajo `feature/scene` (integración de escenas). Los cambios originalmente documentados en `feature/core` siguen en esa rama.

Notas y recomendaciones (próximos pasos)
- Centralizar carga de fuentes en `AssetManager` para evitar búsquedas de archivos en código de escenas.
- Añadir pruebas unitarias básicas: al menos un test para `SceneManager` (push/replace/pop y llamadas delegadas) y tests para `AssetManager::load*From`.
- Refactor: Considerar un objeto `ResourceLocator` que normalice rutas relativas/absolutas y busque assets en múltiples ubicaciones (repo, install dir, system fonts).
- CI: añadir una tarea de build en CI que ejecute la copia de `assets/` y una prueba de smoke headless (si es posible) para validar que no falten recursos.

Resumen corto de estado
- El core está funcional y ha recibido cambios importantes: integración de un sistema de escenas, ajustes en `Game` para delegar a `SceneManager`, inclusión de nuevas fuentes y copiado de assets en el build. El binario se construye y ejecuta localmente mostrando el flujo básico Menu->Play->Exit.

Si quieres, aplico alguno de los siguientes pasos automáticamente:
- [ ] Mover la carga de fuentes a `AssetManager` y actualizar llamadas en `MenuScene`.
- [ ] Añadir tests unitarios para `SceneManager` y `AssetManager`.
- [ ] Hacer commit y push del branch `feature/scene` a remoto (si no lo has subido aún).

Fin del informe.
