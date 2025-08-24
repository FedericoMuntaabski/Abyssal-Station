---
mode: agent
---
Define the task to achieve, including specific requirements, constraints, and success criteria.

Contexto breve:
El repositorio contiene varios documentos en `docs/` (por ejemplo `ai-status.md`, `collision-status.md`, `ui-implementation.md`, etc.) que registran cambios y decisiones ya implementadas en distintas áreas del motor (AI, colisiones, core, entidades, gameplay, input, escena, UI). El objetivo de este prompt es guiar a un agente para: 1) revisar e integrar los cambios descritos en `docs/` en la rama actual, 2) concretar tareas accionables por área, y 3) dejar criterios claros de aceptación y pruebas mínimas.

Requerimientos globales (outputs esperados):
- Una lista priorizada de tareas por área (ai, collisions, core, entities, gameplay, input, scene, ui, save_config).
- Para cada tarea: descripción, archivos a tocar (si se conocen), criterios de aceptación, pruebas a ejecutar, restricciones y estimación de esfuerzo (pequeño/medio/grande).
- Un checklist de integración que incluya: build limpio, tests unitarios relevantes pasados, smoke test de la escena (ejecutable en Release/), y actualización de documentación (si aplica).

Restricciones generales:
- No romper compatibilidad pública sin justificación y sin bump de versión (si aplica).
- Mantener estilo y convenciones del repo (C++, CMake, tests existentes).
- Evitar cambios de arquitectura grandes sin pedir aprobación; proponer alternativas cuando sean necesarios.
- No realizar llamadas a red externas ni exfiltrar secretos.

Modo de trabajo (cómo usar este prompt):
1. Leer cada archivo de `docs/` para extraer las entradas "What was done" y las notas de implementación.
2. Generar tareas concretas por área y mapearlas a archivos/funciones existentes.
3. Implementar cambios pequeños/medio y añadir o actualizar tests. Para cambios grandes, publicar una RFC breve y preparar PR separado.
4. Ejecutar: build en modo Release, tests unitarios, y realizar smoke test en `Release/AbyssalStation.exe`.

Tareas solicitadas (para cada área sigue el formato: Título — Requerimientos — Restricciones — Criterio de éxito — Pruebas mínimas — Estimación):

- AI: Integrar mejoras y correcciones documentadas en `docs/ai-implementation-complete.md` y `docs/ai-improvement-implementation.md`.
	- Requerimientos: aplicar cambios de comportamiento (pathfinding, decisiones, estados) según el documento; conservar interfaces públicas de `src/ai/` salvo que la documentación indique refactor necesario.
	- Restricciones: mantener performance; no introducir bloqueos de hilo en lógica principal.
	- Criterio de éxito: nuevas rutas de test en `tests/ai/` pasan; comportamiento observado en escena demuestra la mejora descrita (ej. enemigos toman coberturas, usan comportamiento X).
	- Pruebas mínimas: tests unitarios añadidos/actualizados (happy path + 1 fallo controlado); ejecutar `tests/` relevantes.
	- Estimación: medio.

- Collisions: Aplicar las mejoras listadas en `docs/collision-improvements-implementation.md`.
	- Requerimientos: asegurar corrección de detección/solución; agregar tests de regresión para los casos documentados.
	- Restricciones: mantener compatibilidad con sistemas de físicas actuales; no cambiar formatos de serialización sin actualizar `saves/` y `save_config` docs.
	- Criterio de éxito: tests de colisión pasan; escenas con objetos previamente problemáticos ya no colisionan incorrectamente.
	- Pruebas mínimas: tests automáticos en `tests/collisions/` + reproducción manual de la escena indicada en los docs.
	- Estimación: pequeño a medio.

- Core: Implementaciones y ajustes en `docs/core-status.md`.
	- Requerimientos: aplicar ajustes de API/servicios centrales documentados; verificar que no se rompan subsistemas dependientes.
	- Restricciones: evitar cambios públicos grandes sin despegue en PRs separados.
	- Criterio de éxito: proyecto compila; pruebas unitarias globales pasan.
	- Pruebas mínimas: build completo Release; ejecutar suite de tests.
	- Estimación: pequeño.

- Entities / Player: Integrar cambios señalados en `docs/entities-improvements-implementation.md` y `docs/entities-status.md`.
	- Requerimientos: alinear componentes de entidades con las descripciones; actualizar serialización si documentado.
	- Restricciones: conservar performance en actualización de entidades en loop de juego.
	- Criterio de éxito: entidades se comportan según especificación; tests de entidad pasan.
	- Pruebas mínimas: pruebas unitarias en `tests/entities/`, plus un escenario en el ejecutable donde la entidad X realiza la acción Y.
	- Estimación: medio.

