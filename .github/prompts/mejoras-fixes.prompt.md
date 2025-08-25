---
mode: agent
---

# Mejoras y Fixes para Abyssal Station - Proyecto de Juego 2D Supervivencia/Horror

## Objetivo del Proyecto
Implementar mejoras criticas, fixes de bugs y optimizaciones para el juego 2D de supervivencia/horror "Abyssal Station" desarrollado en C++ con SFML, enfocandose en estabilidad, rendimiento y experiencia de usuario.

## Tareas Criticas a Realizar

### 1. FIXES CRITICOS DE MENU Y NAVEGACION

#### 1.1 Fix del Menu de Opciones
- **Problema**: El menu de opciones crashea el juego al presionar el boton "Opciones" desde el menu principal
- **Solucion**: Investigar y corregir el crash en OptionsMenu.cpp, verificando inicializacion de ConfigManager
- **Archivos**: `src/ui/OptionsMenu.cpp`, `src/ui/MainMenu.cpp`
- **Test**: Navegar al menu opciones sin crashes, modificar settings y guardar correctamente

#### 1.2 Fix de Transicion "Jugar (Solo)"
- **Problema**: El boton "Jugar (Solo)" no envia correctamente a LoadingScene y luego a PlayScene
- **Solucion**: Verificar la cadena de transiciones MenuScene → LoadingScene → PlayScene
- **Archivos**: `src/ui/MainMenu.cpp`, `src/scene/MenuScene.cpp`, `src/scene/LoadingScene.cpp`
- **Test**: Al presionar "Jugar (Solo)" debe mostrar pantalla de carga y luego escena de juego

#### 1.3 Eliminacion de Tildes y Caracteres Especiales
- **Problema**: Problemas de encoding UTF-8 con caracteres acentuados en textos
- **Solucion**: Reemplazar todos los textos con tildes por equivalentes sin acentos
- **Cambios**:
  - "Opciones" → "Opciones" (mantener)
  - "Resolución" → "Resolucion"
  - "Configuración" → "Configuracion"
  - "Música" → "Musica"
  - "Volumen Música" → "Volumen Musica"
- **Archivos**: Todos los archivos con strings de UI
- **Test**: Verificar que todos los textos se rendericen correctamente sin simbolos raros

### 2. MEJORAS DE RENDIMIENTO Y ESTABILIDAD

#### 2.1 Optimizacion del Sistema de Colisiones
- **Problema**: QuadTree queries devuelven 0 resultados por punteros dangling
- **Solucion**: Implementar mejor gestion de memoria en SpatialPartition
- **Archivos**: `src/collisions/SpatialPartition.cpp`, `src/collisions/CollisionManager.cpp`
- **Test**: Tests de colision complejos deben pasar correctamente

#### 2.2 Mejora del Sistema de AI
- **Problema**: Metricas de performance siempre muestran 0 agentes
- **Solucion**: Actualizar metricas inmediatamente al agregar/remover agentes
- **Archivos**: `src/ai/AIManager.cpp`, `src/ai/Perception.cpp`
- **Test**: Metricas deben reflejar numero correcto de agentes en tiempo real

#### 2.3 Optimizacion de Carga de Assets
- **Problema**: Carga de assets puede ser lenta en inicio
- **Solucion**: Implementar sistema de precarga asíncrona en LoadingScene
- **Archivos**: `src/core/AssetManager.cpp`, `src/scene/LoadingScene.cpp`
- **Test**: Tiempo de carga debe reducirse en al menos 30%

### 3. MEJORAS DE EXPERIENCIA DE USUARIO

#### 3.1 Sistema de Notificaciones Mejorado
- **Problema**: Notificaciones basicas sin contexto visual
- **Solucion**: Añadir sistema de toasts con iconos y colores segun tipo
- **Archivos**: `src/ui/UIManager.cpp`
- **Tipos**: Error (rojo), Advertencia (amarillo), Exito (verde), Info (azul)
- **Test**: Mostrar notificaciones en diferentes situaciones del juego

#### 3.2 Menu de Pausa Mejorado
- **Problema**: Menu de pausa funcional pero limitado
- **Solucion**: Añadir opciones de configuracion rapida (volumen, brillo)
- **Archivos**: `src/ui/PauseMenu.cpp`
- **Test**: Acceso rapido a settings desde pausa sin salir del juego

#### 3.3 Sistema de Guardado Automatico
- **Problema**: Solo guardado manual disponible
- **Solucion**: Implementar auto-save cada 2 minutos en puntos seguros
- **Archivos**: `src/core/SaveManager.cpp`, `src/scene/PlayScene.cpp`
- **Test**: Verificar que el auto-save funcione sin interrumpir gameplay

