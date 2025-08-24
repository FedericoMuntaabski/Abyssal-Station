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
        default: return "UNKNOWN";
    }
}

const char* perceptionToString(PerceptionType type) {
    switch (type) {
        case PerceptionType::SIGHT: return "SIGHT";
        case PerceptionType::HEARING: return "HEARING";
        case PerceptionType::PROXIMITY: return "PROXIMITY";
        case PerceptionType::MEMORY: return "MEMORY";
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
