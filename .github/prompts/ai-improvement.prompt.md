---
mode: agent
---

## 🎯 Contexto
Estamos trabajando en el módulo `ai` de un motor de juego retro low-poly.  
El módulo controla **enemigos y NPCs**, integrándose con `entities` (posición, estado, atributos) y `collisions` (detección de obstáculos, raycasting, triggers).  

Actualmente, la IA solo hace un seguimiento básico del jugador con rango de visión limitado.  
Queremos expandirlo a un sistema modular, escalable y con soporte para diferentes comportamientos.

---

## ✅ Mejoras principales (15)
1. Integrar con `CollisionManager::segmentIntersectsAny` para **línea de visión real** (enemigos no ven a través de paredes).  
2. Añadir **sistema de estados (FSM)** para NPCs: Idle, Patrol, Chase, Attack, Flee.  
3. Soporte para **pathfinding básico (A*)**, usando colliders de `Wall` como obstáculos.  
4. Integración con `EntityManager`: IA puede buscar entidades cercanas por tipo (ej: ítems, jugadores).  
5. Implementar **detección por área de colisión** (triggers de proximidad).  
6. Añadir sistema de **percepción múltiple** (vista, oído, rango de proximidad).  
7. Debug visual: dibujar rayos de visión, paths y áreas de detección.  
8. Implementar **prioridades de comportamiento** (ej: huir si vida < 20%, perseguir si jugador en rango).  
9. Logging estructurado: registrar decisiones de IA y eventos clave.  
10. Soporte para **enemigos con diferentes perfiles** (agresivo, defensivo, neutral).  
11. Integración con `CollisionSystem::resolve` para evitar que NPCs atraviesen paredes al perseguir.  
12. Extender AI para soportar **multi-target** (varios jugadores o aliados).  
13. Hooks para **acciones de entidades** (ej: recoger objeto, abrir puerta).  
14. Unit tests básicos: FSM, detección de jugador, pathfinding.  
15. Documentar IA con ejemplos de `Enemy` y `NPC`.

---

## 🔧 Tareas extras (10)
1. Añadir soporte para **patrullas configurables desde archivo** (ej: JSON con waypoints).  
2. Implementar **coordinación entre enemigos** (ej: alertar aliados si uno ve al jugador).  
3. Integrar **sistema de capas de colisión** para IA (enemigos ignoran ítems pero no muros).  

## 🎯 Contexto
Estamos a cargo do módulo `ai` de um motor de jogo retro low‑poly. O módulo controla inimigos e NPCs e integra-se com `entities` (posição, atributos, estado), `collisions` (raycast, triggers, resolução de colisões) e a cena (`scene`).

Atualmente a IA faz um seguimento simples do jogador; o objetivo é transformar isso num sistema modular, testável e escalável para vários comportamentos.

---

## ✅ Melhorias principais (15)
1. Usar detecção de linha de visão real integrando com o gerenciador de colisões (ex.: `CollisionManager::segmentIntersectsAny`) para que inimigos não vejam através de paredes.
2. Implementar uma FSM (Idle, Patrol, Chase, Attack, Flee) reutilizável por entidades.
3. Adicionar pathfinding básico (A*) que trate `Wall`/colisores sólidos como obstáculos.
4. Integração com `EntityManager` para consultas de entidades próximas por tipo (jogador, itens, aliados).
5. Detecção por área (triggers) para eventos de proximidade.
6. Percepção múltipla: visão, audição e proximidade com parâmetros configuráveis.
7. Debug visual: rayos de visão, caminhos calculados e áreas de detecção.
8. Sistema de prioridades de comportamento (ex.: fugir se HP < 20%, priorizar alvos perigosos).
9. Logging estruturado para decisões e eventos relevantes da IA.
10. Perfis de inimigo configuráveis (agressivo, defensivo, passivo).
11. Garantir integração com a resolução de colisões para evitar atravessamento de paredes durante perseguições.
12. Suporte a multi‑target e seleção de alvo.
13. Hooks para ações de entidade (pegar item, abrir porta, interagir com objeto).
14. Testes unitários básicos: FSM, percepção e pathfinding.
15. Documentação e exemplos de uso (`Enemy`, `NPC`) atualizados em `docs/`.

---

## 🔧 Tarefas extras (10)
1. Patrulhas configuráveis por ficheiro (JSON/YAML com waypoints e meta‑dados).
2. Coordenação entre inimigos (sinalizar aliados ao detectar o jogador).
3. Suporte a camadas de colisão para que a IA ignore certos tipos de entidades.
4. Zonas de interesse (ir para som, luz ou objetivos do mapa).
5. Eventos onEnter/onStay para triggers relevantes à IA.
6. Detecção de stuck e recálculo de path ou fallback behavior.
7. Níveis de agressividade por entidade e por perfil.
8. Memória: recordar posição vista do jogador e tempos de validade.
9. IA neutra/pacífica com respostas ao ambiente.
10. Testes de stress com dezenas a centenas de NPCs para validar desempenho.

---

## 📚 Considerar os documentos existentes (pasta `docs/`)
Na pasta `docs/` já existem arquivos de estado e implementações relacionadas (ex.: `ai-status.md`, `collision-improvements-implementation.md`, `core-status.md`, `entities-status.md`, etc.). Este prompt deve orientar a implementação levando em conta esses documentos: alinhar termos, evitar duplicação e manter referências cruzadas.