### 4. NUEVAS FUNCIONALIDADES

#### 4.1 Sistema de Achievement/Logros
- **Solucion**: Crear sistema basico de logros para engagement
- **Archivos**: Nuevos: `src/gameplay/AchievementManager.cpp/.h`
- **Logros basicos**:
  - "Primer Paso": Mover al jugador por primera vez
  - "Explorador": Descubrir 5 areas diferentes
  - "Superviviente": Sobrevivir 5 minutos
  - "Coleccionista": Recoger 10 items
- **Test**: Logros se desbloquean correctamente y persisten

#### 4.2 Sistema de Configuracion de Controles
- **Problema**: Controles fijos sin customizacion
- **Solucion**: Menu de reasignacion de teclas completo
- **Archivos**: `src/ui/OptionsMenu.cpp`, `src/input/InputManager.cpp`
- **Test**: Poder reasignar cualquier accion a diferentes teclas

#### 4.3 Sistema de Audio Dinamico
- **Problema**: Audio estatico sin variacion contextual
- **Solucion**: Implementar sistema de audio adaptativo segun situacion
- **Archivos**: Nuevo: `src/audio/AudioManager.cpp/.h`
- **Features**:
  - Musica de tension cuando enemigos cerca
  - Efectos de sonido posicionales
  - Fade in/out suave entre tracks
- **Test**: Audio cambia apropiadamente segun contexto de juego

### 5. MEJORAS DE DEBUGGING Y DESARROLLO

#### 5.1 Sistema de Debug Console
- **Solucion**: Consola de comandos para testing en runtime
- **Archivos**: Nuevo: `src/debug/DebugConsole.cpp/.h`
- **Comandos basicos**:
  - `spawn_enemy <x> <y>`: Generar enemigo en posicion
  - `god_mode`: Activar/desactivar invencibilidad
  - `teleport <x> <y>`: Teletransportar jugador
  - `set_health <value>`: Cambiar vida del jugador
- **Test**: Comandos funcionan correctamente en build debug

#### 5.2 Visualizacion de Metricas en Pantalla
- **Problema**: Metricas solo en logs, no visibles en juego
- **Solucion**: Overlay de debug con FPS, memoria, AI stats
- **Archivos**: `src/debug/DebugOverlay.cpp/.h`
- **Test**: Toggle on/off con F3, metricas actualizadas en tiempo real

#### 5.3 Sistema de Profiling Integrado
- **Solucion**: Herramientas de profiling para identificar bottlenecks
- **Archivos**: Nuevo: `src/debug/Profiler.cpp/.h`
- **Metricas**: Frame time, render time, update time, memoria usada
- **Test**: Exportar reports de performance a archivos JSON

## Requerimientos Especificos

### Tecnicos
- **Compatibilidad**: Mantener compatibilidad con SFML 3.0.1
- **Rendimiento**: No degradar FPS actual (minimo 60 FPS)
- **Memoria**: Uso de memoria no debe exceder 512MB
- **Builds**: Funcionar en Debug y Release builds
- **Tests**: Mantener al menos 85% de tests pasando

### Codigo
- **Estilo**: Seguir convenciones existentes del proyecto
- **Documentacion**: Comentarios en ingles, documentacion en español
- **Error Handling**: Manejo robusto de errores con logging
- **RAII**: Uso apropiado de smart pointers y RAII patterns
- **Threading**: Si se usa threading, debe ser thread-safe

### UI/UX
- **Consistencia**: Mantener look and feel actual
- **Accesibilidad**: Navegacion por teclado en todos los menus
- **Responsividad**: UI debe funcionar en todas las resoluciones soportadas
- **Feedback**: Feedback visual/audio para todas las acciones del usuario

## Limitaciones y Restricciones

### Tecnicas
- **NO** cambiar la arquitectura base del engine
- **NO** añadir dependencias externas nuevas sin justificacion
- **NO** romper la API publica existente
- **NO** modificar el sistema de build (CMake) sin necesidad
- **NO** introducir breaking changes en saves existentes

### Funcionales
- **NO** implementar networking (queda para futura version)
- **NO** cambiar el genero/tema del juego
- **NO** modificar assets visuales sin aprobacion
- **NO** implementar features que requieran mas de 40 horas de desarrollo

### Rendimiento
- **NO** implementar features que degraden FPS por debajo de 45
- **NO** usar mas de 128MB adicionales de RAM
- **NO** introducir pauses/stutters perceptibles durante gameplay

## Criterios de Exito

