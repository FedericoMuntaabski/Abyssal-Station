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
Funcional con mejoras criticas implementadas; permite remapeo en runtime y persistencia de opciones. Sistema de notificaciones mejorado con colores contextuales.

## 3. Implementación detallada
- `UIManager`: gestión de widgets, toasts y animaciones; `FontHelper` para caché de fuentes.
- `OptionsMenu`: remapeo integrándose con `InputManager` y guardado inmediato de bindings.
- `InputHelper`: formatea bindings y detecta dispositivo activo.

## 4. Archivos clave
- `src/ui/UIManager.*`, `MainMenu.*`, `OptionsMenu.*`, `InputHelper.*`.

### Notas de cambios (2025-08-24)
#### **✅ Fixes Criticos Implementados (Fase 1)**

**1. Eliminacion de Tildes/Caracteres UTF-8**
- Removidas todas las tildes de strings de UI para evitar problemas de encoding
- **Cambios**: 
  - "Volumen Música" → "Volumen Musica"
  - "Resolución" → "Resolucion" 
  - "Volver al menú principal" → "Volver al menu principal"
  - "Sí" → "Si"
  - "implementación" → "implementacion"
- **Estado**: ✅ Completado - Todos los textos renderizan correctamente

**2. Fix del Menu de Opciones**
- Menu de opciones ya no crashea al abrirse desde el menu principal
- ConfigManager se pasa correctamente a OptionsMenu via UIManager
- **Estado**: ✅ Completado - Navegacion sin crashes

**3. Fix de Transicion "Jugar (Solo)"**  
- Implementados event callbacks en MenuScene para manejar transiciones
- "Jugar (Solo)" ahora transiciona correctamente: MenuScene → LoadingScene → PlayScene
- Agregado flag m_gameStarting para prevenir multiples activaciones
- **Estado**: ✅ Completado - Transicion funcional

#### **✅ Mejoras UX Implementadas (Fase 2-3)**

**4. Sistema de Notificaciones Mejorado**
- Agregados metodos convenience para notificaciones con colores contextuales:
  - `showErrorNotification()` - Rojo para errores
  - `showWarningNotification()` - Amarillo para advertencias  
  - `showSuccessNotification()` - Verde para exitos
  - `showInfoNotification()` - Cian para informacion
- **Estado**: ✅ Completado - API expandida disponible

**5. Prevencion de Spam de Input**
- MainMenu implementa flag m_gameStarting para prevenir multiples triggers
- Mejora en responsividad de botones y prevencion de acciones repetidas
- **Estado**: ✅ Completado - UX mejorada

#### **📋 Cambios Existentes Mantenidos**
- MainMenu: tamaño del título y texto del menú incrementado ~20%, espaciado mejorado para legibilidad.
- **Title Enhancement**: Título del MainMenu ahora tiene borde naranja y posición ajustada (no pegado al borde).
- **Hover Sounds**: Sistema de sonidos hover implementado en todos los menús (MainMenu, OptionsMenu, PauseMenu).
- **OptionsMenu Rewrite**: Menú de opciones completamente reescrito con controles simples:
  - Volumen Musica/UI con controles +/- (0-100%)
  - Selector de resolucion (1024x576, 1280x720, 1366x768, 1920x1080)
  - Toggle de pantalla completa
  - Persistencia automática de configuración
- **PauseMenu Enhancement**: Navegación mejorada, "Salir" renombrado a "Volver al menu principal".
- **ConfigManager**: Estructura expandida para soportar volúmenes separados y configuración de pantalla.
- LoadingScreen: texto atmosférico añadido y barra de progreso estilizada.
- CustomCursor: implementado cursor gráfico y ocultación del cursor del sistema; escala por defecto reducida para mayor discreción.
- Audio UI: `MainMenu` reproduce `background_music.wav` en loop; SFX demo ajustado para no hacer loop.
- Logger: corrección de encoding para tildes/acentos, lo que mejora la lectura de mensajes en UI e integraciones.

### Cambios recientes (2025-08-24) — delta

- Se corrigieron tildes en strings de UI y en `LoadingScene` para evitar problemas de encoding y se normalizaron textos donde fue necesario.
- `OptionsMenu` recibió un rewrite con manejo robusto de errores; ya no crashea y persiste bindings en `config/input_bindings.json`.
- `PauseMenu` mejorado: ahora recibe entrada por mouse y teclado correctamente, con logs de depuración añadidos para trazabilidad y sonidos de hover/confirm disponibles.
- Integración con `AudioManager` para reproducir hover y confirm SFX desde la UI.
