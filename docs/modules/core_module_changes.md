(2025-08-24) Cambios en Core:

- Se integró `AudioManager` en el stack del core; `Game` ahora crea/posee una instancia y las escenas pueden solicitar cambios de contexto de audio.
- `ConfigManager` extendido para exponer volúmenes separados (master/music/sound) y guardado automático; se validaron getters/setters y compatibilidad hacia atrás.
- `SaveManager` auto-save en PlayScene (cada 2 minutos) confirmado y logs asociados añadidos.

