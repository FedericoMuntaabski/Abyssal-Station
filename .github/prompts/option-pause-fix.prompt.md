---
mode: agent
---
Descripción de la tarea (modo: agent)

Objetivo general:
Implementar y arreglar la interfaz de usuario relacionada al menú principal y al menú de opciones del juego, resolver problemas de acentuación y asegurar persistencia de la configuración.

Requerimientos específicos:
1. Título en el menú principal
   - Aplicar un borde (outline) alrededor del texto del título.
   - El borde debe ser de color naranja, ligeramente opaco (no muy saturado).
   - Mover el título un poco hacia la derecha respecto a su posición actual en la esquina superior izquierda (que siga en la parte superior izquierda pero no pegado al borde).

2. Sonido de hover en botones
   - El sonido de hover debe reproducirse cada vez que el usuario posa el cursor sobre cualquier botón del menú principal y de los menús relacionados (incluido el menú de opciones y pausa).
   - Mantener el mismo comportamiento de sonido que existe al hacer click (click sound) y reutilizar recursos en `assets/sounds`.

3. Soporte de caracteres acentuados
   - Corregir problemas de formato/encoding donde caracteres con acento aparecen como símbolos raros (por ejemplo: "meditación").
   - Asegurar que todos los textos cargados y renderizados por SFML usen UTF-8 y la fuente correcta.
   - Revisar carga de fuentes y el manejo de std::string / sf::String donde corresponda.

4. Menú de opciones estable y funcional
   - Evitar el crash al abrir el menú de opciones desde el menú principal.
   - Implementar un menú de opciones nuevo que reutilice el background del menú principal.
   - El menú debe mostrar opciones centradas verticalmente en la pantalla y con tamaño de texto grande:
     a) Volumen de la música de fondo (`background_music`) — control deslizante o + / -.
     b) Volumen del UI (hover y click) — control deslizante o + / -.
     c) Cambio de resolución — permitir hasta 1920x1080 y ofrecer otras tres resoluciones menores (por ejemplo: 1366x768, 1280x720, 1024x576).
     d) Alternar entre fullscreen y modo ventana.
     e) Botón "Guardar" que persista las opciones para la siguiente sesión.
     f) Botón "Regresar" que devuelve al menú principal.
   - La navegación por botones (arriba/abajo/seleccionar) y los sonidos de hover/click deben comportarse exactamente igual que en el menú principal.

5. Menú de pausa
   - Aplicar la misma lógica de navegación y comportamiento de botones que en el menú principal.
   - Implementar el movimiento entre items para el `PauseMenu` (arriba/abajo/selección) de forma idéntica al menú principal y al menú de opciones.
   - Renombrar el botón "Salir" a "Volver al menú principal" y hacer que su acción cargue el menú principal sin cerrar la aplicación.
   - Asegurarse de que el `PauseMenu` soporte interacción con mouse (hover + sound), teclado y cualquier entrada ya soportada por los otros menús.

Limitaciones y supuestos razonables:
- Lenguaje del proyecto: C++ con SFML (convenios del repositorio).
- Asumo que las fuentes en `assets/fonts` soportan acentos; si no, indicar recomendación de fuente UTF-8.
- Persistencia: usar `config/config.json` (o `config.json`) para guardar y cargar settings; si no existe, crearlo.
- No ejecutar compilación aquí; el agente debe hacer cambios en el código y archivos de recurso/configuración y dejar instrucciones para compilar/probar.

Criterios de éxito (pruebas mínimas y verificaciones):
1. Al abrir el juego e ir al menú principal:
   - El título muestra un borde naranja opaco y está desplazado ligeramente hacia la derecha desde la esquina superior izquierda.
2. Al mover el cursor por los botones (menu principal, opciones, pausa):
   - Se reproduce el sonido de hover en cada entrada al botón.
3. Al abrir el menú de opciones desde el menú principal:
   - No se produce cierre inesperado ni crash.
   - Se muestra el nuevo menú de opciones con fondo del menú principal y opciones centradas y legibles.
4. Al modificar y guardar opciones:
   - Los valores (volumen de música, volumen UI, resolución, modo fullscreen/windowed) se guardan en la configuración y se cargan al reiniciar la aplicación.
5. En el menú de pausa:
   - La navegación entre items funciona con la misma lógica que el menú principal.
   - El botón "Volver al menú principal" lleva efectivamente al menú principal sin cerrar la aplicación.
6. Los textos con acentos se muestran correctamente en todas las pantallas.

Tareas a ejecutar por el agente (pasos concretos):
1. Revisar y, si es necesario, convertir archivos de texto/CPP/headers a UTF-8 sin BOM.
2. Revisar `FontHelper`/carga de fuentes y asegurarse de usar `sf::String` o std::wstring/UTF-8 correcto al crear `sf::Text`.
3. Implementar outline del título en `MainMenu` (o clase responsable) y ajustar su posición X.
4. Auditar código de navegación de menús y `UIManager`/`Menu` para garantizar que el evento hover invoque reproducción del sonido (usar `hover_select.wav`).
5. Corregir crash al abrir `OptionsMenu`: revisar `MenuScene`, `UIManager`, `OptionsMenu` y manejar punteros nulos/recursos faltantes.
6. Implementar la interfaz visual del `OptionsMenu` con controles listados y la lógica de guardar/cargar usando `ConfigManager` o `config/config.json`.
7. Añadir/actualizar pruebas manuales y un pequeño README con cómo probar (abrir menú, cambiar valores, guardar, reiniciar y verificar persistencia).
8. Actualizar `PauseMenu` (`src/ui/PauseMenu.cpp` / `.h`) para reutilizar la lógica de botones y navegación del menú principal, renombrar "Salir" a "Volver al menú principal" y asegurar que los sonidos y la interacción sean iguales a los otros menús.

Casos borde / consideraciones técnicas:
- Valores de volumen fuera de rango: clamp entre 0.0 y 100.0 (o 0.0–1.0 según la API usada).
- Resoluciones no soportadas por la pantalla: priorizar la lista permitida y documentar la limitación.
- Guardado atómico: escribir primero a archivo temporal y renombrar para evitar corrupción.

Salida esperada del agente:
- Parche(s) a los archivos fuente en `src/ui`, `src/scene` y `src/core` (si aplica) que implementen lo descrito.
- Actualización o creación de `config/config.json` (o `config.json`) con esquema de settings.
- Un pequeño archivo `docs/option-menu-fix.md` con pasos para probar y verificación de criterios de éxito.

Si algo no puede implementarse por restricciones del repo (por ejemplo, falta de API para controles deslizantes), el agente debe informar la limitación y proponer una alternativa (por ejemplo: botones + / - en lugar de slider).