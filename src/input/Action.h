#ifndef ABYSSAL_STATION_SRC_INPUT_ACTION_H
#define ABYSSAL_STATION_SRC_INPUT_ACTION_H

#include <cstdint>

namespace input {

// Actions that can be mapped to input events (keyboard, mouse, gamepad)
// Keep this enum minimal and scoped (enum class) for type safety.
enum class Action : std::uint8_t {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Confirm,
    Cancel,
    Interact,
    Pause
};

} // namespace input

#endif // ABYSSAL_STATION_SRC_INPUT_ACTION_H
