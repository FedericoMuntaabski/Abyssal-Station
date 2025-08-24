---
mode: agent
---
Prompt para implementación del Main Menu (UI)

Objetivo
Construir el menú principal del juego (estética terror/supervivencia 2D, inspirado en Barotrauma y Lethal Company) usando únicamente los assets disponibles en el repositorio.

Assets disponibles (usar exactamente estos nombres de archivo):
- `assets/textures/Main Menu.jpg` — fondo del menú principal
- `assets/textures/Loading Screen.png` — fondo de la pantalla de carga
- `assets/textures/cursor.jpg` — imagen del cursor personalizado
- `assets/fonts/Main_font.ttf` — fuente principal (títulos)
- `assets/fonts/Secundary_font.ttf` — fuente secundaria (botones, texto UI)
- `assets/sounds/background_music.wav` — música/ambiente de fondo (loop)
- `assets/sounds/hover_select.wav` — sonido al pasar sobre botones
- `assets/sounds/confirm.wav` — sonido al seleccionar/confirmar

Salida esperada
Devuelve una especificación clara y accionable que incluya:
1) Lista de tareas concretas y ordenadas (implementación técnica: archivos a tocar, componentes/sistemas a crear o adaptar, eventos/input a manejar).
2) Requerimientos funcionales y no funcionales (p. ej. soportar teclado y ratón, volumen ajustable, rendimiento mínimo objetivo).
3) Restricciones (usar solo assets listados, evitar nuevas dependencias, compatibilidad con SFML 3.x / configuración del proyecto existente).
4) Criterios de éxito verificables (checks automatizables o pruebas manuales: reproducción de audio, navegación con teclado, transiciones, apariencia del cursor, uso de fuentes, efectos visuales mínimos).
5) Pequeño plan de pruebas/happy-path y 2 casos de borde (sin audio, assets faltantes, resolución distinta).
6) Sugerencias de implementación incremental (pequeños pasos para implementar y validar rápido).

Requerimientos de diseño y comportamiento (implementarlos según lo posible con los assets):
- Fondo: `Main Menu.jpg` con overlay oscuro; aplicar un sutil efecto de glitch/distorsión si es viable.
- Botones: `JugarSolo`, `Options`, `CrearSala`, `Exit` usando `Secundary_font.ttf`. Borde tenue rojo o glow suave. Reproducir `assets/sounds/hover_select.wav` al hover y `assets/sounds/confirm.wav` al seleccionar.
- Cursor: reemplazar cursor del sistema con `cursor.jpg` cuando esté sobre la ventana del juego.
- Tipografía: usar `Main_font.ttf` para el título/logo en la parte superior.
- Transiciones: fade-in / fade-out entre pantallas; usar `Loading Screen.png` como fondo de carga con animación de transición.
- Sonido de fondo: reproducir `background_music.wav` en loop suave; exponer control de volumen en `Options`.
- Input: navegación por teclado (flechas + Enter) y ratón; focus visible en botón seleccionado.
- Animaciones: parpadeo sutil del glow en el botón seleccionado y ligera oscilación del título.

Requisitos funcionales adicionales (específicos solicitados):
- La música de fondo `assets/sounds/background_music.wav` debe reproducirse en loop continuo mientras el menú esté visible.
- `assets/sounds/hover_select.wav` se debe reproducir al pasar el cursor o moverse el foco a un botón; `assets/sounds/confirm.wav` se debe reproducir al activar/confirmar una opción.
- La imagen principal del menú debe ser `assets/textures/Main Menu.jpg`.
- Al iniciar una partida (p. ej. presionar "JugarSolo"), debe mostrarse una transición breve a la pantalla de carga (`assets/textures/Loading Screen.png`) que dure exactamente 5 segundos. La pantalla de carga debe mostrar, centrado, el siguiente texto con una de las fuentes disponibles y tamaño intermedio:

	"La estación ha quedado en silencio. Algo se mueve entre las sombras. Antes de adentrarte, asegúrate de estar listo: cada decisión puede ser la última. Cargando…"

- Los botones principales relacionados con partida (por ejemplo: "Jugar (Solo)", "Crear Sala", "Unirse a Sala") deben situarse en la parte derecha de la pantalla; otros botones como "Options", "Credits", "Exit" pueden ir debajo o agrupados a la derecha según diseño.

Notas de fallback y robustez
- Si algún asset de audio no está disponible, el sistema debe permitir continuar sin audio y registrar en consola la ausencia.
- Si la textura `Main Menu.jpg` falta, usar un color de fondo oscuro y mostrar título y botones en primer plano.

