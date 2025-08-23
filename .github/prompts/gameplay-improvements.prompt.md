---
mode: agent
---

# Task: Mejoras módulo Gameplay (Items/Puzzles)

## Contexto
El módulo `gameplay` centraliza lógica de **objetos interactivos (Items)** y **Puzzles**. Incluye:
- `Item` y `ItemManager`
- `Puzzle` y `PuzzleManager`
- Integración con `UIManager` (notificaciones)
- Integración opcional con `CollisionManager` (detección de colisiones)
- Input vía `Action::Interact`

El código actual funciona, pero hay mejoras pendientes: duplicación de colliders, falta de serialización, tests unitarios, idempotencia de interacción y vista de bindings en tiempo de ejecución.

## Objetivos del agente

1. **Colliders y detección**
   - Evitar re-add de colliders cada frame.
   - Añadir función `updateColliderBounds(Entity*, sf::FloatRect)` si no existe.
   - Validar que la detección sea idempotente.

2. **Serialización / guardado**
   - Implementar guardar y cargar estado de items (`id`, `position`, `type`, `collected`) y puzzles (`completedSteps_`) en JSON.
   - Permitir restaurar el juego exactamente en el mismo estado.

3. **Integración UI**
   - Notificar a `UIManager` al recoger items o completar puzzles.
   - Mostrar feedback visual (toast, icono).

4. **Concurrencia y seguridad**
   - Asegurar que la recolección múltiple simultánea sea idempotente.
   - Añadir locks mínimos o atomics si es necesario.

5. **Tests unitarios**
   - Crear pruebas para interacción de items, actualización de puzzles y notificaciones UI.
   - Verificar que `interact` sea seguro y consistente.

6. **Mejoras adicionales sugeridas(Realizar)**
   - Detectar items o puzzles fuera de bounds y loggear warnings.
   - Documentar ownership y lifetime de managers.
   - Preparar extensión futura para multiplayer.
   - Optimizar iteración de `ItemManager::updateAll` para evitar comprobaciones innecesarias.
   - Añadir métodos de búsqueda de items/puzzles por `id` para facilitar acceso rápido desde scripts o UI.
   - Soporte opcional para animaciones simples de items recogidos (fade-out o scale-up) antes de desaparecer.
   - Registro de estadísticas: cantidad de items recogidos, puzzles completados, tiempo de completado.
   - Posibilidad de deshabilitar temporalmente items/puzzles sin destruirlos, útil para lógica de niveles dinámicos.
   - Mejora de logging: incluir posición, tipo y estado de item/puzzle en los logs de interacción.

## Requisitos de salida

- Proponer funciones o mejorar existentes (`update`, `interact`, `updateAll`, `markStepCompleted`) siguiendo C++17 y estilo SFML.
- Integración correcta con `UIManager` y `CollisionManager`.
- Sugerir tests unitarios y ejemplos de uso.
- Código seguro respecto a memoria y ownership (`unique_ptr`, referencias seguras).
- Opcional: ideas de mejoras adicionales y refactorizaciones.

## Constraints

- Mantener compatibilidad con el proyecto actual.
- No romper la lógica existente de Items y Puzzles.
- Evitar duplicación de colliders y memory leaks.
- Ser lo más modular posible para facilitar futuras extensiones.

## Success Criteria

- Código que aplica las mejoras descritas.
- Items y puzzles reaccionan correctamente al input del jugador.
- Estado serializable y restaurable.
- UI recibe notificaciones consistentes.
- Tests unitarios cubren los casos más importantes.
- Logging completo y trazable de interacciones y cambios de estado.
- Mejoras extra (animaciones, estadísticas, búsqueda por id) implementadas o sugeridas de forma clara.
