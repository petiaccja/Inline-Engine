#pragma once

#include "Event.hpp"

#include <chrono>
#include <deque>


namespace inl {

/// <summary> Contains an event and its timestamp. </summary>
struct EventEntry {
	std::chrono::high_resolution_clock::time_point timestamp;
	LogEvent event;
};


/// <summary> Used by LogPipe and LogNode to buffer incoming events. </summary>
using EventBuffer = std::deque<EventEntry>;


}