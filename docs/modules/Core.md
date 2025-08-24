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
Implementado y en uso. Carga de config con defaults, save/load JSON funcional y ejemplos en `config/` y `saves/`.

## 3. Referencias
- docs/archive/core-status.md

### Notas de cambios (2025-08-24)
- Logger: mejoras para salida Unicode en consola (SetConsoleOutputCP + WriteConsoleW fallback) para corregir problemas con tildes/acentos en logs.
- AssetManager: añadidas APIs para invalidar y recargar texturas en tiempo de ejecución (`hasTexture`, `removeTexture`, `reloadTexturesFrom`).
- **ConfigManager Enhancement**: Estructura expandida para soporte de configuraciones avanzadas:
  - Volúmenes separados (master, music, ui) en lugar de volumen único
  - Soporte para configuración de pantalla (fullscreen, vsync)
  - Nuevos métodos setter para todas las opciones
  - Compatibilidad con formato anterior de config.json
  - Guardado automático con estructura JSON mejorada
- Game: responsabilidad de dibujar el fondo movida a las escenas; `Game` ya no dibuja un background global para evitar mostrar texturas eliminadas del disco.
