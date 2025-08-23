# Estado actual del Core (consolidado)

Resumen corto
- Rama actual de trabajo: `feature/Player` / `feature/scene` (cambios recientes integrados en `src/core`).
- Objetivo: Documento que recoge el estado actual del núcleo del motor tras las últimas modificaciones: integración del sistema de escenas, nuevo subsistema de input, y ajustes en carga de assets y en el bucle principal (`Game`).

Checklist de requisitos de esta adaptación
- [x] Actualizar la descripción del `Game` para reflejar la delegación a `SceneManager`.
- [x] Documentar el nuevo `input::InputManager` y su integración con escenas.
- [x] Reflejar el estado de `AssetManager`, `Logger` y `Timer`.

Archivos revisados
- `src/core/Game.h` / `src/core/Game.cpp`
- `src/core/AssetManager.h` / `src/core/AssetManager.cpp`
- `src/core/Logger.h` / `src/core/Logger.cpp`
- `src/core/Timer.h` / `src/core/Timer.cpp`
- `src/input/InputManager.h` / `src/input/InputManager.cpp`
- `src/scene/` (Scene, SceneManager, MenuScene, PlayScene)

Visión general del core

Componentes principales y estado actualizado

1) Game
- Clase: `Game` — encapsula la ventana SFML y el bucle principal.
- API pública: `Game(width, height, title)`, `run()`, `stop()` (bloqueante el `run`).
- Cambios clave:
  - `Game` crea y mantiene una instancia de `scene::SceneManager` y delega eventos, update y render a la escena activa.
  - `processEvents()` reenvía eventos a `input::InputManager` y a la escena actual (`SceneManager::handleEvent`).
  - Se añadió búsqueda robusta de la carpeta `assets/` ascendiendo el filesystem cuando el ejecutable corre desde `build/Release`.
  - Soporte de FPS en el título para depuración.
- Estado: Implementado y probado localmente; el ejecutable se construye en `build/Release/AbyssalStation.exe` y las escenas básicas (Menu/Play) funcionan y responden a entradas.

2) Subsystema de Input
- Archivos: `src/input/Action.h`, `src/input/InputManager.h/.cpp`.
- Descripción: `InputManager` es un singleton que mapea `input::Action` a teclas (`sf::Keyboard::Key`), mantiene estado actual y previo, y expone consultas como `isActionPressed()` y `isActionJustPressed()`.
- Integración: `Game` y las escenas usan `InputManager::getInstance()` en lugar de comprobar teclas crudas; `InputManager` inicializa bindings por defecto (WASD, Enter, Escape, P) y permite rebind.

3) Sistema de escenas
- Archivos: `src/scene/Scene.h`, `SceneManager.*`, `MenuScene.*`, `PlayScene.*`.
- Funcionalidad: stack de escenas (push/pop/replace), delegación de eventos/update/render, hooks `onEnter()`/`onExit()` y logging de transiciones.
- Integración con `Game`: `Game` crea el `SceneManager` y empuja `MenuScene` al inicio.

4) AssetManager
- Archivos: `src/core/AssetManager.h/.cpp`.
- Estado: Singleton que carga texturas y buffers de sonido desde carpetas específicas; `Game` lo usa para una carga rápida de `textures` y `sounds` (p. ej. `background`, `sound_test`).
- Limitaciones: aún no centraliza la carga de fuentes (actualmente `MenuScene` busca fuentes directamente), no hay protección por mutex para cargas concurrentes ni hot-reload.

5) Logger
- Archivos: `src/core/Logger.h/.cpp`.
- Estado: Logger singleton con salida a consola y archivo (thread-safe mediante mutex). Usado por `Game`, `SceneManager` y escenas para trazas y advertencias.

6) Timer / TimerScope
- Archivos: `src/core/Timer.h/.cpp`.
- Estado: utilidades para medir delta times y duraciones; `Timer` y `TimerScope` siguen presentes y estables.

7) main.cpp
- `main()` crea `Game` y llama `run()`; incluye manejo básico de excepciones y salidas limpias.

Contratos mínimos
- `Game::run()` es bloqueante y finaliza limpiamente si no hay escenas activas.
- `SceneManager` no lanza en operaciones normales; errores se registran con `Logger`.
- `AssetManager::load*From(path)` registra errores de I/O y devuelve los assets cargados cuando es posible.

Casos borde y riesgos
- Recursos faltantes: la app registra y continúa (por ejemplo, si faltan fuentes o imágenes el comportamiento degrade graceful con logs).
- Concurrencia: `Logger` es thread-safe; `AssetManager` no está protegido (riesgo si se accede desde múltiples hilos).
- Dependencias: el proyecto está adaptado a SFML 3; requiere que SFML (con Audio) esté disponible en vcpkg o sistema.
- Paths y ejecución: Existe un paso post-build que copia `assets/` a la carpeta del ejecutable; si se ejecuta desde otra ruta la búsqueda robusta intenta localizar `assets/` ascendiendo el árbol.

