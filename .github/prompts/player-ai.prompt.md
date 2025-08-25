---
mode: agent
title: "Mejoras Avanzadas del Sistema de Jugador y Enemigos - Abyssal Station"
module: "player-enemies"
priority: "high"
deadline: "Q1 2025"
---

# Mejoras Avanzadas del Sistema de Jugador y Enemigos - Abyssal Station

## OBJETIVO PRINCIPAL
Implementar mejoras sustanciales al sistema de supervivencia/horror de Abyssal Station, enfocándose en características realistas para el jugador (Player) y comportamientos avanzados para enemigos (Enemy), creando una experiencia inmersiva y desafiante.

## TAREAS ESPECÍFICAS A REALIZAR

### 🎮 MEJORAS DEL SISTEMA DE JUGADOR (Player)

#### 1. Sistema de Estados Vitales
**Descripción**: Implementar múltiples recursos vitales interconectados que afecten el gameplay
- **Vida (Health)**: 0-100, muerte permanente al llegar a 0
- **Batería (Battery)**: 0-100, recarga lenta automática (1 punto/3 segundos)
- **Fatiga (Fatigue)**: 0-100, aumenta con acciones físicas
- **Peso (Weight)**: Calculado dinámicamente según inventario

**Archivos a modificar**:
- `src/entities/Player.h/.cpp` - Agregar variables de estado
- `src/entities/PlayerController.h/.cpp` - Lógica de control de estados
- `src/ui/UIManager.h/.cpp` - HUD de estados vitales

#### 2. Sistema de Estamina Limitada
**Descripción**: Gestión de energía que limita acciones del jugador
- Correr drena fatiga (5 puntos/segundo)
- Atacar drena fatiga (15 puntos por ataque)
- Peso afecta velocidad de drenaje (+1 punto/segundo por cada 10kg extra)
- Recuperación: 2 puntos/segundo en reposo

#### 3. Inventario Restringido por Peso y Slots
**Descripción**: Sistema de inventario limitado y realista
- **4 slots principales** de acceso rápido (hotbar)
- **Capacidad de peso**: 20kg máximo
- **Gestión de slots**: Intercambio automático entre slots principales y mochila
- **Peso por item**: Varía según tipo de objeto

**Nuevo archivo**: `src/gameplay/InventorySystem.h/.cpp`

#### 4. Sistema de Visión y Linterna
**Descripción**: Campo de visión limitado con consumo de batería
- **Visión en cono**: 60° hacia adelante, alcance 150 unidades
- **Sin visión trasera**: El jugador no puede ver detrás
- **Linterna consume batería**: 2 puntos/minuto cuando está encendida
- **Oscuridad dinámica**: Renderizado basado en fuentes de luz

**Archivos a modificar**:
- `src/scene/PlayScene.h/.cpp` - Sistema de renderizado por capas
- Nuevo: `src/gameplay/VisionSystem.h/.cpp`

#### 5. Sistema de Sigilo Avanzado
**Descripción**: Mecánicas de ocultación para evitar detección
- **Agacharse**: Reduce velocidad 50%, reduce ruido 70%
- **Caminar lento**: Reduce ruido 50%
- **Apagar luces**: Evita detección visual de enemigos
- **Cobertura**: Usar objetos del entorno para ocultarse

#### 6. Sistema de Ruido Dinámico
**Descripción**: Cada acción genera niveles específicos de ruido
- **Caminar normal**: 3 unidades de ruido
- **Correr**: 8 unidades de ruido
- **Abrir puertas**: 5 unidades de ruido
- **Combate melee**: 12 unidades de ruido
- **Disparar** (futuro): 15 unidades de ruido

**Nuevo archivo**: `src/gameplay/NoiseSystem.h/.cpp`

#### 7. Sistema de Combate Improvisado
**Descripción**: Arsenal limitado y realista para supervivencia
- **Armas improvisadas**: Tubería (+15 daño), Cuchillo (+25 daño)
- **Durabilidad**: Las armas se degradan con el uso
- **Munición escasa**: Máximo 3 proyectiles por tipo
- **Cooldown de ataque**: 1.5 segundos entre ataques melee

#### 8. Sistema de Heridas Persistentes
**Descripción**: Consecuencias duraderas por daño severo
- **Umbral crítico**: Si la vida baja del 35%, se activan penalizaciones
- **Cojera**: Velocidad reducida 25% permanentemente
- **Pérdida de slots**: Solo 2 slots principales disponibles
- **Regeneración lenta**: 1 punto de vida cada 30 segundos hasta 50% máximo