### Funcionalidad (40%)
- [ ] Menu de opciones abre sin crashes
- [ ] "Jugar (Solo)" transiciona correctamente a LoadingScene → PlayScene
- [ ] Todos los textos sin tildes renderizan correctamente
- [ ] Sistema de guardado automatico funciona
- [ ] Controles customizables funcionan
- [ ] Audio dinamico responde a contexto

### Estabilidad (25%)
- [ ] 0 crashes durante 30 minutos de gameplay continuo
- [ ] Todos los tests criticos pasan (AI, Collision, Input, UI)
- [ ] Manejo robusto de errores en todos los sistemas nuevos
- [ ] No memory leaks detectados en herramientas de profiling

### Rendimiento (20%)
- [ ] Mantener 60+ FPS constantes en escenas complejas
- [ ] Tiempo de carga inicial < 5 segundos
- [ ] Uso de memoria estable (sin crecimiento infinito)
- [ ] Sistema de colisiones optimizado para 100+ entidades

### Experiencia de Usuario (15%)
- [ ] Navegacion fluida en todos los menus
- [ ] Feedback visual/audio apropiado en todas las interacciones
- [ ] Sistema de notificaciones informativo y no intrusivo
- [ ] Configuracion persiste correctamente entre sesiones

## Documentacion Requerida

Al completar cada modulo, actualizar la documentacion correspondiente:

### Core Module
- **Archivo**: `docs/modules/Core.md` y `docs/modules/core_module_changes.md`
- **Incluir**: Cambios en SaveManager, AssetManager, ConfigManager
- **Detallar**: Nuevos sistemas de auto-save y configuracion

### UI Module  
- **Archivo**: `docs/modules/UI.md` y `docs/modules/ui_module_changes.md`
- **Incluir**: Fixes de OptionsMenu, mejoras de PauseMenu, sistema de notificaciones
- **Detallar**: Eliminacion de tildes y mejoras de accesibilidad

### Scene Module
- **Archivo**: `docs/modules/Scene.md`
- **Incluir**: Fixes de transiciones MenuScene → LoadingScene → PlayScene
- **Detallar**: Optimizaciones de carga de assets

### AI Module
- **Archivo**: `docs/modules/AI.md` y `docs/modules/ai_module_changes.md`
- **Incluir**: Fixes de metricas de performance, mejoras de perception
- **Detallar**: Optimizaciones de AIManager

### Collision Module
- **Archivo**: `docs/modules/Collisions.md`
- **Incluir**: Fixes de SpatialPartition, optimizaciones de QuadTree
- **Detallar**: Mejoras en manejo de memoria

### Input Module
- **Archivo**: `docs/modules/Input.md` y `docs/modules/input_module_changes.md`
- **Incluir**: Sistema de reasignacion de controles
- **Detallar**: Mejoras de InputManager

### Gameplay Module
- **Archivo**: `docs/modules/Gameplay.md`
- **Incluir**: Sistema de achievements, nuevas mecanicas
- **Detallar**: Integracion con sistemas existentes

### Integration Documentation
- **Archivo**: `docs/integration/IMPLEMENTATION_COMPLETE.md`
- **Actualizar**: Status de todos los sistemas, nuevos criterios de exito
- **Incluir**: Resultados de testing y validation

## Proceso de Implementacion

1. **Fase 1 - Fixes Criticos** (Prioridad Alta)
   - Fix menu opciones
   - Fix transicion "Jugar (Solo)"
   - Eliminacion tildes

2. **Fase 2 - Optimizaciones** (Prioridad Media)
   - Sistema colisiones
   - Metricas AI
   - Carga assets

3. **Fase 3 - Nuevas Features** (Prioridad Media-Baja)
   - Auto-save
   - Notificaciones
   - Controles customizables

4. **Fase 4 - Features Avanzadas** (Prioridad Baja)
   - Achievement system
   - Audio dinamico
   - Debug tools

5. **Fase 5 - Documentacion y Testing** (Continuo)
   - Actualizar docs por modulo
   - Validar criterios de exito
   - Testing final integral

## Validacion Final

Antes de considerar completado:
- [ ] Ejecutar suite completa de tests (minimo 85% passing)
- [ ] Verificar todos los criterios de exito
- [ ] Actualizar toda la documentacion de modulos
- [ ] Realizar session de testing manual de 30 minutos
- [ ] Validar que no hay degradacion de rendimiento
- [ ] Confirmar que todos los fixes criticos funcionan

---

**Nota**: Este prompt debe ejecutarse por fases, documentando progreso en cada modulo correspondiente y consolidando cambios en lugar de crear nuevos archivos de documentacion.