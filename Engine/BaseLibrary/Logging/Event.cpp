#include "Event.hpp"

namespace exc {


Event::Event() : type(eEventType::UNSPECIFIED) {}

//Event::Event(std::string message, eEventType type) 
//	: message(std::move(message)), type(type)
//{}

Event::Event(const Event& other) {
	message = other.message;
	type = other.type;
	for (size_t i = 0; i < other.GetNumParameters(); i++) {
		PutParameter(other[i]);
	}
}


void Event::SetMessage(const std::string& message) {
	this->message = message;
}

const std::string& Event::GetMessage() const {
	return message;
}


void Event::PutParameter(const EventParameter& parameter) {
	this->parameters.push_back(std::unique_ptr<EventParameter>(parameter.Clone()));
}

size_t Event::GetNumParameters() const {
	return parameters.size();
}


EventParameter& Event::operator[](size_t index) {
	return *parameters[index];
}

const EventParameter& Event::operator[](size_t index) const {
	return *parameters[index];
}

} // namespace exc