### 🤖 MEJORAS DEL SISTEMA DE ENEMIGOS (Enemy)

#### 1. IA Reactiva Mejorada
**Descripción**: Comportamientos inteligentes basados en estímulos del entorno
- **Patrullaje dinámico**: Rutas variables, no predecibles
- **Investigación de ruidos**: Se dirigen al último punto de ruido conocido
- **Persecución inteligente**: Predicen movimiento del jugador
- **Memoria de ubicaciones**: Recuerdan donde vieron al jugador por última vez

**Archivos a modificar**:
- `src/ai/AIState.h/.cpp` - Nuevos estados INVESTIGATE_NOISE, PREDICT_MOVEMENT
- `src/ai/Perception.h/.cpp` - Mejoras en detección sensorial

#### 2. Ataques Físicos y Psicológicos
**Descripción**: Daño diversificado que afecta múltiples aspectos del jugador
- **Ataque físico**: 20-35 de daño directo
- **Grito intimidante**: +25 fatiga, efecto de pantalla temblorosa
- **Presencia amenazante**: Drenaje pasivo de 1 batería/segundo en proximidad
- **Ataques de emboscada**: Doble daño si atacan desde la oscuridad

#### 3. Detección Sensorial Múltiple
**Descripción**: Sistema sensorial complejo y realista
- **Visión limitada**: Cono de 45°, alcance 120 unidades
- **Audición sensible**: Detecta ruido en radio de 200 unidades
- **Detección de vibraciones**: Percibe movimiento en suelo en 50 unidades
- **Memoria sensorial**: Mantienen alerta 30 segundos después de perder contacto

#### 4. Sistema de Persistencia Inteligente
**Descripción**: Enemigos que no se rinden fácilmente
- **Búsqueda activa**: Continúan buscando 5 minutos después de perder rastro
- **Puntos de interés**: Revisan lugares donde encontraron al jugador antes
- **Comunicación entre enemigos**: Comparten información de ubicación
- **Escalada de alerta**: Más enemigos se activan si uno encuentra al jugador

#### 5. Modo Acecho Psicológico
**Descripción**: Tensión psicológica mediante seguimiento silencioso
- **Seguimiento a distancia**: Mantienen 100 unidades de distancia
- **Apariciones breves**: Se muestran brevemente en la periferia de la visión
- **Sonidos ambientales**: Generan ruidos sutiles para crear tensión
- **Ataque sorpresa**: Después de 60 segundos de acecho, atacan con ventaja

#### 6. Sistema de Recompensa de Riesgo
**Descripción**: Incentivos equilibrados para enfrentar o evitar enemigos
- **Derrotar enemigos**: +1 batería extra permanente (máximo +5)
- **Objetos valiosos**: Enemigos portan llaves o recursos únicos
- **Zonas de alto riesgo**: Más enemigos = mejores recompensas
- **Penalización por huida**: Enemigos se vuelven más agresivos si huyes repetidamente

## REQUERIMIENTOS ESPECÍFICOS

### Técnicos
1. **Compatibilidad**: Mantener compatibilidad con SFML 3.0.1 y sistemas existentes
2. **Performance**: Máximo 5ms adicionales por frame en sistemas de IA y visión
3. **Memoria**: Incremento máximo de 50MB en uso de RAM
4. **Modularidad**: Cada sistema debe ser independiente y configurable
5. **Serialización**: Todos los estados deben ser guardables/cargables

### Gameplay
1. **Balance**: El juego debe ser desafiante pero no frustrante
2. **Progresión**: Curva de dificultad escalable
3. **Feedback**: Retroalimentación clara de todos los sistemas al jugador
4. **Configurabilidad**: Parámetros ajustables en `config/config.json`

### Interfaz
1. **HUD Informativo**: Barras de estados vitales claramente visibles
2. **Iconografía**: Íconos intuitivos para estados y acciones
3. **Notificaciones**: Alertas contextuales para eventos importantes
4. **Accesibilidad**: Soporte para daltónicos y opciones de contraste

## LIMITACIONES Y RESTRICCIONES

### Restricciones Técnicas
- **No modificar** la arquitectura base de Entity/Component
- **Preservar** compatibilidad con save/load existente
- **Mantener** performance de 60 FPS en hardware objetivo
- **Usar** únicamente dependencias ya integradas (SFML, nlohmann-json)