- Gameplay / Items & Puzzles: Según `docs/gameplay-improvements-implementation.md`.
	- Requerimientos: implementar mecánicas nuevas/ajustadas; documentar parámetros de tuning.
	- Restricciones: no alterar save format sin nota y compatibilidad.
	- Criterio de éxito: flujo jugable con nuevas mecánicas; tests automáticos y manual QA.
	- Pruebas mínimas: tests de integración de gameplay y playthrough corto en Release.
	- Estimación: medio a grande.

- Input: Actualizaciones en `docs/input-improvements-implementation.md`.
	- Requerimientos: garantizar que bindings, deadzones y mapeos describos funcionen; añadir pruebas de integración para dispositivos soportados (teclado/mouse/gamepad).
	- Restricciones: no romper los atajos existentes.
	- Criterio de éxito: input coherente en escena y tests de integración pasan.
	- Pruebas mínimas: test manual con teclado y gamepad; automatizados si la infra los soporta.
	- Estimación: pequeño.

- Scene: Consolidar `scene-status.md` y garantizar que la escena principal integre cambios (render, cámara, lighting) documentados.
	- Requerimientos: actualizar sistemas de escena en `src/scene/` conforme a los docs; asegurar que assets referenciados existan en `assets/`.
	- Restricciones: mantener rendimiento objetivo y no aumentar el tiempo de carga significativamente.
	- Criterio de éxito: la escena carga y muestra los elementos nuevos; smoke test visual y sin crashes.
	- Pruebas mínimas: build Release + ejecutar `Release/AbyssalStation.exe` y comprobar la escena objetivo.
	- Estimación: medio.

- UI: Implementaciones en `docs/ui-implementation.md`.
	- Requerimientos: integrar pantallas/elementos UI descritos; seguir la tipografía y assets en `assets/fonts/` y `assets/textures/`.
	- Restricciones: mantener accesibilidad mínima y escalabilidad de UI.
	- Criterio de éxito: UI visible y funcional en escenarios principales; pruebas de interacción básica pasan.
	- Pruebas mínimas: tests manuales de flujo de UI y unit tests si la UI dispone de lógica testeable.
	- Estimación: pequeño a medio.

- Save/Config: Ver `save_config` docs y `saves/example_save.json`.
	- Requerimientos: garantizar compatibilidad de formato salvo que docs indiquen migración; añadir migraciones si se cambia el formato.
	- Restricciones: no perder saves de usuario sin migración.
	- Criterio de éxito: cargar `saves/example_save.json`; round-trip save/load para cambios realizados.
	- Pruebas mínimas: test de serialización round-trip.
	- Estimación: pequeño.

Checklist de integración (para marcar en PR):
- [ ] Compila en Release (sin warnings nuevos relevantes).
- [ ] Tests unitarios y de integración pasan (mencionados por área).
- [ ] Smoke test de la escena principal: ejecutable inicia y escena objetivo renderiza sin crash.
- [ ] Documentación actualizada: modificar el `docs/` correspondiente con notas de integración/decisiones.
- [ ] Si hubo cambios de formato (save/config/serialización), añadir migración y nota en `docs/`.

Edge cases y riesgos a considerar:
- Cambios interdependientes entre subsistemas (p. ej. AI que depende de colisiones) pueden requerir coordinación; planear integración incremental y pruebas combinadas.
- Performance regressions: añadir benchmarks simples si un cambio puede impactar el loop de juego.
- Flakes en tests: identificar tests no determinísticos y estabilizarlos antes de confiar en la suite completa.

Criterios de aprobación final (PR merge):
- Todos los items del checklist de integración completados o documentados con un plan claro para los pendientes.
- PR pequeño y enfocado por cada área cuando el cambio es no trivial; o una PR de integración con un resumen claro y tests verdes.

Notas finales para el agente:
- Prioriza tasks que desbloqueen otras áreas (ej. core, collisions) antes de cambios de coordinación (AI, gameplay).
- Si encuentras una discrepancia entre la implementación actual y lo documentado en `docs/`, añade una entrada de "desviación" en la PR describiendo la diferencia y proponiendo la corrección.
- Mantén los commits pequeños y con mensajes claros que referencien los archivos de `docs/` usados como fuente.

Éxito del encargo: el código en la rama `feature/complete-scene` compila, pasa tests relevantes, la escena objetivo se carga correctamente y los `docs/` quedan mapeados a las tareas implementadas.