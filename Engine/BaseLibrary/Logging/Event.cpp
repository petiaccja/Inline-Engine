#include "Event.hpp"

namespace exc {


Event::Event() : type(eEventType::UNSPECIFIED) {}

Event::Event(const std::string& message) {
	this->message = message;
}

Event::Event(const Event& other) {
	message = other.message;
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
	switch (parameter.Type()) {
		case eEventParameterType::DEFAULT:
			this->parameters.push_back(
				std::unique_ptr<EventParameter>(new EventParameter(parameter)));
			break;
		case eEventParameterType::FLOAT:
			this->parameters.push_back(
				std::unique_ptr<EventParameter>(new EventParameterFloat(static_cast<const EventParameterFloat&>(parameter))));
			break;
		case eEventParameterType::INT:
			this->parameters.push_back(
				std::unique_ptr<EventParameter>(new EventParameterInt(static_cast<const EventParameterInt&>(parameter))));
			break;
		case eEventParameterType::RAW:
			this->parameters.push_back(
				std::unique_ptr<EventParameter>(new EventParameterRaw(static_cast<const EventParameterRaw&>(parameter))));
			break;
		case eEventParameterType::STRING:
			this->parameters.push_back(
				std::unique_ptr<EventParameter>(new EventParameterString(static_cast<const EventParameterString&>(parameter))));
			break;
	}
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