### Restricciones de Diseño
- **Realismo**: Todas las mecánicas deben ser creíbles en el contexto
- **Cohesión**: Los sistemas deben sentirse integrados, no como parches
- **Simplicidad**: Evitar micro-gestión excesiva que frustre al jugador

### Restricciones de Implementación
- **Tiempo**: Desarrollo incremental, sistema por sistema
- **Testing**: Cada feature debe incluir tests unitarios
- **Documentación**: Actualizar docs correspondientes después de cada implementación

## CRITERIOS DE ÉXITO

### Criterios Funcionales
1. ✅ **Estados vitales operativos**: Vida, batería, fatiga y peso afectan gameplay
2. ✅ **Sistema de inventario funcional**: 4 slots principales, gestión de peso
3. ✅ **Visión limitada implementada**: Cono de visión, consumo de batería
4. ✅ **Sigilo operativo**: Agacharse, caminar lento, reducción de ruido
5. ✅ **Combate melee funcional**: Armas improvisadas con durabilidad
6. ✅ **IA enemiga mejorada**: Detección múltiple, persistencia, acecho
7. ✅ **Sistema de ruido completo**: Generación y detección de sonidos
8. ✅ **Heridas persistentes**: Penalizaciones por daño severo

### Criterios de Calidad
1. ✅ **Performance estable**: 60 FPS mantenidos durante gameplay intenso
2. ✅ **Sin memory leaks**: Gestión correcta de memoria en todos los sistemas
3. ✅ **Save/Load completo**: Todos los estados se preservan correctamente
4. ✅ **UI responsiva**: Feedback visual inmediato para todas las acciones

### Criterios de Experiencia
1. ✅ **Tensión balanceada**: El juego mantiene tensión sin frustración
2. ✅ **Decisiones significativas**: Las elecciones del jugador tienen consecuencias claras
3. ✅ **Curva de aprendizaje**: Los sistemas se introducen progresivamente
4. ✅ **Rejugabilidad**: Variabilidad en encounters y estrategias

## PLAN DE IMPLEMENTACIÓN

### Fase 1: Estados Vitales y Inventario (Sprint 1-2)
- Implementar sistema de estados vitales básico
- Crear sistema de inventario con peso y slots
- Integrar HUD para estados vitales
- Tests unitarios para gestión de estados

### Fase 2: Visión y Sigilo (Sprint 3-4)
- Sistema de visión en cono con linterna
- Mecánicas de sigilo y agacharse
- Sistema de ruido básico
- Integración visual de efectos de iluminación

### Fase 3: Combate y Heridas (Sprint 5-6)
- Sistema de combate melee improvisado
- Durabilidad de armas
- Sistema de heridas persistentes
- Balance y ajuste de valores

### Fase 4: IA Avanzada de Enemigos (Sprint 7-8)
- Mejoras en detección sensorial múltiple
- Sistema de persistencia y memoria
- Modo acecho psicológico
- Comunicación entre enemigos

### Fase 5: Polish y Optimización (Sprint 9-10)
- Optimización de performance
- Balanceo final de gameplay
- Documentación completa
- Testing exhaustivo

## DOCUMENTACIÓN REQUERIDA

Al completar este desarrollo, actualizar los siguientes archivos:

### Módulos de Documentación
- `docs/modules/Entities.md` - Actualizar con nuevos sistemas de Player
- `docs/modules/AI.md` - Documentar mejoras en sistema de enemigos
- `docs/modules/Gameplay.md` - Nuevos sistemas de inventario, combate, y sigilo
- `docs/modules/UI.md` - HUD y interfaces de estados vitales

### Documentación Técnica
- `docs/PROJECT_SUMMARY.md` - Resumen de nuevas características implementadas
- `docs/RECENT_IMPROVEMENTS_SUMMARY.md` - Changelog detallado de mejoras
- `README.md` del proyecto - Actualizar características principales

### Archivos de Configuración
- `config/config.json` - Nuevos parámetros configurables
- `config/balance.json` (nuevo) - Valores de balanceo de gameplay

## UX/UI - ESPECIFICACIONES DETALLADAS DEL HUD

### 📍 Principios de Diseño del HUD
- **Bajo ruido visual**: Que no tape la tensión del entorno
- **Lectura rápida**: Barras, íconos y colores fáciles de interpretar
- **Estética temática**: Oscura, industrial, con tonos azulados/verdosos (Barotrauma) y contraste rojo/ámbar para alertas
- **Feedback inmediato**: Cambios visuales claros cuando algo va mal (sangrado, fatiga, linterna baja)

