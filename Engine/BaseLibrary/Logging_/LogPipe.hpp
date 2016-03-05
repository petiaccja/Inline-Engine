#pragma once

#include "EventEntry.hpp"

#include <BaseLibrary/Utility_/Spinlock.hpp>
#include <mutex>


namespace exc {

class LogPipe {
	friend class LogNode;
private:
	/// <summary> Private to allow only LogNode to create a pipe. </summary>
	LogPipe(LogNode* node);
public:
	LogPipe(const LogPipe&) = delete;
	LogPipe& operator=(const LogPipe&) = delete;

	/// <summary> Add a new event for logging. </summary>
	void PutEvent(const Event& evt);
	/// <summary> Add a new event for logging. </summary>
	void PutEvent(Event&& evt);

	/// <summary> Close pipe before closing node! </summary>
	void Close();
private:
	EventBuffer buffer; /// <sumary> Temporary buffer for events, so less disk writes. </summary>
	LogNode* node; /// <summary> Which node *this belongs to. </summary>
	std::mutex pipeLock; /// <summary> Prevent concurrent access to this pipe instance. </summary>
};


} // namespace exc