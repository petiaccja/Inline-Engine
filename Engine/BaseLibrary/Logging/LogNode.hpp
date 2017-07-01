#pragma once

#include <shared_mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <chrono>

namespace inl {

/// <summary>
/// The LogNode groups together a list of LogPipes.
/// Each pipe collects and buffers events, the lognode periodically sorts
/// and writes these messages to an output stream.
/// </summary>
class LogNode {
	friend class LogPipe;
private:
	struct PipeInfo {
		std::weak_ptr<LogPipe> pipe;
		std::string name;
	};
	struct PipeInfoShared {
		std::shared_ptr<LogPipe> pipe;
		std::string name;
	};
public:
	LogNode();
	~LogNode();

	/// <summary> Force writing all pending events to disk. </summary>
	void Flush();

	/// <summar> Create a pipe connected to *this. </summary>
	void AddPipe(std::shared_ptr<LogPipe> pipe, const std::string& name);

	/// <summary> Specify output stream. </summary>
	void SetOutputStream(std::ostream* outputStream);
private:
	/// <summary> Call this from Pipe whenever a new message is buffered. </summary>
	void NotifyNewEvent();
	
	std::vector<PipeInfo> pipes; /// <summary> List of associated pipes. </summary>	
	std::shared_timed_mutex mtx; /// <summary> Synchronize pipes with node. Node uses exclusive, pipes use shared mode. </summary>
	std::atomic_bool prohibitPipes;	/// <summary> Set to true to give node priority locking mtx. </summary>
	std::atomic_ptrdiff_t pendingEvents; /// <summary> Number of pending events. </summary>

	std::ostream* outputStream;
	std::chrono::high_resolution_clock::time_point startTime; /// <summary> When the logging started. </summary>

	static constexpr ptrdiff_t flushThreshold = 1000; /// <summary> Auto-flush if pending more than this. </summary>
};


} // namespace inl
