#include "Event.hpp"

namespace inl {


LogEvent::LogEvent() : type(eEventType::UNSPECIFIED) {}

//LogEvent::LogEvent(std::string message, eEventType type) 
//	: message(std::move(message)), type(type)
//{}

LogEvent::LogEvent(const LogEvent& other) {
	message = other.message;
	type = other.type;
	for (size_t i = 0; i < other.GetNumParameters(); i++) {
		PutParameter(other[i]);
	}
}


void LogEvent::SetMessage(const std::string& message) {
	this->message = message;
}

const std::string& LogEvent::GetMessage() const {
	return message;
}


void LogEvent::PutParameter(const EventParameter& parameter) {
	this->parameters.push_back(std::unique_ptr<EventParameter>(parameter.Clone()));
}

size_t LogEvent::GetNumParameters() const {
	return parameters.size();
}


EventParameter& LogEvent::operator[](size_t index) {
	return *parameters[index];
}

const EventParameter& LogEvent::operator[](size_t index) const {
	return *parameters[index];
}

} // namespace inl