Opciones de UI y persistencia (requerimiento específico)
- Cursor: implementar reemplazo del cursor del sistema por `assets/textures/cursor.jpg` cuando la ventana del juego esté en foco; usar cursor por defecto si el asset no carga.
- Título: mostrar, en la parte derecha superior junto al bloque de botones, un título que diga "ABYSSAL-STATION" usando `Main_font.ttf` con estilo conspicuo (tamaño grande, color claro sobre overlay oscuro).
- Menú de Options: debe ser una pantalla/panel accesible desde el Main Menu que permita:
	- Seleccionar resolución entre opciones 16:9, máximo 4 opciones ordenadas de mayor a menor. Propuesta: `1920x1080`, `1600x900`, `1365x768`, `1280x720`.
	- Alternar modo Fullscreen / Windowed.
	- Ajustar volumen de la música de fondo (`background_music.wav`) y volumen de los sonidos UI (hover + confirm) por separado (rangos 0-100 o 0.0-1.0).
	- Botón "Guardar" que persista la configuración para futuras sesiones.
	- Botón "Volver" y la tecla ESC deben cerrar el menú de Options y volver al Main Menu sin guardar (a menos que se haya presionado "Guardar").

Persistencia y formato
- Guardar las preferencias en el sistema de configuración existente si existe (`config/config.json`), o en un nuevo archivo `config/ui_settings.json` si no hay un manager central. El archivo debe incluir: resolution, fullscreen (bool), music_volume (float), ui_volume (float).
- Al arrancar el juego, cargar estas preferencias y aplicarlas (resolución, fullscreen, volúmenes, cursor), con validación y fallback si la resolución no es soportada.

Mejoras al menú de pausa
- El menú de pausa debe tener características visuales y de interacción consistentes con el Main Menu: overlay oscuro, uso de `Secundary_font.ttf` para opciones, glow rojo tenue en el item seleccionado y reproducción de los sonidos UI.
- Input: permitir navegación por teclado usando W/S o Up/Down y selección con Enter; también permitir navegación y selección con el mouse (hover y click izquierdo). Al cambiar el foco o pasar el cursor, reproducir `assets/sounds/hover_select.wav`; al activar una opción, reproducir `assets/sounds/confirm.wav`.
- Opciones típicas del menú de pausa: "Continuar", "Options", "Volver al Main Menu", "Salir". El botón "Volver al Main Menu" debe llevar al jugador al Main Menu (con la transición de fade-out/fade-in correspondiente).
- El menú de pausa debe poder cerrarse con la tecla ESC (reanudando la partida si estaba pausada) y debe respetar la configuración de audio y resolución ya cargada.

Reconstrucción del módulo Options, documentación y tests
- Si la arquitectura actual no soporta las opciones requeridas (persistencia, resolución, toggles, volúmenes), vuelve a diseñar o refactorizar el módulo de Options de forma modular. Documenta las decisiones de diseño en el código y en la carpeta `docs/`.
- Añade o actualiza un documento en `docs/modules/` (por ejemplo `docs/modules/UI.md` o `docs/modules/Options.md`) con un resumen de los cambios implementados, rutas de archivos modificados y cómo probar las nuevas funciones.
- Implementa tests unitarios o de integración mínimos para cubrir: carga/guardado de configuración, aplicación de resolución/fullscreen, reproducción en loop de la música y reproducción de sonidos UI en hover/confirm. Añade los tests en `tests/ui/` siguiendo la convención existente y actualiza `tests/CMakeLists.txt` si es necesario.
- Antes de marcar la tarea como completa, ejecutar la suite de tests y documentar en el resumen de `docs/` los resultados y cualquier fallback o limitación conocida.

Restricciones técnicas
- No introducir dependencias externas nuevas sin justificar (usar lo que ya hay en el proyecto, p. ej. SFML 3.x).
- Manejar la ausencia de un asset con fallback (ej. color sólido, texto en lugar de imagen, cursor por defecto).
- Mantener performance razonable en resoluciones comunes (720p/1080p).

Formato de la respuesta
Entregar la especificación en español, con secciones claras: "Tareas", "Requerimientos", "Restricciones", "Criterios de éxito", "Plan de pruebas" y "Pasos de implementación incremental". Incluir rutas de archivos concretas a modificar o agregar y un checklist final de verificación.

Éxito
El prompt se considerará correcto si produce una especificación accionable que permita a un desarrollador implementar y validar el Main Menu usando únicamente los assets listados.

Adición: mejoras en Core, UI y Scene
----------------------------------

Además de la especificación del Main Menu, este prompt debe indicar mejoras concretas y accionables para los módulos `core`, `ui` y `scene` a fin de soportar correctamente las funcionalidades del menú principal y la pantalla de carga.

Requerimientos adicionales de mejora (resumen técnico):

