#include "Timer.hpp"

#include <cassert>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
//#include <chrono>
//static unsigned long long frequency;

Timer::Timer() {
	//QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	//assert(frequency > 0);
}

void Timer::Start()
{
	startTime = std::chrono::high_resolution_clock::now();

	//QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
}

void Timer::Reset()
{
	Start();
}

float Timer::GetSecondsPassed()
{	
	auto currTime = std::chrono::high_resolution_clock::now();
	return (double)std::chrono::duration_cast<std::chrono::microseconds>(currTime - startTime).count() / 1.0e6f;
}

float Timer::GetMicroSecondsPassed()
{
	auto currTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(currTime - startTime).count();
}