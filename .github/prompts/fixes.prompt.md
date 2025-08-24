---
mode: agent
---
Resumen breve

Este documento define de forma concisa las tareas a realizar, los requerimientos funcionales y no funcionales, las restricciones y los criterios de éxito para una tarea de corrección/implementación en el repositorio.

Checklist de entrega (visión rápida)

- [ ] Tareas desglosadas y priorizadas
- [ ] Requerimientos claros y medibles
- [ ] Restricciones técnicas y de alcance identificadas
- [ ] Criterios de éxito verificables
- [ ] Revisión de código: redundancias y archivos no usados documentados

Tareas a realizar

1. Analizar el área del código afectada y reproducir el problema (si aplica).
1.1. Revisar el código del módulo/área afectada en busca de:
	- Código redundante o duplicado.
	- Archivos o recursos no referenciados/obsoletos.
	- Comentarios desactualizados o documentación faltante.
	Documentar los hallazgos en `docs/` o en el `README`/`.md` del módulo correspondiente.
2. Diseñar la solución: cambios mínimos necesarios y riesgos.
3. Implementar la corrección/funcionalidad en una rama aislada.
4. Añadir o actualizar tests automatizados (unitarios/integración) que cubran el caso feliz y al menos 1-2 bordes.
5. Ejecutar build, linters y tests; corregir fallos hasta pasar.
6. Preparar PR con descripción, lista de cambios y pasos para reproducir/verificar.

Requerimientos

- Funcionales:
	- La corrección debe resolver el comportamiento reportado (descripción reproducible).
	- Mantener compatibilidad con las APIs públicas existentes salvo que se acuerde romperlas.

	- Revisar y documentar redundancias y archivos no usados detectados en el área afectada. Si la acción propuesta implica eliminar archivos, listar dichos archivos en la PR y justificar la eliminación.

- No funcionales:
	- El cambio no debe degradar el rendimiento significativamente (medible con pruebas si procede).
	- Mantener el nivel de cobertura de tests del módulo o aumentarlo.

Restricciones

- Tiempo: aplicar cambios en una rama y PR pequeña (preferible commits atómicos).
- Dependencias: evitar añadir dependencias externas salvo justificación clara.
- Entorno: los cambios deben compilar en la configuración principal de CI (por ejemplo, Release/x64) y no requerir herramientas propietarias.
 - Eliminaciones de archivos: no borrar archivos del repositorio sin una justificación en la PR y preferiblemente separar la eliminación en un commit/revisión aparte para facilitar la revisión.

Criterios de éxito (verificables)

- Criterio principal: el fallo original ya no se reproduce en la rama de trabajo y los tests relevantes pasan.
- Calidad: la build completa (compilación + linters) pasa sin errores en la rama del PR.
- Pruebas: al menos un test nuevo que cubra el caso corregido y que sea estable.
- Revisión: PR aprobado por al menos un revisor y merge sin regressiones en CI.

- Documentación de limpieza: se ha añadido un documento o una sección en el `docs/` o en el `.md` del módulo donde se listan redundancias detectadas y archivos no usados, junto con la recomendación (eliminar/archivar/ignorar) y la justificación técnica.
- Seguridad de cambios: si se propone eliminar archivos, la PR contiene pruebas o referencias que demuestran que los archivos no se usan (por ejemplo, búsquedas, grafo de dependencias, cobertura de tests).

Suposiciones (si no se especifica lo contrario)

- El repositorio tiene integración continua que ejecuta build y tests; si no, se documentará cómo validar localmente.
- Se dispone de permisos para crear ramas y PRs en el repositorio.

Entregables

- Rama de feature/bugfix con commits claros.
- Tests automatizados nuevos/actualizados.
- PR con descripción, capturas o registros que muestren la verificación.

- Documento en `docs/` o actualización del `.md` del/los módulo(s) que documente redundancias y archivos no usados, con la recomendación sobre cada elemento (mantener/eliminar/archivar) y la evidencia que lo respalda.

Notas finales

Mantener los cambios pequeños y reversibles. Si una solución requiere trabajo mayor (refactor o cambio de API), crear una propuesta separada con plan de migración y estimación de tiempo.
