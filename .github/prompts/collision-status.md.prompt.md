---
mode: agent
---

## 🎯 Contexto
Estamos trabajando en el módulo `collisions` de un motor de juego estilo retro low-poly.  
Actualmente soporta AABB básicos, segment checks para raycasting simple, capas de colisión y resolución de bloqueos con MTV.  

El objetivo es **mejorar y escalar el sistema de colisiones** para que sea más robusto, eficiente y flexible, integrándose con otros módulos (`entities`, `scene`, `AI`, `UI`).

---

## ✅ Mejoras principales (15)
1. Implementar un sistema de **broad-phase spatial partitioning** (ej: QuadTree o Spatial Hash).  
2. Extender la API de **CollisionBox** para soportar colliders dinámicos (resize en runtime).  
3. Agregar un **sistema de eventos de colisión** (`onEnter`, `onExit`).  
4. Implementar **unit tests** para `CollisionManager` y `CollisionSystem`.  
5. Documentar con ejemplos de uso (`Player`, `Enemy`, `Wall`).  
6. Agregar un modo de **debug visual** (dibujar colliders).  
7. Optimizar `resolve()` para soportar múltiples entidades simultáneas.  
8. Integrar con `EntityManager` para invalidar colliders al destruir entidades.  
9. Implementar soporte de **filtros por capas múltiples** (máscara avanzada).  
10. Añadir logging estructurado para colisiones.  
11. Soporte de **multi-shapes por entidad** (ej: torso + piernas).  
12. Métodos para **desplazamiento predictivo** (sweep tests).  
13. Extender `segmentIntersectsAny` con resultado de punto de impacto + normal.  
14. Añadir **profiling hooks** para medir tiempo de detección.  
15. Preparar hooks para físicas (rebote, fricción simple).

---

## 🔧 Tareas extras (10)
1. **Soporte para colisión circular y poligonal** (con `CollisionShape` base).  
2. **Raycasting avanzado** con hit-info (punto, normal, distancia).  
3. Implementar **evento `onStay`** para entidades en contacto prolongado.  
4. Soporte para **capas dinámicas en runtime**.  
5. **Persistencia/serialización** de colliders (ej: `map.json`).  
6. Implementar **zonas fantasma / sensores** (no sólidos).  
7. Añadir **coeficientes de restitución** para rebotes simples.  
8. Sistema de **capas jerárquicas** para filtrado avanzado.  
9. Preparar **sincronización de colisiones en red** (determinísticas).  
10. **Stress tests y benchmark** con >1000 colliders.  

---

## 📌 Integración esperada
- `entities`: colliders registrados automáticamente al crear entidades.  
- `scene`: controla validación de movimiento con `firstColliderForBounds`.  
- `AI`: usar raycasts para línea de visión (`Enemy::detectPlayer`).  
- `UI/Debug`: visualización de colliders.  
- `physics`: futuros rebotes/fricción.  

---

## 🎯 Criterios de éxito
- El sistema debe ser **robusto, extensible y probado con unit tests**.  
- Manejar correctamente **bloqueos, triggers y sensores**.  
- Ofrecer **rendimiento estable** en escenarios con cientos de entidades.  
- Integrarse con el flujo actual de `Player`, `Enemy` y `Scene`.  

Crea al final de todo un documento en la carpeta docs con los cambios hechos, llamalo `collision-improvement-implementation.md`.

---
