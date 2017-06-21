#pragma once

#include <chrono>

namespace exc {

/// <summary>
/// Utility class for high resolution timing.
/// </summary>
class Timer {
	enum eState {
		RUNNING,
		STOPPED,
		PAUSED
	};
public:
	Timer();

	/// <summary> Start the timer or continue if it was paused. </summary>
	void Start();

	/// <summary> Get the amount of time the clock was running for. </summary>
	/// <returns> Elapsed time in floating point seconds. </returns>
	double Elapsed();

	/// <summary> Pause the timer. Call <see cref="Start()"/> to continue running. </summary>
	void Pause();

	/// <summary> Stops the timer and sets the counter to zero. </summary>
	void Stop();

	/// <summary> Sets the counter to zero. </summary>
	void Reset();

	/// <summary> How fast time goes. Set to 0.2 to record 1 second when real-world time advances by 5 seconds. </summary>
	void SetSpeed(double speed);

	double GetSpeed() const;
private:
	double m_accumulator;
	double m_speed;
	eState m_state;
	std::chrono::high_resolution_clock::time_point m_startTime;
};


} // namespace exc