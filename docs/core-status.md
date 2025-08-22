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

## Cómo compilar y probar (PowerShell)

Pasos reproducibles para configurar, compilar y ejecutar un smoke test en Windows PowerShell (ejecutar desde la raíz del repo):

```powershell
# 1) Configurar el directorio de build y generar (x64 Release)
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release

# 2) Compilar (Release)
cmake --build build --config Release

# 3) Ejecutar el binario resultante (ruta probable)
if (Test-Path "build/Release/AbyssalStation.exe") {
    & "build/Release/AbyssalStation.exe"
} elseif (Test-Path "Release/AbyssalStation.exe") {
    & "Release/AbyssalStation.exe"
} else {
    Write-Host "Binario no encontrado. Revisa el output del build o busca AbyssalStation.exe en la carpeta build/ o Release/."
}

# Resultado esperado mínimo:
# - El ejecutable arranca y abre la ventana SFML sin lanzar excepciones.
# - El proceso devuelve exit code 0 al cerrarse (o cierra sin errores visibles).
```

Notas:
- Si usás Visual Studio como generador, necesitás tener la versión adecuada instalada.
- En CI conviene usar el flag `--config Release` al invocar `cmake --build`.

## Build status

- Estado: Manual / Unknown
- Última verificación: 2025-08-22

Nota: Actualmente el estado se actualiza manualmente aquí; se recomienda añadir un workflow de CI (GitHub Actions) que actualice un badge automáticamente.

## Pre-merge checklist (must-have)

Antes de abrir un merge request hacia `develop`/`main`, verificar lo siguiente en la rama `feature/core`:

- [ ] Compilar en Release (PowerShell): `cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release; cmake --build build --config Release`
- [ ] Ejecutar smoke test: ejecutar el binario y confirmar que el juego arranca y cierra sin excepciones.
- [ ] Ejecutar tests unitarios básicos (si existen):

```powershell
# Ejecutar pruebas con CTest (si se definieron los tests en CMake)
ctest --test-dir build -C Release --output-on-failure
```

- [ ] Revisar no usar `std::cerr`/`std::cout` en `src/core` (usar `core::Logger`).

```powershell
Select-String -Path "src/core/**/*.*" -Pattern 'std::cerr','std::cout' -SimpleMatch
```

- [ ] Actualizar `docs/core-status.md` con la fecha y resultado de la verificación (marcar Build status apropiadamente).

Si querés, puedo ejecutar la compilación localmente desde aquí y actualizar el campo "Build status" con el resultado.
