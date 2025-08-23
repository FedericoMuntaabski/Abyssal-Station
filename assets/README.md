# Carpeta de assets
# Carpeta de assets

Esta carpeta contiene los recursos usados por el juego. El proyecto incluye una copia de ejemplo en `assets/` que se copia automáticamente al directorio del ejecutable durante el build.

Contenido actual (ejemplo)

- `assets/fonts/Long_Shot.ttf` — fuente TrueType incluida de ejemplo.
- `assets/textures/background.jpg` — imagen de fondo de ejemplo.
- `assets/sounds/sound_test.wav` — efecto de sonido de ejemplo.

Rutas esperadas por el código

- Texturas: `assets/textures/` (extensiones aceptadas: .png .jpg .bmp)
- Sonidos:  `assets/sounds/` (extensiones aceptadas: .wav .ogg .flac)
- Fuentes:   `assets/fonts/` (extensiones aceptadas: .ttf .otf)

Cómo cargarlos desde el código

En el arranque (por ejemplo en `Game`):

```cpp
AssetManager::instance().loadTexturesFrom("assets/textures");
AssetManager::instance().loadSoundsFrom("assets/sounds");
```

Y luego obtén recursos por nombre (sin extensión):

```cpp
auto tex = AssetManager::instance().getTexture("background");
if (tex) sprite.setTexture(*tex);

auto sfx = AssetManager::instance().getSound("sound_test");
if (sfx) { /* usar buffer */ }
```

Notas sobre fuentes

- SFML soporta TrueType (.ttf) y OpenType (.otf). Coloca tus fuentes en `assets/fonts/`.
- `MenuScene` busca automáticamente archivos `.ttf`/`.otf` dentro de `assets/fonts/` y en rutas comunes (también prueba `assets/textures/arial.ttf` y, en Windows, `C:\Windows\Fonts\arial.ttf`).
- Si ejecutas el binario desde `build/Release`, la carpeta `assets/` se copia al directorio del ejecutable (regla CMake), por lo que las rutas relativas `assets/...` funcionan.

Consejos prácticos

- Mantén nombres de archivo simples (sin espacios) y usa nombres semánticos (por ejemplo `player.png`, `explosion.wav`, `main.ttf`).
- Añade nuevas fuentes a `assets/fonts/` y el juego las encontrará automáticamente.
- Si quieres recursividad o carga más avanzada, puedo ajustar `AssetManager::load*From`.

Si querés que incluya una fuente concreta o que cambie el fallback, decime cuál y lo actualizo.
