---
title: "UI"
module: "ui"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# UI

## 1. Resumen
Subsistema de interfaz: `UIManager`, `MainMenu`, `OptionsMenu`, `InputHelper`, y sistema de `Toast` para notificaciones.

## 2. Estado actual
Funcional; permite remapeo en runtime y persistencia de opciones. Falta iconografía y soporte de combinaciones avanzadas.

## 3. Implementación detallada
- `UIManager`: gestión de widgets, toasts y animaciones; `FontHelper` para caché de fuentes.
- `OptionsMenu`: remapeo integrándose con `InputManager` y guardado inmediato de bindings.
- `InputHelper`: formatea bindings y detecta dispositivo activo.

## 4. Archivos clave
- `src/ui/UIManager.*`, `MainMenu.*`, `OptionsMenu.*`, `InputHelper.*`.

### Notas de cambios (2025-08-24)
- MainMenu: tamaño del título y texto del menú incrementado ~20%, espaciado mejorado para legibilidad.
- **Title Enhancement**: Título del MainMenu ahora tiene borde naranja y posición ajustada (no pegado al borde).
- **Hover Sounds**: Sistema de sonidos hover implementado en todos los menús (MainMenu, OptionsMenu, PauseMenu).
- **UTF-8 Support**: Soporte completo para caracteres acentuados en toda la interfaz.
- **OptionsMenu Rewrite**: Menú de opciones completamente reescrito con controles simples:
  - Volumen Música/UI con controles +/- (0-100%)
  - Selector de resolución (1024x576, 1280x720, 1366x768, 1920x1080)
  - Toggle de pantalla completa
  - Persistencia automática de configuración
- **PauseMenu Enhancement**: Navegación mejorada, "Salir" renombrado a "Volver al menú principal".
- **ConfigManager**: Estructura expandida para soportar volúmenes separados y configuración de pantalla.
- LoadingScreen: texto atmosférico añadido y barra de progreso estilizada.
- CustomCursor: implementado cursor gráfico y ocultación del cursor del sistema; escala por defecto reducida para mayor discreción.
- Audio UI: `MainMenu` reproduce `background_music.wav` en loop; SFX demo ajustado para no hacer loop.
- Logger: corrección de encoding para tildes/acentos, lo que mejora la lectura de mensajes en UI e integraciones.
