#pragma once

#include "EventEntry.hpp"

#include <mutex>
#include <memory>


namespace inl {

class Logger;
class LogNode;

class LogPipe {
	friend class inl::Logger;
	friend class inl::LogNode;
private:
	/// <summary> Private to allow only Logger to create a pipe. </summary>
	LogPipe(std::shared_ptr<LogNode> node);
public:
	LogPipe(const LogPipe&) = delete;
	LogPipe& operator=(const LogPipe&) = delete;
	~LogPipe();

	/// <summary> Add a new event for logging. </summary>
	void PutEvent(const LogEvent& evt);
	/// <summary> Add a new event for logging. </summary>
	void PutEvent(LogEvent&& evt);

	/// <summary> Get attached log node. </summary>
	std::shared_ptr<LogNode> GetNode();
private:
	EventBuffer buffer; /// <sumary> Temporary buffer for events, so less disk writes. </summary>
	std::shared_ptr<LogNode> node; /// <summary> Which node *this belongs to. </summary>
	std::mutex pipeLock; /// <summary> Prevent concurrent access to this pipe instance. </summary>
};


} // namespace inl
