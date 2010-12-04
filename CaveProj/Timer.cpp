#include "timer.h"

#include <sstream>

Timer::Timer() : 
	m_timeTaken( 0.0 )
{

}

Timer::~Timer()
{
}

void Timer::Start()
{
	QueryPerformanceCounter(&m_startTime);
}

void Timer::Stop()
{
	LARGE_INTEGER endTime;
	LARGE_INTEGER freq;

	QueryPerformanceCounter(&endTime);
	QueryPerformanceFrequency(&freq);
	m_timeTaken = (float)(endTime.QuadPart-m_startTime.QuadPart)/freq.QuadPart;
}

float Timer::GetTime()
{
	return m_timeTaken;
}