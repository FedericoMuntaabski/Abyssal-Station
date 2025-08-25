---
title: "Core"
module: "core"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Core

## 1. Resumen
Subsystems centrales: ConfigManager, SaveManager, GameState, logging y el bucle principal. Provee APIs de persistencia y configuración global.

## 2. Estado actual
Implementado y en uso con mejoras criticas recientes. Carga de config con defaults, save/load JSON funcional, auto-save implementado y ejemplos en `config/` y `saves/`.

## 3. Referencias
- docs/archive/core-status.md

### Notas de cambios (2025-08-24)

#### **✅ Sistema Auto-Save Implementado (Fase 2)**

**SaveManager Auto-Save Enhancement**
- Implementado sistema de auto-save con intervalos de 2 minutos
- **Nuevos metodos**:
  - `update()` - Ejecuta logica de auto-save en cada frame
  - `shouldAutoSave()` - Verifica si es tiempo de auto-save basado en timer
  - `performAutoSave()` - Ejecuta auto-save usando GameState actual
- **Integracion**: PlayScene llama SaveManager::update() en su metodo update()
- **Configuracion**: Intervalo de 2 minutos (120 segundos) por defecto
- **Estado**: ✅ Completado - Auto-save funcional en PlayScene

#### **📋 Mejoras Existentes Mantenidas**
- Logger: mejoras para salida Unicode en consola (SetConsoleOutputCP + WriteConsoleW fallback) para corregir problemas con tildes/acentos en logs.
- AssetManager: añadidas APIs para invalidar y recargar texturas en tiempo de ejecución (`hasTexture`, `removeTexture`, `reloadTexturesFrom`).
- **ConfigManager Enhancement**: Estructura expandida para soporte de configuraciones avanzadas:
  - Volúmenes separados (master, music, ui) en lugar de volumen único
  - Soporte para configuración de pantalla (fullscreen, vsync)
  - Nuevos métodos setter para todas las opciones
  - Compatibilidad con formato anterior de config.json
  - Guardado automático con estructura JSON mejorada
- Game: responsabilidad de dibujar el fondo movida a las escenas; `Game` ya no dibuja un background global para evitar mostrar texturas eliminadas del disco.

### Cambios recientes (2025-08-24) — delta

- Se añadió un `AudioManager` en `src/audio/` con soporte para tracks/music, SFX, crossfade, fades y control de volúmenes por tipo (master, music, sound). Integrado en el ciclo de escenas y probado en `MainMenu` y `PlayScene`.
- `Game` ahora posee una referencia a `AudioManager` y delega reproducción contextual a las escenas vía `audio::AudioManager::setAudioContext`.
- Se reforzó la API de `ConfigManager` para exponer volúmenes separados y persistencia de preferencias de audio.