Requisitos específicos relativos a `docs/`:
- Gerar ou atualizar `docs/ai-improvement-implementation.md` ao término das mudanças, sumarizando decisões arquiteturais, chamadas de API usadas, testes adicionados e exemplos de uso.
- Atualizar `ai-status.md` com o novo plano e progressos.
- Se a implementação tocar colisões, anotar as mudanças em `collision-improvements-implementation.md`.

---

## 📌 Integração esperada
- `entities`: usar atributos como HP, velocidade, layer e callbacks de ação.
- `collisions`: raycast, segment tests, triggers e resolução (para evitar atravessamentos).
- `scene`: orquestrar estados de alto nível (patrulha global, alertas de grupo).
- `ui/debug`: visualização opcional de estados e sinais de percepção.

---

## 🎯 Critérios de sucesso
- Arquitetura modular e extensível (novos comportamentos fáceis de adicionar).
- Determinismo básico onde necessário (simulações reprodutíveis para testes / multiplayer).
- Integração robusta com `entities` e `collisions` sem regressões.

## 🎯 Contexto
Estamos trabajando en el módulo `ai` de un motor de juego retro low‑poly. Este módulo controla enemigos y NPCs y se integra con `entities` (posición, atributos, estado), `collisions` (raycasting, triggers, resolución de colisiones) y la `scene`.

Actualmente la IA sigue al jugador de forma simple. El objetivo es convertirla en un sistema modular, testable y escalable que soporte múltiples comportamientos y perfiles de entidad.

---

## ✅ Mejoras principales (15)
1. Detección de línea de visión real usando el sistema de colisiones (por ejemplo `CollisionManager::segmentIntersectsAny`) para impedir ver a través de muros.
2. FSM reutilizable (estados mínimos: Idle, Patrol, Chase, Attack, Flee) con fácil extensión.
3. Pathfinding básico (A*) considerando `Wall`/colisores sólidos como obstáculos.
4. Integración con `EntityManager` para consultas de entidades cercanas por tipo (jugador, ítems, aliados).
5. Detección por áreas (triggers) para eventos de proximidad y zonas de interés.
6. Percepción múltiple: visión, audición y proximidad con parámetros configurables.
7. Debug visual para rayos de visión, caminos calculados y áreas de detección.
8. Sistema de prioridades y prioridades reactivas (ej.: huir si HP < 20%, perseguir si jugador en rango).
9. Logging estructurado de decisiones y eventos clave de la IA.
10. Perfiles de enemigos configurables (agresivo, defensivo, neutral), preferentemente en data files.
11. Integración con la resolución de colisiones para evitar que NPCs atraviesen muros durante persecuciones.
12. Soporte multi‑target y selección inteligente de objetivo.
13. Hooks para acciones de entidad (recoger objeto, abrir puerta, activar mecanismo).
14. Tests unitarios básicos: FSM, percepción y pathfinding (cobertura mínima para comportamientos críticos).
15. Documentación con ejemplos de uso (`Enemy`, `NPC`) dentro de `docs/`.

---

## 🔧 Tareas adicionales (10)
1. Patrullas configurables desde archivos (JSON/YAML con waypoints y metadatos).
2. Coordinación entre enemigos (alertar/compartir información al detectar al jugador).
3. Soporte de capas/tags de colisión para que la IA ignore ciertos tipos de entidades.
4. Zonas de interés (ir hacia sonido, luz, objetivo de mapa) y manejo de eventos asociados.
5. Eventos onEnter/onStay para triggers relevantes a la IA.
6. Detección de situaciones de bloqueo (stuck) y recálculo de rutas o fallback behaviors.
7. Niveles de agresividad y ajuste dinámico por entidad.
8. Memoria simple: recordar última posición vista del objetivo con tiempos de expiración.
9. Soporte para IA no agresiva/neutra que sólo interactúa con el entorno.
10. Pruebas de carga y estrés (decenas a cientos de NPCs) para medir rendimiento y colisiones.

---

## 📚 Documentación y consistencia con la carpeta `docs/`
En `docs/` ya existen archivos relevantes (por ejemplo `ai-status.md`, `collision-improvements-implementation.md`, `core-status.md`, `entities-status.md`). Este prompt solicita coherencia con esos documentos: usar terminología alineada, evitar duplicaciones y mantener referencias cruzadas.

Acciones concretas sobre `docs/` al finalizar la mejora:
- Crear o actualizar `docs/ai-improvement-implementation.md` con un resumen arquitectural, APIs usadas, decisiones clave, ejemplos y lista de tests añadidos.
- Actualizar `docs/ai-status.md` con el nuevo plan y el progreso.
- Si se modifican sistemas de colisión, reflejar los cambios en `docs/collision-improvements-implementation.md`.

---

## 📌 Integración esperada
- `entities`: usar atributos públicos (HP, velocidad, layer, callbacks de acción) y respetar el lifecycle de la entidad.
- `collisions`: apoyarse en raycasts, segment tests y resolución para percepción y movimiento.
- `scene`: coordinar comportamientos globales (patrullas, alertas por zona, spawns).
- `ui/debug`: hooks opcionales para visualizar estados, rayos y caminos.

---

## 🎯 Criterios de éxito
- Diseño modular y extensible para añadir nuevos comportamientos sin cambios invasivos.
- Determinismo básico donde sea necesario (tests reproducibles / multiplayer).
- Integración limpia con `entities` y `collisions`, sin regresiones funcionales.
- Rendimiento aceptable con decenas de NPCs activos.
- Documentación actualizada en `docs/ai-improvement-implementation.md` y referencias cruzadas en los documentos relevantes.

---

```