# Carpeta de assets

Esta carpeta contiene un ejemplo de estructura para los recursos del juego.
Sustituye los archivos placeholder por tus recursos reales (imágenes, audio, etc.).

Estructura recomendada:

- assets/
  - textures/
    - background.png    # imagen de fondo (reemplazar por PNG real)
    - player.png        # textura del jugador (reemplazar por PNG real)
  - sounds/
    - explosion.wav     # efecto de sonido (reemplazar por WAV/OGG real)
    - music.ogg         # música (reemplazar por OGG real)

Cómo cargarlos desde el código

En el arranque del `Game` (o en la fase de carga) llama:

```cpp
AssetManager::instance().loadTexturesFrom("assets/textures");
AssetManager::instance().loadSoundsFrom("assets/sounds");
```

Y luego obtén texturas por nombre (sin extensión):

```cpp
auto tex = AssetManager::instance().getTexture("player");
if (tex) sprite.setTexture(*tex);
```

Notas:
- `AssetManager` lista solo el contenido directo de la carpeta. Si necesitas recursividad, añade subcarpetas y modifica `load*From`.
- Reemplaza estos archivos placeholder por assets válidos antes de ejecutar el juego para evitar errores en la carga.
