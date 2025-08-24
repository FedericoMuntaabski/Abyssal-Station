---
mode: agent
---
Objetivo
-------
Consolidar y unificar la documentación existente bajo la carpeta `docs/` en archivos únicos por módulo (o en un único documento maestro cuando corresponda) para facilitar el seguimiento, el contexto y la mantenibilidad del proyecto.

Alcance
------
- Leer la estructura y el contenido actual en `docs/` (incluyendo `modules/`, `integration/`, `archive/` y `PROJECT_SUMMARY.md`).
- Generar archivos unificados claros y consistentes que reemplacen o agrupen la documentación fragmentada por módulo.
- Mantener historial y autores cuando exista; crear archivos de respaldo de los originales.

Entregables
-----------
- `docs/modules/<ModuleName>.md` para cada módulo principal (por ejemplo: `AI.md`, `Collisions.md`, `Core.md`, `Entities.md`, `Gameplay.md`, `Input.md`, `Scene.md`, `UI.md`) — cada archivo debe ser la fuente única de verdad para ese módulo.
- Un `docs/README.md` o `docs/INDEX.md` que describa la estructura y enlaces a los archivos unificados.
- Carpeta `docs/backups/` con copias de los archivos originales (con sufijo de fecha).
- Lista de cambios (changelog corto) indicando qué archivos fueron combinados.

Tareas (pasos sugeridos)
------------------------
1. Auditoría: leer todos los archivos dentro de `docs/` y mapear por módulo qué información existe (status, implementaciones, mejoras, integraciones, notas históricas).
2. Plantilla: crear una plantilla de archivo unificado por módulo con secciones mínimas obligatorias (Resumen, Estado actual, Implementación, Mejoras pendientes, Dependencias, Referencias, Historial/Autores).
3. Consolidación: para cada módulo, combinar el contenido disperso en un solo archivo siguiendo la plantilla. Mantener subsecciones claras y, cuando haya contenido duplicado o contradictorio, anotar ambos y priorizar la más reciente o la que tenga mayor contexto técnico.
4. Backups: antes de sobrescribir o mover, copiar los archivos originales a `docs/backups/YYYYMMDD/`.
5. Índice: actualizar o crear `docs/README.md` con enlaces a los archivos unificados y guía rápida de navegación.
6. Verificación: correr una revisión rápida (manual o script simple) que compruebe que todos los archivos referenciados existen y que no se han perdido documentos importantes.
7. PR: preparar un commit/PR con los archivos nuevos, backups y un changelog breve explicando las decisiones y las migraciones.

Requisitos específicos
---------------------
- No eliminar información sin respaldarla: cualquier contenido eliminado debe conservarse en `docs/backups/` y referenciado en el changelog.
- Cada archivo unificado debe incluir metadatos al inicio: Título, Módulo, Última actualización, Autores (si están disponibles), y Etiquetas (status/implementación).
- Mantener formato Markdown y compatibilidad con el estilo existente (cabeceras, listas, enlaces relativos).
- Preservar referencias a issues o PRs si existen (copiar enlaces).

Limitaciones y restricciones
---------------------------
- No modificar código fuente ni archivos fuera de `docs/` salvo para actualizar enlaces relativos que apunten a los docs unificados.
- No se debe reescribir el contenido técnico en profundidad: consolidar y clarificar; las correcciones sustantivas requieren revisión adicional y se deben marcar como "propuesta" en el changelog.
- Si hay ambigüedad entre dos versiones de la misma sección, conservar ambas y anotar la preferida junto a la razón.

Criterios de éxito (definición de "hecho")
---------------------------------------
- Para cada módulo listado en `docs/modules/` existe un único archivo unificado con las secciones mínimas obligatorias.
- `docs/README.md` enlaza claramente a cada archivo unificado.
- Todos los archivos originales que fueron reemplazados están presentes en `docs/backups/YYYYMMDD/` y listados en el changelog.
- No hay enlaces rotos entre documentos internos (se verifica con un script simple o revisión manual dirigida).
- El PR incluye una descripción breve y el changelog; al menos una persona (revisor) puede navegar la nueva estructura y encontrar la misma información o más que antes.

Formato sugerido para cada archivo unificado
------------------------------------------
---
title: "<Module Name>"
module: "<module-identifier>"
last_updated: "YYYY-MM-DD"
authors: ["Nombre1", "Nombre2"]
tags: [status, implementation]
---

# <Module Name>

## 1. Resumen
Breve descripción del propósito del módulo.

## 2. Estado actual
Breve estado (implementado/parcial/prototipo) y notas relevantes.

## 3. Implementación
Descripción técnica de cómo está implementado hoy. Incluir enlaces a archivos fuente o tests si aplica.

## 4. Mejoras pendientes / To-Do
Lista priorizada de mejoras o deudas técnicas.

## 5. Dependencias
Librerías, módulos o sistemas que afectan este módulo.

## 6. Referencias
Enlaces a documentos originales, issues, PRs y recursos externos.

## 7. Historial / Backups
Lista de archivos combinados y ruta en `docs/backups/` con notas de migración.

Instrucciones de entrega
-----------------------
- Generar branch feature/docs-unify-<fecha> con los cambios.
- Incluir en la PR: descripción, checklist de verificación (criterios de éxito) y pedir revisión de al menos un mantenedor del módulo.

Notas adicionales
----------------
- Si el contenido es muy extenso para quedar en un solo archivo por módulo, dividir en subdocumentos bajo `docs/modules/<module>/` y mantener un `README.md` que los enlace y actúe como archivo unificado lógico.
- Proponer en la PR una tarea posterior para mantener la documentación (ownership) y una plantilla de contribución para docs.

Fin del prompt.