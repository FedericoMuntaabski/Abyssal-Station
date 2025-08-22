#pragma once

#include <chrono>
#include <functional>

// Simple Timer utility for delta time and measuring durations.
class Timer {
public:
    using clock = std::chrono::steady_clock;
    using duration = std::chrono::duration<float>;

    // Construct and start the timer
    Timer();

    // Reset the timer to now
    void reset();

    // Return elapsed time in seconds since start/reset (does not reset)
    float elapsed() const;

    // Restart the timer and return elapsed seconds since last reset
    float restart();

    // Return time (seconds) since last frame and update the internal last-frame marker
    float delta();

private:
    clock::time_point m_start;
    clock::time_point m_last;
};

// RAII helper to measure the duration of a scope. The callback receives seconds as float.
class TimerScope {
public:
    explicit TimerScope(std::function<void(float)> callback);
    ~TimerScope();

private:
    Timer m_timer;
    std::function<void(float)> m_callback;
};