Cambios aplicados (delta)
- `src/core/Game.h/cpp`: integración de `scene::SceneManager`, delegación de eventos/update/render, búsqueda de assets y soporte de FPS.
- `src/input/*`: nuevo subsistema `InputManager` y `Action`.
- `src/scene/*`: nuevo sistema de escenas con `MenuScene` y `PlayScene` de ejemplo.
- `src/core/AssetManager.*`: carga central de texturas y sonidos (fuentes aún no centralizadas).
- `src/core/Logger.*`, `src/core/Timer.*`: utilidades consolidadas y usadas por el core y las escenas.

Quality gates y estado del build
- Build: el proyecto compila con CMake / Visual Studio y el post-build copia `assets/` a `build/Release`.
- Tests: no se añadieron tests automatizados en esta iteración.
- Smoke test: ejecución local desde `build/Release` evidencia:
  - `MenuScene` carga `assets/fonts/Long_Shot.ttf` si está disponible (búsqueda robusta), muestra texto y permite transicionar a `PlayScene` con Enter.
  - `PlayScene` responde a movimiento y Escape vuelve al menú.

Recomendaciones y próximos pasos (sugeridos)
- Centralizar la carga de fuentes en `AssetManager` y eliminar búsquedas ad-hoc en escenas.
- Añadir tests unitarios básicos para `SceneManager` y `AssetManager`.
- Añadir un `ResourceLocator` para normalizar rutas de assets (repo, build dir, install dir).
- Considerar protección con mutex en `AssetManager` si se prevé carga desde múltiples hilos.

Resumen corto
- El core está operativo y ha recibido cambios importantes: sistema de escenas, subsistema de input, centralización parcial de assets y mejoras en `Game`. El binario se construye y ejecuta mostrando el flujo básico Menu->Play->Exit.

Estado de requisitos
- Actualizar doc: Done.
- Refactor sugerido (fuentes en AssetManager): Recomendado (pendiente).

Fin del documento.
# Estado actual del Core (estado consolidado con cambios recientes)

Resumen corto
- Ramas relevantes:
  - `feature/core` (documenta cambios específicos del core)
  - `feature/scene` (rama activa donde se integraron ajustes de `src/core` para soportar escenas)
- Objetivo: Documento guía que describe el estado actual del core y recoge los cambios recientes aplicados en `src/core` (por ejemplo: integración de SceneManager, actualizaciones en CMake y manejo de assets en tiempo de ejecución).

Checklist de lo que actualicé en este documento
- [x] Incluir los cambios recientes en `src/core` (Game y ajustes relacionados).
- [x] Anotar las nuevas dependencias y ajustes en `CMakeLists.txt`.
- [x] Documentar el comportamiento de build y runtime (copiado de `assets/` y ruta del ejecutable).

Archivos leídos / relevantes
- `src/core/Game.h` / `src/core/Game.cpp` (modificados para integrar SceneManager y pruebas de assets)
- `src/core/AssetManager.h` / `src/core/AssetManager.cpp`
- `src/core/Logger.h` / `src/core/Logger.cpp`
- `src/input/Action.h`, `src/input/InputManager.h`, `src/input/InputManager.cpp` (nuevo módulo de input integrado con core)
- `src/core/Timer.h` / `src/core/Timer.cpp`
- `src/main.cpp`

Visión general del core

Componentes principales y estado actualizado

1) Game
- Clase: `Game` (encapsula ventana SFML y bucle principal).
- API pública: `Game(width, height, title)`, `run()`, `stop()`.
- Nuevas responsabilidades añadidas: gestionar una instancia de `scene::SceneManager` y delegar eventos, update y render a la escena activa; mantener lógica de prueba para assets (background y audio demo) y mostrar FPS en el título.
- Implementación relevante:
  - Se añadió `std::unique_ptr<scene::SceneManager> m_sceneManager;` en la clase `Game`.
  - `Game::processEvents()` ahora reenvía eventos a `input::InputManager::getInstance().update(event)` además de a la escena actual. Esto permite centralizar y mapear entradas (WASD, Enter, Escape, P) a acciones del juego.
  - En `run()` se inicializa el `SceneManager`, se hace push de la `MenuScene` al inicio y se delegan eventos/updates/render a `m_sceneManager`.
  - `processEvents()` ahora reenvía eventos a la escena actual mediante `m_sceneManager->handleEvent(ev)`.
  - Se mantiene la búsqueda hacia arriba en el filesystem para localizar `assets/` cuando el exe se ejecuta desde `build/Release`.
- Estado: Implementado y probado localmente: el ejecutable se genera en `build/Release/AbyssalStation.exe`, las escenas (Menu/Play) funcionan, y las transiciones (Enter/Escape) y movimiento en PlayScene están operativas.

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
