
# Estado actual del módulo `scene` (actualizado)

Resumen corto
- Objetivo: Documentar el estado actual de `src/scene` tras las últimas iteraciones: gestor de escenas, escenas de ejemplo, integración con `Game`, uso de `InputManager` y entidades.
- Estado general: Implementación funcional y probada localmente. `MenuScene` y `PlayScene` están operativos, las transiciones funcionan y las escenas usan `core::Logger` para trazas.

Checklist de adaptación
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Detallar comportamiento de `MenuScene` y `PlayScene`, incluyendo uso de `InputManager` y `EntityManager`.
- [x] Documentar la integración con `Game`, búsqueda de assets y ajustes de build.

Archivos relevantes
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp`

Visión general
El módulo `scene` proporciona una abstracción de pantallas (scenes) con una pila LIFO para gestionar estados (menú, juego, pausa, etc.). El `SceneManager` delega eventos, actualización y render a la escena activa y maneja hooks de ciclo de vida (`onEnter` / `onExit`).

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
- Implementación: mantiene `std::stack<std::unique_ptr<Scene>> m_scenes`.
- API principal:
  - push(std::unique_ptr<Scene>) — llama `onExit` en la anterior, `onEnter` en la nueva.
  - pop() — llama `onExit` en la actual y `onEnter` en la anterior (si existe).
  - replace(std::unique_ptr<Scene>) — pop + push atómico.
  - current() — devuelve puntero a la escena activa o nullptr.
  - handleEvent/update/render — delegan a la escena actual.
- Logging: registra transiciones y situaciones anómalas con `core::Logger`.

3) `MenuScene`
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento clave:
  - En `onEnter()` realiza una búsqueda robusta de fuentes en `assets/fonts` (y rutas alternativas como fuentes del sistema en Windows) y carga la primera disponible.
  - Si la fuente se carga, crea un `sf::Text` que dice "Presione Enter para jugar" y lo renderiza.
  - En `handleEvent()`: Enter → `replace(PlayScene)`, Escape → `pop()`.
  - Registra información y advertencias mediante `core::Logger`.
- Notas: la carga de fuentes sigue siendo ad-hoc en la escena (pendiente de centralizar en `AssetManager`).

4) `PlayScene`
- Ubicación: `src/scene/PlayScene.*`.
- Comportamiento clave:
  - Crea un `sf::RectangleShape` rojo como marcador del jugador y lo posiciona en `onEnter()`.
  - Usa `input::InputManager` para consultar acciones mapeadas (MoveLeft/Right/Up/Down) y mueve el rectángulo en `update(dt)` normalizando la velocidad por `dt`.
  - Inicializa un `entities::EntityManager` y añade una `entities::Player` que sincroniza posición con el rectángulo.
  - `handleEvent()` detecta Escape y reemplaza la escena por `MenuScene`.

Integración con `Game`
- `Game` crea `SceneManager` en `run()`, hace `push` de `MenuScene` al iniciar y delega `handleEvent/update/render` al `SceneManager`.
- Si la pila queda vacía, `Game` finaliza limpiamente.

Build, CMake y assets
- `CMakeLists.txt` fue actualizado para incluir los archivos de `scene` y asegurar que `Logger.cpp` y `Timer.cpp` estén enlazados.
- Existe un paso post-build que copia `assets/` al directorio del ejecutable (`build/Release/assets`) para que la búsqueda de fuentes y texturas funcione en ejecuciones locales.

Quality gates y pruebas rápidas
- Compilación: OK después de incluir los nuevos ficheros en CMake.
- Smoke test: ejecución desde `build/Release`:
  - `MenuScene` carga una fuente desde `assets/fonts` y muestra el texto del menú.
  - Presionar Enter → `PlayScene`; Escape → vuelve al menú.
  - En `PlayScene` el rectángulo se mueve con las acciones mapeadas en `InputManager`.

Limitaciones y riesgos
- Centralización de recursos: las fuentes no están en `AssetManager` (actualmente `MenuScene` las busca directamente).
- AssetManager: sin protección contra acceso concurrente y sin gestión de fuentes.
- Falta de pruebas unitarias para `SceneManager`.

Recomendaciones y siguientes pasos
- Mover la carga/gestión de fuentes a `AssetManager` y exponer `getFont(name)` / `loadFontsFrom(path)`.
- Añadir tests unitarios para `SceneManager` (push/pop/replace) y pruebas de carga para `AssetManager`.
- Introducir un `ResourceLocator` para normalizar rutas de assets (repo, build output, instalación, fuentes del sistema).

Resumen rápido
- El módulo `scene` está implementado como POC funcional. Integración con `core` verificada localmente. Pendientes: centralización de recursos y tests automatizados.

Fin del `scene-status`.
