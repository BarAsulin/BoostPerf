#ifndef _TIMER_HEADER
#define _TIMER_HEADER
#include <chrono>

class Timer
{
private:
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

public:
    void Start();
    float GetDuration();
};

inline void Timer::Start()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
}

inline float Timer::GetDuration()
{
    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - m_StartTime;
    return duration.count();
}

#endif // !_TIMER_HEADER