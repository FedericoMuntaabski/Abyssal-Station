---
title: "UI"
module: "ui"
last_updated: "2025-08-25"
authors: ["Equipo"]
tags: [status, implementation, planned-improvements]
---

# UI

## 1. Resumen
Subsistema de interfaz: `UIManager`, `MainMenu`, `OptionsMenu`, `InputHelper`, y sistema de `Toast` para notificaciones. **MEJORAS PLANIFICADAS**: HUD de estados vitales, inventario visual, indicadores de sigilo, y retroalimentación contextual para sistemas de supervivencia.

## 2. Estado actual
Funcional; permite remapeo en runtime y persistencia de opciones. Falta iconografía y soporte de combinaciones avanzadas.

## 3. Mejoras Planificadas - UI de Supervivencia Avanzada

### 3.1 HUD de Estados Vitales
**Objetivo**: Interfaz clara y accesible para monitorear recursos del jugador en tiempo real.

**Elementos a implementar**:
- **Barra de Vida**: Roja, con indicador de heridas críticas (parpadea bajo 35%)
- **Barra de Batería**: Azul, con indicador de consumo de linterna
- **Barra de Fatiga**: Amarilla, muestra drenaje por acciones físicas
- **Indicador de Peso**: Cambia color según capacidad (Verde: <50%, Amarillo: 50-80%, Rojo: >80%)
- **Estado de Sigilo**: Ícono de ojo que indica nivel de visibilidad

**Características**:
- Posicionamiento no intrusivo (esquinas de pantalla)
- Animaciones suaves para cambios de estado
- Alertas visuales para estados críticos
- Opciones de transparencia y tamaño configurables

### 3.2 Inventario Visual Interactivo
**Descripción**: Interfaz de gestión de objetos con arrastrar y soltar.

**Características**:
- **Hotbar de 4 slots**: Acceso rápido a items principales
- **Vista de mochila**: Grid expandible para storage adicional
- **Información de items**: Tooltips con peso, durabilidad, descripción
- **Indicadores de peso**: Código de colores por capacidad
- **Gestión automática**: Sugerencias de organización óptima

**Controles**:
- Click derecho para usar items
- Arrastrar y soltar para reorganizar
- Teclas numéricas (1-4) para acceso rápido
- Tab para abrir/cerrar mochila completa

### 3.3 Indicadores de Sigilo y Detección
**Descripción**: Feedback visual para mecánicas de ocultación.

**Elementos**:
- **Medidor de Ruido**: Círculo que crece con el nivel de ruido generado
- **Estado de Sigilo**: Ícono que cambia según postura (parado, agachado, oculto)
- **Alerta de Detección**: Indicador cuando enemigos están cerca de detectar
- **Campo de Visión**: Overlay opcional mostrando cono de visión del jugador

### 3.4 Sistema de Notificaciones Contextual
**Descripción**: Alertas específicas para eventos de supervivencia.

**Tipos de notificaciones**:
- **Estado crítico**: "¡Vida baja! Busca refugio"
- **Batería agotándose**: "Linterna se está agotando"
- **Sobrecarga de peso**: "Demasiado peso, movimento reducido"
- **Detección enemiga**: "¡Has sido detectado!"
- **Item recogido**: Con información de peso y espacio

**Características**:
- Colores diferenciados por urgencia
- Duración variable según importancia
- Posicionamiento inteligente para no obstruir gameplay

### 3.5 Menú de Configuración Expandido
**Descripción**: Opciones específicas para sistemas de supervivencia.

**Nuevas opciones**:
- **Opacidad del HUD**: Ajustar transparencia de elementos vitales
- **Tamaño de UI**: Escalado para diferentes resoluciones
- **Assist de Colorblind**: Patrones alternativos para daltonismo
- **Indicadores de Audio**: Subtítulos para ruidos del entorno
- **Autopausa**: Pausar juego al abrir inventario

## 4. Implementación (Estado Actual)
- `UIManager`: gestión de widgets, toasts y animaciones; `FontHelper` para caché de fuentes.
- `OptionsMenu`: remapeo integrándose con `InputManager` y guardado inmediato de bindings.
- `InputHelper`: formatea bindings y detecta dispositivo activo.

## 5. Archivos clave
- **Existentes**: `src/ui/UIManager.*`, `MainMenu.*`, `OptionsMenu.*`, `InputHelper.*`
- **Planificados**: `VitalStatsHUD.*`, `InventoryUI.*`, `StealthIndicators.*`, `SurvivalNotifications.*`

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

### Notas de cambios (2025-08-25)
- **PLANIFICACIÓN**: Definida interfaz comprehensiva para sistemas de supervivencia
- **ARQUITECTURA**: Diseñado HUD de estados vitales, inventario visual e indicadores de sigilo
- **UX/UI**: Especificadas mejoras de accesibilidad y feedback contextual para supervivencia
