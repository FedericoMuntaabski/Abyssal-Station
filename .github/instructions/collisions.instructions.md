---
applyTo: "src/collisions/**"
---

# Módulo: Collisions

## Objetivo
Implementar un sistema básico de colisiones 2D para detectar intersecciones entre entidades (jugador, paredes, objetos interactivos) y resolverlas de forma sencilla.

## Requisitos
- Lenguaje: C++17
- Librería gráfica: SFML (para `sf::FloatRect`)
- Depender de `Entities/Entity` y `EntityManager`
- Debe integrarse en `PlayScene`

## Componentes
1. **CollisionBox**  
   - Clase que representa un rectángulo de colisión (`sf::FloatRect`) asociado a una entidad.  

2. **CollisionManager**  
   - Contenedor que registra y gestiona todos los `CollisionBox`.  
   - Métodos principales:  
     - `addCollider(Entity*, sf::FloatRect)`  
     - `removeCollider(Entity*)`  
     - `checkCollisions(Entity*) -> std::vector<Entity*>`  

3. **CollisionSystem**  
   - Lógica de detección y resolución:  
     - Detección por AABB (Axis-Aligned Bounding Box).  
     - Resolución: reposicionar jugador fuera del obstáculo.  
     - Notificar colisión a través de `Logger`.  

4. **Integración con EntityManager**  
   - Cada entidad puede registrar su `CollisionBox`.  
   - `EntityManager` notifica al `CollisionManager` en `updateAll`.  

5. **Colisiones especiales**  
   - `Player vs Wall` → detener movimiento.  
   - `Player vs Item` → recoger objeto (placeholder de evento).  
   - `Player vs Enemy` → reducir vida.  

## Reglas de implementación
- Evitar cálculos redundantes (optimizar usando bounding boxes).  
- Mantener separación: detección (qué choca con qué) y resolución (qué hacer con la colisión).  
- Usar logs para debug de colisiones.  

## Ejemplo de flujo esperado
1. `Player` intenta moverse a la derecha.  
2. `CollisionManager` detecta colisión con pared.  
3. Se ajusta la posición del `Player` para que no atraviese la pared.  
4. Se registra el evento en `Logger`.  

## Instrucciones para Copilot
- Generar clases modulares (`CollisionBox`, `CollisionManager`).  
- Implementar funciones de detección y resolución paso a paso.  
- Sugerir cómo registrar colisiones en `EntityManager`.  
- Proporcionar ejemplos de integración con `PlayScene`.  
- Incluir logs de debug mostrando entidades en colisión.  
