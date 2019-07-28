#pragma once

#include "LogNode.hpp"
#include "LogStream.hpp"

#include <BaseLibrary/Singleton.hpp>

#include <fstream>
#include <memory>
#include <ostream>
#include <string>

namespace inl {

/// <summary>
/// <para> Event log facility with file/stream output. </para>
/// <para> A logger can create multiple distinct logstreams.
///		Each logstream represents a flow of events, which are logged into file
///		through the logger. Streams are identified by their names, and can
///		be distinguished in the log file. </para>
/// <para> This class is completely thread-safe. </para>
/// </summary>
class Logger {
public:
	Logger();

	/// <summary> Open a log file for output. </summary>
	bool OpenFile(const std::string& path);

	/// <summary> Use an already opened output stream. </summary>
	void OpenStream(std::ostream* stream);

	/// <summary> Stop logging to output stream, close file, if any. </summary>
	void CloseStream();

	/// <summary> Create a logstream. Use logstreams to log events. </summary>
	LogStream CreateLogStream(const std::string& name);

	/// <summary> Write all pending events to log file immediately. </summary>
	void Flush();

private:
	// do not ever flip the order of the two below!
	// myNode must be destroyed first because it's using outputFile
	std::unique_ptr<std::ofstream> outputFile;
	std::shared_ptr<LogNode> myNode;
};


extern Logger g_logger;


} // namespace inl
