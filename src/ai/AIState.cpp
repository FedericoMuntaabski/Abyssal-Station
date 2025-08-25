#include "AIState.h"

namespace ai {

const char* stateToString(AIState state) {
    switch (state) {
        case AIState::IDLE: return "IDLE";
        case AIState::PATROL: return "PATROL";
        case AIState::CHASE: return "CHASE";
        case AIState::ATTACK: return "ATTACK";
        case AIState::FLEE: return "FLEE";
        case AIState::RETURN: return "RETURN";
        case AIState::INVESTIGATE: return "INVESTIGATE";
        case AIState::ALERT: return "ALERT";
        case AIState::STUNNED: return "STUNNED";
        case AIState::DEAD: return "DEAD";
        // NEW: Advanced states
        case AIState::INVESTIGATE_NOISE: return "INVESTIGATE_NOISE";
        case AIState::PREDICT_MOVEMENT: return "PREDICT_MOVEMENT";
        case AIState::STALK: return "STALK";
        case AIState::AMBUSH: return "AMBUSH";
        case AIState::COMMUNICATE: return "COMMUNICATE";
        case AIState::SEARCH_LAST_KNOWN: return "SEARCH_LAST_KNOWN";
        case AIState::ESCALATE_ALERT: return "ESCALATE_ALERT";
        default: return "UNKNOWN";
    }
}

const char* profileToString(BehaviorProfile profile) {
    switch (profile) {
        case BehaviorProfile::AGGRESSIVE: return "AGGRESSIVE";
        case BehaviorProfile::DEFENSIVE: return "DEFENSIVE";
        case BehaviorProfile::NEUTRAL: return "NEUTRAL";
        case BehaviorProfile::PASSIVE: return "PASSIVE";
        case BehaviorProfile::GUARD: return "GUARD";
        case BehaviorProfile::SCOUT: return "SCOUT";
        // NEW: Advanced profiles
        case BehaviorProfile::STALKER: return "STALKER";
        case BehaviorProfile::AMBUSHER: return "AMBUSHER";
        case BehaviorProfile::COMMUNICATOR: return "COMMUNICATOR";
        case BehaviorProfile::PERSISTENT: return "PERSISTENT";
        default: return "UNKNOWN";
    }
}

const char* perceptionToString(PerceptionType type) {
    switch (type) {
        case PerceptionType::SIGHT: return "SIGHT";
        case PerceptionType::HEARING: return "HEARING";
        case PerceptionType::PROXIMITY: return "PROXIMITY";
        case PerceptionType::MEMORY: return "MEMORY";
        // NEW: Advanced perception
        case PerceptionType::VIBRATION: return "VIBRATION";
        case PerceptionType::LIGHT: return "LIGHT";
        case PerceptionType::PREDICTION: return "PREDICTION";
        case PerceptionType::COMMUNICATION: return "COMMUNICATION";
        default: return "UNKNOWN";
    }
}

const char* priorityToString(Priority priority) {
    switch (priority) {
        case Priority::CRITICAL: return "CRITICAL";
        case Priority::HIGH: return "HIGH";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

} // namespace ai
