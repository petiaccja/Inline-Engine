#pragma once

#include "Event.hpp"
#include "EventEntry.hpp"

#include <cstdint>
#include <deque>
#include <chrono>
#include <mutex>


extern volatile int calldepth;


namespace exc {


class Logger;
class LogPipe;

/// <summary> Display events immediately on stdout or stderr. </summary>
enum class eEventDisplayMode {
	DONT_DISPLAY,
	STDOUT,
	STDERR,
};

/// <summary> 
/// Used to create individual stream that all belong to a Logger.
/// Each stream is independent, but events are logged into the same file.
/// Completely thread-safe.
/// <summary>
class LogStream {
	friend class LoggerInterface;
private:
	LogStream(std::shared_ptr<LogPipe> pipe);
public:
	LogStream();
	LogStream(const LogStream&) = delete;
	LogStream(LogStream&&);
	~LogStream();

	LogStream& operator=(const LogStream&) = delete;
	LogStream& operator=(LogStream&&);

	/// <summary> Log an event. </summary>
	/// <param name="displayMode"> Optionally display event immediatly to stdout or stderr. 
	///		Event is still logged. </param>
	void Event(const exc::Event& e, eEventDisplayMode displayMode = eEventDisplayMode::DONT_DISPLAY);

	/// <summary> Log an event. </summary>
	/// <param name="displayMode"> Optionally display event immediatly to stdout or stderr. 
	///		Event is still logged. </param>
	void Event(exc::Event&& e, eEventDisplayMode displayMode = eEventDisplayMode::DONT_DISPLAY);
private:
	std::shared_ptr<LogPipe> pipe; // log goes through this pipe
};


/// <summary> Just a little helper to expose only the ctor to Logger, not the whole class. </summary>
class LoggerInterface {
	friend class Logger;
private:
	/// <summary> Does exactly what you think it does. 
	/// Okay... it creates a LogStream with specified pipe associated.
	/// </summary>
	inline static LogStream Construct(std::shared_ptr<LogPipe> pipe) {
		return LogStream(pipe);
	}
};



} // namespace exc