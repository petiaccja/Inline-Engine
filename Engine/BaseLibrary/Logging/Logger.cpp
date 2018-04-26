#include "Logger.hpp"
#include "LogNode.hpp"
#include "LogPipe.hpp"

#include <cassert>


namespace inl {


Logger::Logger() {
	myNode = std::make_shared<LogNode>();
	outputFile = std::make_unique<std::ofstream>();
}

Logger::~Logger() {

}

bool Logger::OpenFile(const std::string& path) {
	std::ofstream newStream(path, std::ios::out | std::ios::trunc);
	if (!newStream.is_open()) {
		myNode->SetOutputStream(nullptr);
		return false;
	}
	else {
		myNode->SetOutputStream(nullptr);
		outputFile->close();
		*outputFile = std::move(newStream);
		myNode->SetOutputStream(outputFile.get());
		return true;
	}
}

void Logger::OpenStream(std::ostream* stream) {
	myNode->SetOutputStream(stream);
	outputFile->close();
}

void Logger::CloseStream() {
	myNode->SetOutputStream(nullptr);
	outputFile->close();
}

LogStream Logger::CreateLogStream(const std::string& name) {
	std::shared_ptr<LogPipe> pipe(new LogPipe(myNode));
	myNode->AddPipe(pipe, name);
	return LoggerInterface::Construct(pipe);
}

void Logger::Flush() {
	myNode->Flush();
}




} // namespace inl
