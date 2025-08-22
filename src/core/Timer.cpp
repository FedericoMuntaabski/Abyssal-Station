#include "Timer.h"

Timer::Timer()
{
    reset();
    m_last = m_start;
}

void Timer::reset()
{
    m_start = clock::now();
    m_last = m_start;
}

float Timer::elapsed() const
{
    return std::chrono::duration_cast<duration>(clock::now() - m_start).count();
}

float Timer::restart()
{
    auto now = clock::now();
    float elapsedSec = std::chrono::duration_cast<duration>(now - m_start).count();
    m_start = now;
    m_last = now;
    return elapsedSec;
}

float Timer::delta()
{
    auto now = clock::now();
    float dt = std::chrono::duration_cast<duration>(now - m_last).count();
    m_last = now;
    return dt;
}

// TimerScope
TimerScope::TimerScope(std::function<void(float)> callback)
    : m_timer(), m_callback(std::move(callback))
{
}

TimerScope::~TimerScope()
{
    if (m_callback) {
        m_callback(m_timer.elapsed());
    }
}
