#ifndef _TIMER_H
#define _TIMER_H

#include <Windows.h>

class Timer
{
private:
	LARGE_INTEGER m_startTime;
	float m_timeTaken;

public:
	Timer();
	~Timer();
	void Start();
	void Stop();
	float GetTime();
};

#endif