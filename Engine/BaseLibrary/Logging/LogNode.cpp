#include "LogNode.hpp"
#include "LogPipe.hpp"

#include <cassert>
#include <chrono>
#include <iostream>


namespace inl {


LogNode::LogNode() {
	prohibitPipes = false;
	startTime = std::chrono::high_resolution_clock::now();
	outputStream = nullptr;
	pendingEvents = 0;
}

LogNode::~LogNode() {
	// this code is probably not necessery since i replaced all shit with shared and weak ptrs

	//prohibitPipes = true;
	//mtx.lock();
	//prohibitPipes = false;

	//for (auto& pipeInfo : pipes) {
	//	// check if a LogStream keeps any pipe open.
	//	if (pipeInfo.pipe.use_count() != 1) {
	//		// if so then...

	//		// write that fact to logfile
	//		if (outputStream && outputStream->good()) {
	//			*outputStream << "[ERROR] The program will now terminate because you haven't closed all LogStreams before closing the Logger. Dumfuck...\n";
	//			outputStream->flush();
	//		}

	//		// terminate program, we would have dangling pointers
	//		std::terminate();
	//	}
	//}

	//mtx.unlock();
}


void LogNode::Flush() {
	// Exclude all action by pipes.
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	// promote all pipes to shared_ptr
	std::vector<PipeInfoShared> promotedPipes;
	bool dirty = false;
	for (auto& pipeInfo : pipes) {
		std::shared_ptr<LogPipe> locked = pipeInfo.pipe.lock();
		if (locked) {
			promotedPipes.push_back({ locked, pipeInfo.name });
		}
		else {
			dirty = true;
		}
	}
	if (dirty) {
		pipes.clear();
		for (auto& pipeInfo : promotedPipes) {
			pipes.push_back({ pipeInfo.pipe, pipeInfo.name });
		}
	}


	// pop events from pipes
	while (true) {
		EventBuffer* oldestBuffer = nullptr;
		std::string* oldestPipeName = nullptr;
		std::chrono::high_resolution_clock::time_point oldestTimestamp = std::chrono::high_resolution_clock::time_point::max();
		for (auto& pipeInfo : promotedPipes) {
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


void LogNode::AddPipe(std::shared_ptr<LogPipe> pipe, const std::string& name) {
	// Exclude all action by pipes.
	// And anyone else for that matter.
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	pipes.push_back({ pipe, name });

	mtx.unlock();
}


void LogNode::SetOutputStream(std::ostream* outputStream) {
	prohibitPipes = true;
	mtx.lock();
	prohibitPipes = false;

	this->outputStream = outputStream;

	mtx.unlock();
}


} // namespace inl
