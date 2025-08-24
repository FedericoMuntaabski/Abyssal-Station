# Implementation changes summary (2025-08-24)

Breve resumen de cambios aplicados en la iteración actual:

- Core / Logger
  - Se corrigió la salida de consola para soportar UTF-8/accented characters en Windows: SetConsoleOutputCP(CP_UTF8) y uso de WriteConsoleW cuando es posible, con fallback a bytes UTF-8.

- AssetManager
  - Añadidas utilidades para invalidar y recargar assets en runtime: `hasTexture`, `removeTexture`, `reloadTexturesFrom`.

- Game / Scene rendering
  - Se movió la responsabilidad de dibujar el background desde `Game` a las escenas individuales. `Game` ya no dibuja un fondo global; las escenas deben dibujar el suyo si lo requieren.

- UI
  - Main menu y loading screen: aumento de tamaño de título/menú (~20%), espaciado mejorado, texto de carga atmosférico y barra de progreso estilizada.
  - Implementado cursor gráfico (`CustomCursor`) y ocultación del cursor del sistema.

- Audio
  - `MainMenu` reproduce `assets/sounds/background_music.wav` en loop.
  - Demo SFX no hace loop para evitar reproducir múltiples pistas en paralelo.

- Tests / Build
  - El proyecto compila y el ejecutable Release fue generado. Algunos tests ya están integrados bajo `build/tests/Release/`.

Verificación realizada
- Compilé y ejecuté el juego localmente. Verifiqué:
  - Logs con acentos (tildes) se muestran correctamente.
  - `LoadingScene` y `MainMenu` cargan sus fondos localmente; ya no aparece fondo obsoleto cuando un asset se elimina del disco.
  - Custom cursor aparece y la música de fondo suena en loop mientras el menú está visible.

Siguientes pasos recomendados
- Añadir persistencia por canal de audio (master/music/sfx) en `ConfigManager` y controles en `OptionsMenu`.
- Implementar overlay y sonidos en `PauseMenu` para consistencia visual.
- Añadir tests UI para MainMenu, audio y carga de assets.

Archivo generado automáticamente por cambios en la rama `feature/menus`.