### 🖥️ Layout General del HUD
**HUD no intrusivo** → distribuido en bordes de pantalla:
- **Zona inferior izquierda** → Estados vitales (Health, Fatigue, Battery)
- **Zona inferior derecha** → Hotbar (4 slots + peso)
- **Zona superior derecha** → Notificaciones contextuales (ruido generado, alerta enemigo, heridas)
- **Zona central baja** → Estamina en barra dinámica visible solo cuando corre/ataca

### 🔹 Elementos Específicos del HUD

#### 1. Estados Vitales (parte inferior izquierda)
**Vida (Health)**:
- Barra horizontal roja con degradado oscuro
- Parpadea al <25%
- Borde rojizo pulsante en pantalla si <35%

**Batería (Battery)**:
- Barra azul con icono de pila
- Descarga lenta visual progresiva
- Vibración de barra al <10%
- Icono se "rompe" visualmente con efecto glitchy al 0%

**Fatiga (Fatigue)**:
- Barra amarilla/naranja
- Aumenta con acciones → cuando está al 100% el jugador no puede correr
- Icono de "pulmones" o "cansancio" en ámbar al máximo

**Peso (Weight)**:
- Texto "XX / 20kg" junto a icono de mochila
- Cambia a rojo si supera 90%
- Afecta velocidad de movimiento visualmente

#### 2. Estamina Dinámica (centro-bajo)
- Barra blanca fina que aparece **solo** al correr, atacar o agacharse
- Se drena rápido y desaparece al recuperarse
- Vibración y color rojo cuando está al 10%
- Integrada con sistema de fatiga

#### 3. Inventario Rápido - Hotbar (parte inferior derecha)
- **4 slots cuadrados** con iconos de objetos
- Indicador visual de peso total debajo
- Ítem activo resaltado con borde luminoso tenue
- **Slots deshabilitados** (por heridas críticas) aparecen con candado o "X" grisáceo
- Teclas numéricas (1-4) para acceso rápido

#### 4. Sistema de Visión/Linterna
- Icono de linterna en HUD con estado visual
- Brillo del icono baja a medida que la batería se consume
- Al 0%, icono se rompe visualmente con crack glitchy
- Integración con campo de visión en cono

#### 5. Indicadores de Ruido y Sigilo
**Ruido**:
- Icono de onda sonora aparece temporalmente cuando el jugador genera sonido
- Intensidad visual (tamaño/brillo) escala con el nivel de ruido
- Diferentes iconos para diferentes acciones (pasos, puertas, combate)

**Sigilo**:
- Icono de ojo semiabierto (detectabilidad)
- Se cierra cuando está agachado o en sombra
- Cambia color según nivel de ocultación

#### 6. Indicadores de Heridas/Estados Críticos
**Vida crítica (<35%)**:
- Ícono de "gota de sangre" aparece en HUD
- Pantalla con borde rojizo pulsante
- Texto contextual: "Herida crítica – movilidad reducida"

**Fatiga máxima (100%)**:
- Icono de "pulmones" o "cansancio" en ámbar
- Imposibilidad de correr visualmente indicada

**Cojera permanente**:
- Texto contextual: "Movimiento reducido"
- Icono de pierna con "X" o grieta

#### 7. Notificaciones Contextuales (parte superior derecha)
**Mensajes cortos tipo log** con fade-in/out en 3 segundos:
- "Enemigo detectó ruido"
- "Batería baja"
- "Herida crítica – movilidad reducida"
- "Linterna agotada"
- "Sobrecarga de peso"
- "Enemigo cercano"

**Características técnicas**:
- Transparencias oscuras detrás de elementos para legibilidad
- Colores diferenciados por urgencia (verde/azul: info, amarillo: advertencia, rojo: crítico)
- Posicionamiento inteligente para no obstruir gameplay
- Animaciones suaves y no intrusivas

### 🎨 Paleta de Colores Temática
- **Base**: Tonos azulados/verdosos oscuros (estilo Barotrauma)
- **Alertas**: Rojo/ámbar para estados críticos
- **Información**: Azul para batería, amarillo para fatiga
- **Éxito**: Verde tenue para acciones completadas
- **Backgrounds**: Negro translúcido para legibilidad

Este prompt define una roadmap comprensiva para transformar Abyssal Station en una experiencia de supervivencia/horror más inmersiva y desafiante, con sistemas interconectados que crean emergencia narrativa y decisiones significativas para el jugador.