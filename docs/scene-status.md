# Estado actual del Módulo `scene`

Este documento describe el estado actual del módulo `src/scene` (rama activa: `feature/scene`), su integración con el `core`, el comportamiento observado en tiempo de ejecución y recomendaciones para los próximos pasos — en el mismo estilo del `core-status`.

Resumen corto
- Objetivo: Describir la implementación del sistema de escenas (interfaz, gestor, escenas de ejemplo), su integración con `Game` y el comportamiento observado tras la última iteración de desarrollo.
- Estado general: Implementado y funcional. La app compila y ejecuta localmente; `MenuScene` y `PlayScene` operan (texto, transición y control simple). Las escenas utilizan `core::Logger` para trazas.

Checklist (qué incluye este informe)
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Detallar comportamiento de `MenuScene` y `PlayScene` y manejo de assets (fuentes).
- [x] Documentar integración con `Game` y ajustes en `CMakeLists.txt` relacionados.
- [x] Presentar pruebas rápidas (build y smoke test) y limitaciones actuales.

Archivos leídos / relevantes
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp`

Visión general y propósito
El módulo `scene` introduce una abstracción de pantallas (scenes) para separar la lógica de estado y UI del `Game` principal. Su finalidad es:
- permitir múltiples pantallas (menú, juego, pausa, etc.)
- gestionar transiciones (push/pop/replace) con una pila LIFO
- delegar el flujo principal (eventos, update, render) a la escena activa

1) `Scene` (interfaz)
- Ubicación: `src/scene/Scene.h`.
- Contrato (resumido):
  - virtual ~Scene() = default;
  - virtual void handleEvent(sf::Event &event) = 0;
  - virtual void update(float dt) = 0;
  - virtual void render(sf::RenderWindow &window) = 0;
  - hooks opcionales: onEnter(), onExit() para inicialización/limpieza.
- Propósito: forzar a las escenas a implementar el ciclo básico y permitir hooks de ciclo de vida.

2) `SceneManager`
- Ubicación: `src/scene/SceneManager.h` / `.cpp`.
- Implementación: gestiona una `std::stack<std::unique_ptr<Scene>>`.
- API pública (principales métodos):
  - void push(std::unique_ptr<Scene> scene);
  - void pop();
  - void replace(std::unique_ptr<Scene> scene); // pop + push atomico
  - Scene* current() const; // puntero a la escena activa o nullptr
  - void handleEvent(sf::Event&), update(float dt), render(sf::RenderWindow&)
- Responsabilidades adicionales: invocar `onEnter()` y `onExit()` al cambiar escenas y registrar transiciones mediante `core::Logger`.

3) `MenuScene` (ejemplo)
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento:
  - Carga una fuente en `onEnter()` — implementada una búsqueda robusta por `assets/fonts/*.ttf` y rutas alternativas (para mitigar la ejecución desde `build/Release`).
  - Renderiza texto tipo "Presione Enter para jugar" si la fuente se carga.
  - En `handleEvent()`: Detecta Enter → llama a `sceneManager->replace(std::make_unique<PlayScene>(...))`; Detecta Escape → `pop()` (salir del menú / cerrar escena).
  - Registra acciones y fallos con `core::Logger`.
- Notas: la búsqueda de fuentes está en `MenuScene` (aún no está centralizada en `AssetManager`).

4) `PlayScene` (ejemplo jugable mínimo)
- Ubicación: `src/scene/PlayScene.*`.
- Comportamiento:
  - Dibuja un `sf::RectangleShape` rojo como marcador de jugador.
  - `update(dt)` mueve el rectángulo con las teclas de flecha (velocidad basada en `dt` y normalización de vector de movimiento).
  - `handleEvent()`: Detecta Escape y hace `replace(MenuScene)`.

Integración con `core` / `Game`
- `Game` ahora contiene `std::unique_ptr<scene::SceneManager> m_sceneManager`.
- En `Game::run()` se crea el `SceneManager`, se hace `push` de `MenuScene` al iniciar y se delegan los tres pasos principales:
  - eventos -> `m_sceneManager->handleEvent(ev)`
  - actualización -> `m_sceneManager->update(dt)`
  - render -> `m_sceneManager->render(window)`
- Si la pila de escenas queda vacía, el juego termina de forma limpia.

Build / CMake y assets
- `CMakeLists.txt` se actualizó para incluir los nuevos archivos fuente del módulo `scene` y asegurar que `Logger.cpp` y `Timer.cpp` estén en las fuentes del ejecutable (evita errores de enlace).
- Se añadió un paso post-build para copiar la carpeta `assets/` al directorio del ejecutable (por ejemplo `build/Release/assets`) para que rutas relativas como `assets/fonts/Long_Shot.ttf` funcionen cuando se ejecuta el binario desde `build/Release`.

Quality gates y pruebas rápidas realizadas
- Compilación: El proyecto compila con éxito tras incluir los nuevos archivos en `CMakeLists.txt`.
- Smoke test: Ejecutado el binario en `build/Release`.
  - Observado: `MenuScene` carga `assets/fonts/Long_Shot.ttf` (post-build copy presente).
  - Transiciones: Enter → `PlayScene`, Escape → `MenuScene`.
  - Movimiento: en `PlayScene` el rectángulo responde a flechas.

Casos borde y limitaciones conocidas
- Carga de fuentes: actualmente en `MenuScene` mediante búsqueda en disco; sería preferible centralizar en `AssetManager`.
- AssetManager: no soporta carga recursiva ni protección contra acceso concurrente; las fuentes no están gestionadas por él aún.
- Dependencia SFML: código adaptado a SFML 3 (event API variant, Text/Font cambios). Requiere la versión correcta de SFML para compilar/executar.
- Tests: no hay tests unitarios para `SceneManager`; sería útil al manipular la pila y la delegación.

Recomendaciones y próximos pasos
- Mover la gestión/registro de fuentes a `AssetManager` y exponer `getFont(name)` o `loadFontsFrom(path)`.
- Añadir tests unitarios para `SceneManager` (push/pop/replace) y para la carga básica de assets (AssetManager).
- Crear un `ResourceLocator` o servicio de resolución de rutas para normalizar la búsqueda de assets (repo root, build output, instalación, fuentes del sistema).
- Añadir una simple prueba de integración en CI que compile, copie assets y ejecute una comprobación no interactiva (p. ej. inicializa `Game`, hace un ciclo y comprueba que no se crashée).

Archivos y responsabilidades (resumen rápido)
- `src/scene/Scene.h` — interfaz base de escena.
- `src/scene/SceneManager.{h,cpp}` — gestor de pila de escenas, delegación y logging.
- `src/scene/MenuScene.{h,cpp}` — escena menú; gestión de fuente y transición a PlayScene.
- `src/scene/PlayScene.{h,cpp}` — escena jugable mínima con movimiento y Escape->Menu.

Estado actual (línea de tiempo)
- Implementación: completa como demo/POC. Integración y build verificados localmente.
- Falta: pruebas automatizadas, centralización de recursos (fuentes) y documentación de API para `AssetManager` respecto a fuentes.

Si quieres que implemente ahora alguna de las siguientes tareas, indícala y la hago:
- [ ] Mover la carga de fuentes a `AssetManager` y actualizar `MenuScene` para consumirlo (implementación + prueba).
- [ ] Añadir tests unitarios para `SceneManager`.
- [ ] Añadir una tarea de CI/Build que copie `assets/` y ejecute un smoke test.

Fin del `scene-status`.
