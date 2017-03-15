// Windows implementation of high resolution timer
#pragma once
#include <chrono>

class Timer
{
public:
	Timer();

	void Start();
	void Reset();

	float GetSecondsPassed();
	float GetMicroSecondsPassed();

private:
	std::chrono::steady_clock::time_point startTime;
};