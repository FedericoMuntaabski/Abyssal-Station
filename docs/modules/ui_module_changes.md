---
title: "UI - Cambios Recientes"
module: "ui"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [changes, fixes]
---

# Cambios recientes

- Se corrigieron caracteres con tildes en strings de UI y `LoadingScene` para evitar problemas de encoding.
- `OptionsMenu` reescrito con manejo de excepciones y persistencia de bindings; ya no provoca crash al abrir o guardar.
- `PauseMenu` arreglado para aceptar entrada de mouse y teclado; logs añadidos para depuración de entrada y control de audio SFX (hover/confirm).
- Integración con `AudioManager` para SFX y música de fondo desde menús (MainMenu y PauseMenu verificados).

