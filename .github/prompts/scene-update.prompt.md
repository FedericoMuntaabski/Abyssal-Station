---
mode: agent
---
Define the task to achieve, including specific requirements, constraints, and success criteria.

Tarea general (en español):
Reestructurar e implementar el módulo de "scenes" para que provea un menú principal jugable y un submenú de opciones. El objetivo es tener una interfaz navegable por teclado y mouse que permita: entrar a una partida en modo solo (mostrar la escena principal), crear una sala (el botón existe pero su funcionalidad queda como placeholder para implementación futura) y abrir un menú de opciones donde se puedan configurar resolución de pantalla, volumen de la música y las teclas por defecto para moverse en escena.

Checklist de tareas concretas:
- Crear/actualizar la escena del Menú Principal con las entradas: "Jugar (Solo)", "Crear Sala" (placeholder), "Opciones", "Salir".
- Implementar la transición para "Jugar (Solo)" que cargue y muestre la escena principal (escena de juego básica).
 - Implementar la transición para "Jugar (Solo)" que cargue y muestre la escena principal (escena de juego básica). Al activar "Jugar (Solo)", la aplicación debe cargar y abrir la escena principal modificada: esta escena principal se debe adaptar para servir como un ejemplo de escenario que integre y demuestre todas las mecánicas de los módulos del proyecto (IA, colisiones, núcleo, entidades, gameplay, input, escena, UI). Es decir, la escena principal será extendida para incluir ejemplos jugables de las funcionalidades de cada módulo.
- Añadir el botón "Crear Sala" con apariencia y navegación, pero sin implementar la lógica de red (dejar marcado como TODO/placeholder).
- Implementar el menú "Opciones" con controles para: resolución de pantalla (lista seleccionable), volumen de la música (slider) y reasignación/restauración de teclas de movimiento por defecto.
- Implementar navegación entre botones usando W, S, ArrowUp, ArrowDown y con soporte por mouse (hover para foco).
- Implementar interacción (activar) con Enter o click izquierdo del mouse.
- Añadir tests o pasos de verificación manual que validen la navegación por teclado y mouse y que las opciones se apliquen correctamente (al menos en sesión).

Requerimientos específicos:
- Controles de navegación: W, S, KeyUp (ArrowUp), KeyDown (ArrowDown) para moverse; el mouse debe poder enfocar elementos por hover.
- Controles de selección: tecla Enter o click izquierdo del mouse para activar un elemento.
- Menú Principal: debe contener claramente las cuatro entradas solicitadas; "Crear Sala" no debe ejecutar lógica de red, solo mostrar estado "pendiente" o similar.
- Opciones: al menos las siguientes opciones deben ser visibles y manipulables:
	- Resolución de pantalla: listado de resoluciones (por ejemplo: 800x600, 1280x720, 1920x1080) y la selección debe aplicar el cambio visualmente en la sesión.
	- Volumen de la música: slider o control numérico que ajusta el volumen en la sesión.
	- Teclas de movimiento: mostrar las teclas por defecto (ej. W/A/S/D o flechas) y permitir restaurar los valores por defecto; reasignación completa puede ser un ajuste mínimo (seleccionar nueva tecla para 'mover arriba' debe registrarse en la configuración de la sesión).
- Accesibilidad mínima: foco visual evidente al navegar con teclado y con hover de mouse.

Restricciones y notas técnicas:
- La funcionalidad de "Crear Sala" queda como placeholder; no implementar red ni matchmaking en esta tarea.
- Cambios deben integrarse respetando la arquitectura de escenas existente (usar el sistema de escenas/escenas del proyecto). Si hay convenciones de proyecto para carga de escenas, seguirlas.
- Evitar cambios grandes y arriesgados fuera del alcance (no refactorizar todo el sistema de input). Hacer adaptaciones localizadas si es necesario.
- Mantener estilo y estructuras de código existentes; documentar los puntos marcados como TODO para continuar en otra rama.
- Si no es posible persistir las opciones entre ejecuciones en esta tarea, documentar eso y asegurar que las opciones se aplican durante la sesión (en memoria).

Criterios de éxito (Definición de Done):
- Menú Principal navegable con W/S/ArrowUp/ArrowDown y con mouse hover; el foco cambia visualmente y coincide con el elemento seleccionado.
- Activación de elementos funciona con Enter y con click izquierdo.
- "Jugar (Solo)" carga la escena principal y ésta se muestra correctamente (pantalla de juego básica funcional).
- "Crear Sala" está presente y al activarla muestra un estado/diálogo que indique que la funcionalidad está pendiente (no debe intentar conectar a red).
- Menú "Opciones" permite cambiar resolución y volume de música y reasignar (o al menos restaurar) teclas de movimiento; los cambios deben aplicarse en la sesión.
- Añadir al menos una prueba manual o automatizada que verifique la navegación por teclado y la activación por Enter.
- El cambio no debe romper la carga normal del juego ni introducir errores de compilación.

Aceptación y pruebas mínimas sugeridas:
- Test manual 1: Navegar el Menú Principal solo con teclado (W/S/ArrowUp/ArrowDown), verificar foco y activar "Jugar (Solo)" con Enter → comprobar que la escena principal se carga.
- Test manual 2: Navegar con mouse (hover) y activar elementos con click izquierdo; verificar que coincide con el comportamiento de teclado.
- Test manual 3: Abrir "Opciones", cambiar resolución y volumen, verificar efecto inmediato en la sesión.
- Test manual 4: Pulsar "Crear Sala" y verificar que aparece un mensaje/estado "Funcionalidad pendiente" sin errores.

Notas adicionales y prioridades:
- Prioridad alta: navegación por teclado y carga de "Jugar (Solo)".
- Prioridad media: menú "Opciones" con resolución y volumen aplicados en sesión.
- Prioridad baja: persistencia de opciones entre ejecuciones y la implementación de red para "Crear Sala" (estas pueden quedar para otra tarea).

Entrega esperada del agente:
- Un conjunto de tareas/PR checklist claro y reproducible que un desarrollador pueda implementar.
- Archivos de código o cambios sugeridos (si procede) y pasos de verificación.
- Marcar explícitamente las partes que quedan como "placeholder/TODO" para futuras implementaciones.
- Implementar pruebas automatizadas para la navegación por teclado y la activación de elementos.
- Documentar el proceso de configuración y uso del menú en el código.
- Documentar en la carpeta de docs lo hecho en los modulos que corresponden.
 - Modificar la escena principal para incluir un escenario ejemplo que integre y demuestre las mecánicas de todos los módulos (esto debe concretarse mediante cambios de código y ejemplos jugables en la escena principal).