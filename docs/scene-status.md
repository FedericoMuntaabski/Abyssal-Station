# Estado actual del módulo `scene` (actualizado)

Resumen corto
- Objetivo: Documentar el estado actual de `src/scene` tras las últimas iteraciones: gestor de escenas, integración con el subsistema de UI/menus (`UIManager` y menús concretos), y las mejoras en `PlayScene` para soportar interacción con objetos (item pickup), hints en pantalla y visibilidad de IA.
- Estado general: Implementación funcional y probada localmente. `MenuScene`, `PlayScene`, `DebugPickupScene` y `SceneManager` operan correctamente. `PlayScene` ahora maneja la detección de items cercanos, muestra una indicación visual (ícono pulsante con fallback textual/shape) y ejecuta la interacción cuando el jugador pulsa la acción `Interact`.

Checklist de adaptación
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Documentar `MenuScene`, `PlayScene` y `DebugPickupScene` con integración a `UIManager` y `InputManager` actualizado.
- [x] Explicar la política de pause cuando un menú está activo y el logging de transiciones.

Archivos relevantes
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp`
- `src/scene/DebugPickupScene.h` / `src/scene/DebugPickupScene.cpp` (escena auxiliar para probar pickups)

Visión general
El módulo `scene` sigue proveyendo la abstracción de pantallas con una pila LIFO. En esta iteración se reforzó la integración con el sistema de UI y se añadieron comportamientos de interacción en `PlayScene`:
- `UIManager` (en `src/ui`) administra una pila de `Menu`s y delega `handleInput/update/render` al menú activo.
- `MenuScene` inicializa el `UIManager` y empuja el `MainMenu` al arrancar.
- `PlayScene` integra: control de entidades/colisiones/IA, gestión de menús en juego y lógica de interacción con items. Además existe `DebugPickupScene` para pruebas de recolección.

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
- Comportamiento: mantiene la pila de escenas (internamente `std::stack<std::unique_ptr<Scene>>`) y delega `handleEvent`, `update` y `render` a la escena actual. Todas las transiciones (push/pop/replace) registran trazas en `core::Logger`.

3) `MenuScene`
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento actualizado:
  - En `onEnter()` crea o reutiliza un `UIManager` y realiza `push(MainMenu)` para mostrar el menú principal.
  - Durante el loop, `MenuScene` delega eventos y render al `UIManager`.
  - `MenuScene` y `MainMenu` usan `core::FontHelper` para localizar y cargar fuentes (centralizado en `src/core/FontHelper.*`).

4) `PlayScene` (novedades y flujo)
- Ubicación: `src/scene/PlayScene.*`.
- Componentes inicializados en `onEnter()` (resumen):
  - `entities::EntityManager`
  - `collisions::CollisionManager`
  - `collisions::CollisionSystem`
  - `ai::EnemyManager`
  - `gameplay::ItemManager` / `PuzzleManager`
  - `UIManager` (para menús en juego)

- Flujo principal en `update(dt)` (resumen y diferencias relevantes):
  1. `PlayScene` delega eventos a `UIManager` cuando hay menús activos; la UI tiene prioridad sobre la lógica del juego.
  2. Si no hay menús activos, `PlayScene` procesa input (consulta a `InputManager`) y ejecuta la lógica: planificación de IA, validación de colisiones y commit de movimientos.
  3. Detección e interacción con items: `PlayScene` detecta items cercanos (filtrando por capas de colisión), muestra una indicación en pantalla (ícono pulsante con fallback a texto o círculo animado si no hay sprite/font) y al recibir `isActionJustPressed(Action::Interact)` llama a `gameplay::ItemManager::interactWithItem()` para ejecutar el efecto (mostrar toast, eliminar item o avanzar puzzle).
  4. Spawn y posicionamiento: los items de prueba se spawnean en relación al jugador y enemigos en `PlayScene` (ver `DebugPickupScene` para escenarios de prueba).

- Notas sobre hint visual:
  - Actualmente la indicación de interacción se renderiza desde `PlayScene` (ícono + animación). Hay un fallback que usa `core::FontHelper` para texto si la fuente está disponible, y un pulsing-circle si no. Recomendación: migrar esta indicación a `UIManager` para un overlay consistente.

- Manejo de tecla de interacción:
  - `InputManager` añadió `Action::Interact` enlazado por defecto a la tecla `E`. `PlayScene` usa `isActionJustPressed(Action::Interact)` para disparar la interacción puntual.

5) Integración con AI / EnemyManager (resumen)
- `PlayScene` orquesta enemigos con `EnemyManager`. Importante cambio técnico: la detección de jugador por parte de enemigos respeta obstáculos opacos (walls) usando un helper de colisiones: `collisions::CollisionManager::segmentIntersectsAny(p0,p1,exclude,allowedLayers)`.
- Se introdujeron capas de colisión en `entities::Entity` / `collisions::CollisionBox` y máscaras `kLayerMask*` para filtrar qué colliders deben considerarse (por ejemplo: los enemigos ignoran la capa `Item` en su navegación y no quedan bloqueados por objetos recogibles).

6) Logging y trazabilidad
- Todas las transiciones entre escenas y cambios de estado relevantes (push/pop/replace) registran mensajes a través de `core::Logger`. Transiciones de escena y eventos de interacción (pickup/puzzle complete) también generan logs para facilitar debugging.

Notas de integración y comportamiento observable
- Prioridad de input: los menús tienen prioridad sobre la lógica de juego. Cuando un menú está activo, `UIManager` consume navegación y selección.
- Integración de fuente: `core::FontHelper` centraliza la detección y carga de fuentes (usado por `MenuScene` y `PlayScene` para fallbacks legibles).
- Colisiones y visibilidad AI: la nueva función de test de segmento contra colliders permite que la línea de visión de enemigos sea bloqueada por muros.
- Las capas de colisión evitan que entidades (p. ej. enemigos) consideren items coleccionables como obstáculos.

Integración con `Game` y assets
- `Game` crea `SceneManager` y empuja `MenuScene` al arrancar.
- `CMakeLists.txt` se actualizó para incluir los ficheros nuevos si no estaban presentes (p. ej. `core/FontHelper.*`).

Quality gates y pruebas rápidas
- Compilación: OK (Release build generada en la rama de trabajo `feature/interactions`).
- Runtime verificado: el `MainMenu` aparece al iniciar; dentro de `PlayScene`, pulsar `Escape` abre el `PauseMenu` y congela la lógica de la escena; la indicación de interacción aparece cuando el jugador está cerca de un item y la tecla `E` (por defecto) recoge el item y muestra el toast en pantalla.

Limitaciones y riesgos
- Diseño de pausa:
  - Actualmente la pausa congela la mayor parte de la lógica de la escena (actualizaciones, IA y movimiento). Si se desea que ciertos subsistemas sigan ejecutándose (p. ej. animaciones UI, música o redes), habrá que introducir banderas más finas o excepciones por subsistema.
- Propagación de input:
  - Asegurar que la UI capture todos los inputs relevantes (incluyendo gamepad/ratón cuando se añadan) para evitar que el juego reciba comandos no deseados mientras un menú está activo.

Recomendaciones y próximos pasos
- Mover la indicación de interacción de `PlayScene` a `UIManager` para mantener overlays consistentes y permitir que la UI rinda la indicación desde una capa única.
- Añadir tests unitarios para: `CollisionManager::segmentIntersectsAny`, filtrado por capas, y el flujo `ItemManager::interactWithItem` (pickup + toast + puzzle progression).
- Exponer la reasignación de bindings en `OptionsMenu` con mejor UI para mostrar múltiples bindings y cancelar.

Resumen rápido
- El módulo `scene` integra el sistema de UI/menus, con `MenuScene` mostrando el `MainMenu` al inicio, `PlayScene` capaz de abrir `PauseMenu`, detectar y permitir interacción con objetos cercanos (ícono de interacción + E por defecto) y `SceneManager` delegando ciclo de vida y registrando transiciones en `core::Logger`.

Fin del `scene-status`.
