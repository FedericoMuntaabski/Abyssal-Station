# Estado actual del módulo `scene` (actualizado)

Resumen corto
- Objetivo: Documentar el estado actual de `src/scene` tras las últimas iteraciones: gestor de escenas, integración con el nuevo subsistema de UI/menus (`UIManager` y menús concretos), y las mejoras en `PlayScene` para coordinar colisiones y pausar lógica cuando hay menús activos.
- Estado general: Implementación funcional y probada localmente. `MenuScene`, `PlayScene` y `SceneManager` operan correctamente; se añadió `UIManager` y menús (`MainMenu`, `PauseMenu`, `OptionsMenu`) y se ajustó el flujo de control para que los menús prioricen input y puedan congelar la lógica de juego.

Checklist de adaptación
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Documentar `MenuScene` y `PlayScene` con su integración a `UIManager` y al `InputManager` actualizado.
- [x] Explicar la política de pause cuando un menú está activo y el logging de transiciones.

Archivos relevantes
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp`

Visión general
El módulo `scene` continúa proveyendo la abstracción de pantallas con una pila LIFO. En esta iteración se añadieron integraciones con el nuevo sistema de UI:
- `UIManager` (nuevo, en `src/ui`) administra una pila de `Menu`s y delega `handleInput/update/render` al menú activo.
- `MenuScene` inicializa el `UIManager` y empuja el `MainMenu` al arrancar.
- `PlayScene` crea su propio `UIManager` (o reutiliza uno compartido según diseño) para permitir menús en juego (p.ej. `PauseMenu`) y delega a éste la navegación y render de menús.

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
- Comportamiento: mantiene la pila de escenas (internamente `std::stack<std::unique_ptr<Scene>>`), y delega `handleEvent`, `update` y `render` a la escena actual. Además, todas las transiciones (push/pop/replace) registran trazas en `core::Logger` para facilitar debugging.

3) `MenuScene`
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento actualizado:
  - En `onEnter()` crea un `UIManager` y realiza `push(MainMenu)` para mostrar el menú principal al iniciar la aplicación.
  - Durante el loop, `MenuScene` delega eventos y render al `UIManager` (el `MainMenu` maneja la navegación: Start → `PlayScene`, Options → `OptionsMenu`, Exit → cerrar la escena/app).

4) `PlayScene` (novedades y flujo)
- Ubicación: `src/scene/PlayScene.*`.
- Componentes inicializados en `onEnter()` (resumen):
  - `entities::EntityManager`
  - `collisions::CollisionManager`
  - `collisions::CollisionSystem`
  - `ai::EnemyManager` (si procede)
  - `UIManager` (para menús en juego)

- Flujo principal en `update(dt)` (resumen y diferencias relevantes):
  1. Procesamiento de input: `PlayScene` consulta `InputManager` para controles de juego, pero delega navegación/menu a `UIManager` cuando hay menús activos.
  2. Antes de aplicar lógica de juego, `PlayScene` llama `m_uiManager->update(dt)` y `m_uiManager->handleInput(...)` (la UI consume eventos relevantes).
  3. Si `m_uiManager->isAnyMenuActive()` o `m_uiManager->isMenuActive("PauseMenu")` es true, `PlayScene::update` retorna temprano (efecto: la escena queda 'congelada' mientras el menú de pausa está activo). Esto evita que la física, IA y movimientos se procesen mientras el juego está en pausa.
  4. Si no hay menús activos, se ejecuta la lógica habitual: `EntityManager::updateAll(dt)`, pre-move validation para `Player`, consultas a `CollisionManager`, `CollisionSystem::resolve(...)` y commits de movimiento.

- Manejo de tecla Escape / pausa
  - `PlayScene` detecta la acción de pausa (`Action::Cancel` o `Escape`) y delega a `UIManager` para `push(PauseMenu)` o `pop()` si el `PauseMenu` ya estaba presente. `PauseMenu::onEnter()` / `onExit()` registran logs y actualizan el estado de pausa del `UIManager`.

5) Integración con AI / EnemyManager (resumen)
- `PlayScene` sigue creando y orquestando enemigos a través de `EnemyManager`. Los pasos de planificación y commit de movimientos son compatibles con la validación por `CollisionManager`.

6) Logging y trazabilidad
- Todas las transiciones entre escenas y cambios de estado relevantes (pushing/popping de escenas y menús) registran mensajes a través de `core::Logger`. Esto facilita reproducir el comportamiento en tiempo de ejecución y depurar problemas de sincronización entre la UI y la escena.

Notas de integración y comportamiento observable
- Prioridad de input: los menús tienen prioridad sobre la lógica de juego. Cuando un menú está activo, la navegación y selección se consumen por `UIManager` y no se propagan a la lógica de `PlayScene`.
- La semántica de per-frame para entradas se garantizó con `InputManager::endFrame()`, que `Game::run()` llama una vez por frame; esto era necesario para que la navegación de menús (que usa `isActionJustPressed`) funcione correctamente.
 - La semántica de per-frame para entradas se garantizó con `InputManager::endFrame()`, que `Game::run()` llama una vez por frame; esto era necesario para que la navegación de menús (que usa `isActionJustPressed`) funcione correctamente.
 - `InputManager` fue extendido para soportar mouse buttons y arrow keys por defecto, y expone APIs para remapear bindings y consultar nombres legibles para UI (`rebindKeys`, `rebindMouse`, `getLastKeyEvent`, `getLastMouseButtonEvent`, `getBindingName`, etc.). Esto permite que `OptionsMenu` presente y remapee controles.

Integración con `Game` y assets
- `Game` crea `SceneManager` y empuja `MenuScene` al arrancar (que a su vez muestra el `MainMenu`).
- `CMakeLists.txt` se actualizó para incluir los ficheros nuevos si no estaban presentes y `Game::run()` ahora respeta la nueva contract (`InputManager::endFrame()` al final del frame).

Quality gates y pruebas rápidas
- Compilación: OK (Release build generada en la rama de trabajo `feature/UI`).
- Runtime verificado: el `MainMenu` aparece al iniciar; dentro de `PlayScene`, pulsar `Escape` abre el `PauseMenu` y congela la lógica de la escena; navegar en menús responde correctamente a las acciones configuradas.

Limitaciones y riesgos
- Diseño de pausa:
  - Actualmente la pausa congela la mayor parte de la lógica de la escena (actualizaciones, IA y movimiento). Si se desea que ciertos subsistemas sigan ejecutándose (p. ej. animaciones UI, música o redes), habrá que introducir banderas más finas o excepciones por subsistema.
- Propagación de input:
  - Asegurar que la UI capture todos los inputs relevantes (incluyendo gamepad/ratón cuando se añadan) para evitar que el juego reciba comandos no deseados mientras un menú está activo.

Recomendaciones y próximos pasos
- Mantener la separación de responsabilidades: `UIManager` gestiona menús y su ciclo de vida; `Scene` gestiona la lógica del mundo.
- Reemplazar las llamadas que usan punteros crudos para `pushMenu(Menu*)` por overloads que acepten `std::unique_ptr<Menu>` para evitar ambigüedades de propiedad.
- Exponer en `OptionsMenu` la visualización y reasignación de bindings (usar la API de `InputManager` para listar/bindear teclas).
 - Reemplazar las llamadas que usan punteros crudos para `pushMenu(Menu*)` por overloads que acepten `std::unique_ptr<Menu>` para evitar ambigüedades de propiedad.
 - Mejorar el UI de remapeo en `OptionsMenu`: actualmente existe un flujo básico; se recomienda completar la interfaz para seleccionar por acción, mostrar múltiples bindings y permitir cancelar/timeout.
 - Nota: la llamada a `getenv` en `MenuScene` fue reemplazada por `_dupenv_s` para evitar advertencias del compilador MSVC.

Resumen rápido
- El módulo `scene` ahora integra de forma limpia el sistema de UI/menus, con `MenuScene` mostrando el `MainMenu` al inicio y `PlayScene` capaz de abrir `PauseMenu` y congelar la lógica de juego mientras los menús están activos. `SceneManager` delega ciclo de vida y registra transiciones en `core::Logger`.

Fin del `scene-status`.
