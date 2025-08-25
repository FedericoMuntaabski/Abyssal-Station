(2025-08-24) Cambios generales en módulos varios:

- `AchievementManager` implementado en `src/gameplay/AchievementManager.*` con persistencia JSON, tipos de logros (colección, supervivencia, exploración, combate) y notificaciones en UI.
- `AudioManager` implementado en `src/audio/AudioManager.*` con crossfade, fades, control de volúmenes y API de contextos (Menu, Gameplay, Combat, Exploration, Danger).
- Integración de `AchievementManager` y `AudioManager` con `PlayScene` y menús para notificaciones y audio contextual.

