#include "Timer.hpp"

Timer::Timer() {
	m_state = STOPPED;
	m_accumulator = 0.0;
	m_speed = 1.0;
}


void Timer::Start() {
	m_state = RUNNING;
	m_startTime = std::chrono::high_resolution_clock::now();
}

double Timer::Elapsed() {
	if (m_state == RUNNING) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_startTime);
		return m_accumulator + (double)diff.count() / 1e9 * m_speed;
	}
	else if (m_state == STOPPED) {
		return 0.0;
	}
	else if (m_state == PAUSED) {
		return m_accumulator;
	}
}

void Timer::Pause() {
	if (m_state == RUNNING) {
		m_accumulator = Elapsed();
		m_state = PAUSED;
	}
}

void Timer::Reset() {
	m_accumulator = 0.0;
	m_state = STOPPED;
}

void Timer::SetSpeed(double speed) {
	Pause();
	m_speed = speed;
	Start();
}

double Timer::GetSpeed() const {
	return m_speed;
}