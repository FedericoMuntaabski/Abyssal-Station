#ifndef ABYSSAL_STATION_SRC_SCENE_SAVEINTEGRATION_H
#define ABYSSAL_STATION_SRC_SCENE_SAVEINTEGRATION_H

#include "../core/GameState.h"

namespace entities { class Player; }
namespace gameplay { class ItemManager; class PuzzleManager; }
namespace entities { class EntityManager; }

namespace scene {

// Gather game state from scene managers
core::GameState gatherGameState(entities::EntityManager& em, gameplay::ItemManager& im, gameplay::PuzzleManager& pm);

// Apply loaded game state to scene managers
void applyGameState(const core::GameState& state, entities::EntityManager& em, gameplay::ItemManager& im, gameplay::PuzzleManager& pm);

} // namespace scene

#endif // ABYSSAL_STATION_SRC_SCENE_SAVEINTEGRATION_H
