#ifndef ABYSSAL_STATION_SRC_AI_AISTATE_H
#define ABYSSAL_STATION_SRC_AI_AISTATE_H

namespace ai {

// Enhanced AI states with additional behaviors
enum class AIState {
    IDLE,           // Standing still, minimal processing
    PATROL,         // Moving between patrol points
    CHASE,          // Actively pursuing a target
    ATTACK,         // In combat range, performing attacks
    FLEE,           // Running away from danger
    RETURN,         // Returning to patrol after losing target
    INVESTIGATE,    // Moving to investigate a sound/disturbance
    ALERT,          // High awareness, scanning for threats
    STUNNED,        // Temporarily incapacitated
    DEAD,           // No longer active
    // NEW: Advanced behavioral states from roadmap
    INVESTIGATE_NOISE,  // Investigating specific noise events
    PREDICT_MOVEMENT,   // Predicting and intercepting player movement
    STALK,             // Following at distance for psychological effect
    AMBUSH,            // Waiting in hiding for surprise attack
    COMMUNICATE,       // Sharing information with other enemies
    SEARCH_LAST_KNOWN, // Checking last known player position
    ESCALATE_ALERT     // Activating more enemies in response to player
};

// Behavior profiles that modify AI decision making
enum class BehaviorProfile {
    AGGRESSIVE,  // Prioritizes chase and attack
    DEFENSIVE,   // Prioritizes flee and caution
    NEUTRAL,     // Balanced behavior
    PASSIVE,     // Rarely attacks, prefers flee
    GUARD,       // Stays near patrol area, defensive
    SCOUT,       // High vision range, alerts others
    // NEW: Psychological and advanced profiles
    STALKER,     // Follows at distance, psychological warfare
    AMBUSHER,    // Waits for optimal attack opportunities
    COMMUNICATOR,// Coordinates with other enemies
    PERSISTENT   // Never gives up searching for player
};

// Perception types for multi-modal sensing
enum class PerceptionType {
    SIGHT,      // Line of sight detection
    HEARING,    // Sound-based detection
    PROXIMITY,  // Close-range detection (touch/smell)
    MEMORY,     // Remembering last known position
    // NEW: Advanced sensory types
    VIBRATION,  // Detecting footsteps and movement
    LIGHT,      // Detecting flashlight and other light sources
    PREDICTION, // Predicting player movement patterns
    COMMUNICATION // Information shared by other enemies
};

// Priority levels for behavior decision making
enum class Priority {
    CRITICAL = 4,   // Life-threatening situations (health < 20%)
    HIGH = 3,       // Important goals (player in attack range)
    MEDIUM = 2,     // Normal behaviors (patrol, investigate)
    LOW = 1         // Background activities (idle animations)
};

const char* stateToString(AIState state);
const char* profileToString(BehaviorProfile profile);
const char* perceptionToString(PerceptionType type);
const char* priorityToString(Priority priority);

} // namespace ai

#endif // ABYSSAL_STATION_SRC_AI_AISTATE_H
