#include "pch.hpp"
#include "FrameTimer.hpp"


using namespace std;
using namespace chrono;

FrameTimer::InternalTimer FrameTimer::InternalTimer::s_timer;
uint64_t FrameTimer::InternalTimer::m_frameCount = 0;

double FrameTimer::NewFrame()
{
    InternalTimer::s_timer.stop();
    InternalTimer::s_timer.start();
    return dt();
}

void FrameTimer::print(std::wstring headerMsg)
{
    std::wstring outputStr(
        L"\n--- " + headerMsg + L" ---\n" +
        L"Elapsed: " + std::to_wstring(getTime(Duration::MILLISECONDS)) + L"ms " +
        L"(" + std::to_wstring(getTime(Duration::SECONDS)) + L"s).\n"
    );
    std::wcout << outputStr.c_str();
}

double FrameTimer::getTime(Duration duration)
{
    switch (duration)
    {
    case Duration::MILLISECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count() * 1E-03);    // Milliseconds with decimal
        break;
    case Duration::SECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count() * 1E-06);    // Seconds with decimal
        break;
    case Duration::MICROSECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count());
        break;

        // Microseconds default
    default:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count());
    }
}

double FrameTimer::dt()
{
    return InternalTimer::s_timer.m_duration.count() * 1E-06;    // Seconds with decimal
}

uint64_t FrameTimer::frame()
{
    return InternalTimer::m_frameCount;
}

FrameTimer::InternalTimer::InternalTimer() :
    m_start(high_resolution_clock::now()),
        m_end(high_resolution_clock::now()),
        m_duration(0)
{

}

void FrameTimer::InternalTimer::start()
{
    m_start = high_resolution_clock::now();
}

void FrameTimer::InternalTimer::stop()
{
    m_end = high_resolution_clock::now();
    m_duration = duration_cast<microseconds>(m_end - m_start);
    m_frameCount++;
}
