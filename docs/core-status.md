# Estado actual del Core (feature/core)

Resumen corto
- Rama: `feature/core` (checkout realizado).
- Objetivo: Documento guía que describe el estado actual del "core" (núcleo) del motor para orientar desarrollo futuro.

Requisitos del encargo
- [x] Cambiar a la rama `feature/core`. (Hecho)
- [x] Generar un documento describiendo el estado actual del core. (Este archivo)

Archivos leídos
- `src/core/Game.h` / `src/core/Game.cpp`
- `src/core/AssetManager.h` / `src/core/AssetManager.cpp`
- `src/core/Logger.h` / `src/core/Logger.cpp`
- `src/core/Timer.h` / `src/core/Timer.cpp`
- `src/main.cpp`

Visión general del core

Componentes principales

1) Game
- Clase: `Game` (encapsula ventana SFML y bucle principal).
- API pública: `Game(width, height, title)`, `run()`, `stop()`.
- Responsabilidades: inicializar ventana SFML, procesar eventos, actualizar y renderizar cada frame.
- Implementación: usa `sf::RenderWindow`, `sf::Clock` para delta time, `pollEvent()` (SFML3 optional API). Se fija frame limit a 60 FPS.
- Estado: Implementado y funcional como "shell" (no hay lógica de juego, ni sistemas registrados). Maneja cierre y resize (ajusta View).
- Notas: `update()` y `render()` están vacíos como placeholders; `processEvents()` filtrado básico.

2) AssetManager
- Clase: `AssetManager` (singleton).
- API pública: `instance()`, `loadTexturesFrom(folder)`, `loadSoundsFrom(folder)`, `getTexture(name)`, `getSound(name)`.
- Responsabilidades: cargar texturas y buffers de audio desde directorios usando std::filesystem; almacenar en unordered_map con shared_ptr.
- Estado: Implementado y funcional para cargas sencillas desde disco; registra fallos por `std::cerr`.
- Limitaciones conocidas:
  - No hay manejo de subdirectorios recursivos.
  - No hay hot-reload ni gestión avanzada de memoria (no unloading explícito).
  - No configuración de rutas base; se pasan rutas completas relativas/absolutas.
  - No pruebas unitarias.

3) Logger
- Espacio de nombres: `core::Logger` (singleton façade) con una implementación interna `LoggerImpl`.
- API pública: `instance()`, `enableConsole(bool)`, `setLogFile(path)`, `info/warning/error(string)`.
- Responsabilidades: salida a consola y a fichero con timestamp; thread-safe (mutex) para operaciones de logging.
- Estado: Implementado; soporta habilitar/deshabilitar consola y abrir archivo en append.
- Notas: formatea timestamps y niveles; usa `localtime_s` en Windows y `localtime_r` en POSIX.

4) Timer / TimerScope
- Clases: `Timer` (delta, elapsed, restart) y `TimerScope` (RAII para medir duración de scope).
- API pública: `reset()`, `elapsed()`, `restart()`, `delta()` y `TimerScope(callback)`.
- Estado: Implementadas y utilitarias.

5) main.cpp
- `main()` crea `Game` y llama `run()`; atrapa excepciones y sale con EXIT_FAILURE en error no manejado.
- Estado: Lista para ejecutar el bucle principal.

Contratos mínimos (inputs/outputs, errores)
- Game::run(): bloqueante; usa excepción en constructor para fallos de inicialización.
- AssetManager::load*From(path): intenta enumerar `path`; en errores escribe a std::cerr.
- Logger::setLogFile(path): devuelve bool indicando si se abrió correctamente.

Casos borde y riesgos
- Recursos faltantes: Si carpetas de assets faltan, se loguea fallo, pero no hay excepción.
- Concurrencia: Logger es thread-safe; AssetManager no está protegido por mutex (potencial problema si se carga desde múltiples hilos).
- Rendimiento: carga sin streaming ni caching en disco aparte del map; puede necesitar cargas en background para proyectos grandes.
- SFML API: Código usa características acordes a SFML 3 (VideoMode con Vector2u, optional event API). Requiere confirmar dependencia vinculada a SFML 3.
- Paths: Uso de std::filesystem asume C++17+ y plataformas compatibles.

Cambios aplicados en `feature/core`
----------------------------------
Resumen detallado de cambios aplicados en esta rama (delta y motivos):

- src/core/Game.h
  - Añadidos miembros para soporte de FPS (título base, acumulador, contadores).
  - Añadidos miembros para pruebas de assets: `m_backgroundTexture`, `m_sfxBuffer` y `m_sound`.

- src/core/Game.cpp
  - `initWindow()` ahora almacena el título base en `m_title` para mostrar FPS.
  - Implementada búsqueda hacia arriba en el filesystem para localizar `assets/textures` y `assets/sounds` incluso si el exe se ejecuta desde `Release/`.
  - Llamadas a `AssetManager::loadTexturesFrom(...)` y `loadSoundsFrom(...)` tras inicializar la ventana.
  - Carga de `background` (si existe) y dibujo escalado del background en `render()` para una prueba visual.
  - Carga de `sound_test` (si existe) y construcción de `sf::Sound` con el buffer; reproducción al iniciar `run()` y configuración en loop para demo.
  - Lógica de FPS: acumulador y actualización del título una vez por segundo.

- CMakeLists.txt
  - `find_package(SFML ...)` ahora incluye el componente `Audio`.
  - `src/core/AssetManager.cpp` añadido a las fuentes del ejecutable para resolver símbolos de AssetManager en el enlace.

- assets/ (nuevo en el repo)
  - Se añadió una estructura de ejemplo `assets/textures` y `assets/sounds` con placeholders para pruebas locales.

Commits relevantes
- 45b5e0f: feature/core: update Game to show FPS in title; update core-status doc
- d17f237: feature/core: loop sound_test; document recent core changes

Notas y recomendaciones
- Logging: actualmente las advertencias y errores siguen usando `std::cerr`. Se recomienda sustituir por `core::Logger::instance()` para centralizar trazas y soportar archivo/filtrado por nivel.
- AssetManager: sigue siendo no recursivo. Si se prefiere cargar subdirectorios, conviene añadir un parámetro `recursive` a las funciones de carga o una carga recursiva central.
- Recursos de audio: para pistas largas usar `sf::Music` (streaming) en lugar de `sf::SoundBuffer` para reducir uso de RAM.

Si quieres que deje estos cambios en un changelog separado o que incluya ejemplos de código en esta misma página (cómo inicializar logger, cómo cargar assets en `main`), lo agrego enseguida.
