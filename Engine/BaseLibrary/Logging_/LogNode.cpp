#include "LogNode.hpp"
#include "LogPipe.hpp"

#include <cassert>
#include <chrono>


namespace exc {


LogNode::LogNode() {
	prohibitPipes = false;
	startTime = std::chrono::high_resolution_clock::now();
	outputStream = nullptr;
	pendingEvents = 0;
}

LogNode::~LogNode() {
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	for (auto& pipeInfo : pipes) {
		// check if a LogStream keeps any pipe open.
		if (pipeInfo.pipe.use_count() != 1) {
			// if so then...

			// write that fact to logfile
			if (outputStream && outputStream->good()) {
				*outputStream << "[ERROR] The program will now terminate because you haven't closed all LogStreams before closing the Logger. Dumfuck...\n";
				outputStream->flush();
			}

			// terminate program, we would have dangling pointers
			std::terminate();
		}
	}

	mtx.unlock();
}


void LogNode::Flush() {
	// Exclude all action by pipes.
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	while (true) {
		EventBuffer* oldestBuffer = nullptr;
		std::string* oldestPipeName = nullptr;
		std::chrono::high_resolution_clock::time_point oldestTimestamp = std::chrono::high_resolution_clock::time_point::max();
		for (auto& pipeInfo : pipes) {
			if (!pipeInfo.pipe->buffer.empty()
				&& pipeInfo.pipe->buffer[0].timestamp < oldestTimestamp)
			{
				oldestTimestamp = pipeInfo.pipe->buffer[0].timestamp;
				oldestBuffer = &pipeInfo.pipe->buffer;
				oldestPipeName = &pipeInfo.name;
			}
		}
		if (oldestBuffer) {
			Event& evt = (*oldestBuffer)[0].event;

			// write event to file
			if (outputStream && outputStream->good()) {
				*outputStream
					<< "[" << std::chrono::duration_cast<std::chrono::microseconds>(oldestTimestamp - startTime).count() / 1.e6 << "]"
					<< "[" << *oldestPipeName << "] "
					<< evt.GetMessage() << "\n";
				for (size_t i = 0; i < evt.GetNumParameters(); i++) {
					*outputStream << "   " << evt[i].name << " = " << evt[i].ToString() << "\n";
				}
			}

			// pop event
			oldestBuffer->pop_front();
			pendingEvents--;
		}
		else {
			// flush file
			if (outputStream && outputStream->good()) {
				outputStream->flush();
			}

			break;
		}
	}

	mtx.unlock();
}


void LogNode::NotifyNewEvent() {
	size_t currentlyPending = ++pendingEvents;
	if (currentlyPending >= flushThreshold) {
		Flush();
	}
}


void LogNode::NotifyClose(LogPipe* pipe) {
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	auto it = pipes.begin();
	while (it != pipes.end() && it->pipe.get() != pipe) {
		++it;
	}
	assert(it != pipes.end());
	pipes.erase(it);

	mtx.unlock();
}


std::shared_ptr<LogPipe> LogNode::CreatePipe(const std::string& name) {
	// Exclude all action by pipes.
	// And anyone else for that matter.
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	auto ptr = std::shared_ptr<LogPipe>(new LogPipe(this));
	pipes.push_back({ ptr, name });

	mtx.unlock();

	return ptr;
}


void LogNode::SetOutputStream(std::ostream* outputStream) {
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	this->outputStream = outputStream;

	mtx.unlock();
}


} // namespace exc