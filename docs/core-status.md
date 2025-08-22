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

Cambios recientes en la rama `feature/core`
-----------------------------------------
- Integración básica de assets en `Game`: el constructor ahora busca y carga `assets/textures` y `assets/sounds` (búsqueda hacia arriba desde el working directory), carga `background` y `sound_test` si existen.
- Render: se dibuja el `background` (escalado a la ventana) en `Game::render()` para pruebas visuales.
- Audio: se añadió reproducción de `sound_test` al iniciar `Game::run()` y el SFX se configura en modo loop para demostración.
- FPS: se añadió un contador de FPS que actualiza el título de la ventana una vez por segundo.
- Build/CMake: se añadió la componente `Audio` a `find_package(SFML ...)`, y se incluyó `src/core/AssetManager.cpp` en los sources del ejecutable para resolver símbolos en tiempo de link.

Notas adicionales:
- Los mensajes de error/advertencia siguen usando `std::cerr`; se recomienda reemplazarlos por `core::Logger` para centralizar logs en futuras tareas.
- `AssetManager` continúa siendo no recursivo por defecto; la búsqueda en `Game` compensa la diferencia de working dir al ejecutar el binario desde `Release/`.