- Core (archivos sugeridos: `src/core/AssetManager.*`, `src/core/ConfigManager.*`, `src/core/Game.*`, `src/core/Timer.*`, `src/core/Logger.*`):
	- Añadir o reforzar un AssetManager que exponga carga asíncrona/segura de texturas, fuentes y sonidos y devuelva fallbacks cuando los assets falten. API mínima: `loadTexture(name, path)`, `getTexture(name)`, `loadFont`, `getFont`, `loadSoundBuffer`, `getSoundBuffer`.
	- ConfigManager: garantizar carga/validación/guardado de `config/config.json` y `config/ui_settings.json` (si no existe), con funciones `loadSettings()` y `saveSettings()` y esquema simple de validación.
	- Game: exponer hooks/eventos para transiciones entre escenas, y un manager de estado (`pushState`, `popState`, `changeState`) que pueda coordinar fade-in/fade-out y entrada en pantalla de carga.
	- Timer/delta: asegurar un `Timer` con `getDelta()` estable para animaciones y transiciones (p. ej. oscilación del título, parpadeo de glow).
	- Logger: registrar eventos críticos (assets faltantes, reproducción de audio fallida, cambio de resolución, guardado de settings).

- UI (archivos sugeridos: `src/ui/Menu.*`, `src/ui/UIManager.*`, `src/ui/MainMenu.*`, `src/ui/OptionsMenu.*`, `src/ui/PauseMenu.*`, `src/ui/InputHelper.*`):
	- Asegurar que `Menu` y las implementaciones (`MainMenu`, `PauseMenu`, `OptionsMenu`) usen el AssetManager para cargar texturas, fuentes y sonidos.
	- UIManager: mantener una pila/stack de menús y exponer `openMenu`, `closeMenu`, `update`, `render`. Debe integrarse con el sistema de input para delegar foco y manejar navegación por teclado y ratón.
	- InputHelper: unificar mapeos (teclado y mouse) y exponer métodos para preguntar por `isActionPressed("up")`, `isActionPressed("confirm")`, `isActionPressed("back")`.
	- Sonidos UI: crear un pequeño AudioHelper/AudioManager (puede vivir en UI o core) que reproduzca sonidos UI con control de volumen separado y loguee si un sonido falla.
	- Menús: implementar focus visual, animación de glow (pulsante), reproducción de `hover_select.wav` y `confirm.wav` según acciones.

- Scene (archivos y conceptos: `src/scene/SceneManager.*`, `src/scene/Scene.*` o integración con `GameState.h`):
	- SceneManager debe orquestar la transición entre escenas (MainMenu -> Loading -> Gameplay) con cross-fade y duraciones configurables.
	- Integración con pantalla de carga: al iniciar partida, SceneManager debe mostrar `assets/textures/Loading Screen.png` y un texto centrado por exactamente 5 segundos antes de cargar la escena de juego.
	- Exponer eventos para `onSceneEnter`, `onSceneExit` y `onLoadingComplete` para que UI y Core puedan reaccionar.

Detalles operativos y entregables para estas mejoras:

- Archivos a tocar o crear (mínimo recomendado):
	- src/core/AssetManager.h/.cpp (si falta, crear). Mejorar si existe.
	- src/core/ConfigManager.h/.cpp (extender métodos de carga/guardado con `ui_settings.json`).
	- src/core/Game.h/.cpp (añadir hooks de escena y estado).
	- src/ui/UIManager.h/.cpp (asegurar stack de menús y delegación de input).
	- src/ui/AudioHelper.h/.cpp (pequeño helper para sonidos UI y música).
	- src/scene/SceneManager.h/.cpp (crear o integrar con GameState para transiciones y loading screen).
	- docs/modules/UI.md y docs/modules/Options.md (documentar cambios y pruebas manuales).
	- tests/ui/* (tests para carga/guardado de settings y reproducción simple de audio).

- Criterios de éxito para las mejoras (verificables):
	- AssetManager devuelve fallbacks y el Logger registra ausencia de asset, sin bloquear el menú.
	- ConfigManager persiste `ui_settings.json` con los campos: resolution, fullscreen, music_volume, ui_volume.
	- SceneManager realiza la transición de 5s hacia la pantalla de carga y emite `onLoadingComplete` al finalizar.
	- AudioHelper reproduce `background_music.wav` en loop con volumen configurable y UI sounds con su volumen independiente.

- Recomendaciones de commits y rama:
	- Rama: `feature/ui-core-scene/menus-improvements`.
	- Commits atómicos y descriptivos: `[feature] asset manager async load`, `[feature] ui manager menu stack`, `[feature] scene manager loading transition`, `[test] ui settings persistence`.

Checklist final (incluir en la especificación entregada al desarrollador):
	- [ ] Implementar Main Menu según especificación original (assets, botones, transiciones, audio, cursor).
	- [ ] Implementar persistencia UI en `config/ui_settings.json` y carga al inicio.
	- [ ] Mejorar/crear AssetManager con fallbacks asíncronos.
	- [ ] Añadir AudioHelper para música y sonidos UI con volumen independiente.
	- [ ] Crear SceneManager / integrar con GameState para pantalla de carga de 5 segundos.
	- [ ] Documentar cambios en `docs/modules/` y añadir pruebas en `tests/ui/`.

Trabajo pendiente detectado (según implementación actual en la rama `feature/menus`)
---------------------------------------------------------------
Los siguientes elementos no están completamente implementados o requieren trabajo adicional para cumplir al 100% la especificación original. Incluyo archivos concretos donde aplicar cambios y una nota rápida del estado actual.

- Persistencia de volúmenes por canal
	- Estado: parcial. Existe `m_volume` global en `OptionsMenu` y `ConfigManager` pero no hay campos separados para `music_volume`, `sfx_volume` y `master_volume`.
	- Archivos a modificar: `src/core/ConfigManager.cpp` / `.h`, `src/ui/OptionsMenu.cpp`.
	- Tarea: añadir campos, exponer getters/setters, persistir en `config/ui_settings.json` (o `config/config.json`) y aplicar niveles al AudioHelper/AudioManager.

- Toggle de Fullscreen y persistencia de resolución
	- Estado: parcial. `OptionsMenu` lista resoluciones y `ConfigManager` persiste resolution, pero falta toggle `fullscreen` y aplicación robusta al crear la ventana.
	- Archivos a modificar: `src/ui/OptionsMenu.cpp`, `src/core/ConfigManager.*`, `src/core/Game.cpp` (aplicar fullscreen al initWindow o vía un hook de ConfigManager).

- AudioHelper / AudioManager central
	- Estado: pendiente. El MainMenu reproduce `background_music.wav` y SFX existen, pero no hay una capa central clara para controlar volúmenes por canal ni fallback robusto.
	- Archivos a crear/modificar: `src/ui/AudioHelper.h/.cpp` o `src/core/AudioManager.*`, actualizar `src/ui/MainMenu.cpp`, `src/ui/UIManager.cpp` para usarlo.

- Menú de Pausa: overlay y sonidos
	- Estado: parcial. `PauseMenu` existe y abre `OptionsMenu`, pero falta overlay semitransparente consistente con MainMenu y reproducir `hover_select.wav`/`confirm.wav` en navegación.
	- Archivos a modificar: `src/ui/PauseMenu.cpp`, `src/ui/UIManager.cpp` (para sonidos y estado pausado visual), `assets` si ajustes se requieren.

- Tests UI faltantes
	- Estado: no implementados. Se crearon e hicieron correr tests de integración existentes, pero faltan los tests solicitados:
		- `tests/ui/MainMenuTests.cpp`
		- `tests/ui/AudioTests.cpp`
		- `tests/ui/AssetLoadingTests.cpp`
	- Tarea: crear los tests, añadir a `tests/CMakeLists.txt` y ejecutar `ctest`.

- AssetManager: carga asíncrona / hot-reload
	- Estado: parcialmente cubierto. Se añadieron APIs sincrónicas (`hasTexture`, `removeTexture`, `reloadTexturesFrom`) pero no hay carga asíncrona ni watcher para hot-reload.
	- Archivos a modificar: `src/core/AssetManager.*` (implementar hilo de carga o API asíncrona si se desea), `src/core/Game.cpp` (si se quiere recargar automáticamente al detectar cambios en disco).

- SceneManager: hooks de transición y eventos `onLoadingComplete`
	- Estado: el flujo de escenas funciona y `LoadingScene` existe; sin embargo, la señalización formal `onLoadingComplete` o un hook observable no está estandarizada.
	- Archivos a modificar: `src/scene/SceneManager.*`, `src/scene/Scene.*` (añadir virtuales `onEnter/onExit/onLoadingComplete`) y actualizar las llamadas desde `LoadingScene`.

- Documentación y tests de persistencia
	- Estado: docs de módulos actualizadas parcialmente (se agregaron notas). Falta agregar el test automatizado de persistencia de `ui_settings.json` y documentar el formato exacto en `docs/modules/Core.md` o `docs/modules/UI.md`.
	- Archivos a crear/modificar: `tests/ui/AssetLoadingTests.cpp`, `docs/modules/Options.md` (si se prefiere más específico), `src/core/ConfigManager.*` para documentación del esquema.

Si quieres, procedo a implementar uno de estos puntos ahora (elige uno o indícame prioridad). Puedo hacer cambios atómicos y ejecutar build/tests tras cada cambio.

Notas de compatibilidad y restricciones técnicas (recordatorio):
	- Mantener compatibilidad con SFML 3.x y no introducir dependencias externas.
	- Manejar la ausencia de assets con fallbacks (cursor por defecto, color de fondo oscuro).
	- Evitar lógica de jugador o networking en `core`.
