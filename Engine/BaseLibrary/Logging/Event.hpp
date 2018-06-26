#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <memory>

#undef ERROR

namespace inl {

/// <summary> Denotes the type of polimorphic EventParameters. </summary>
enum class eEventParameterType {
	DEFAULT,
	FLOAT,
	INT,
	RAW,
	STRING,
};

/// <summary> Denote the type of an event:  error, warning, info. Use unspecified if you don't care. </summary>
enum class eEventType {
	UNSPECIFIED,
	ERROR,
	WARNING,
	INFO,
};


/// <summary> 
/// Attach to events as parameters.
/// This class is extended to provide parameters that have values
/// of type float, int or raw bytes. See <see cref="EventParameterFloat"/>,
/// <see cref="EventParameterInt"/> and <see cref="EventParameterRaw"/>.
/// </summary>
struct EventParameter {
	EventParameter() = default;
	EventParameter(const std::string& name) : name(name) {}
	EventParameter(const char* name) : name(name) {}

	/// <summary> Name of the parameter. </summary>
	std::string name;

	/// <summary> Convert the parameter's value to string. </summary>
	virtual std::string ToString() const { return std::string{}; }

	/// <summary> Get the underlying type (Float, Int or Raw). </summary>
	virtual eEventParameterType Type() const { return eEventParameterType::DEFAULT; }

	/// <summary> Virtual copy constructor. </summary>
	virtual EventParameter* Clone() const { return new EventParameter{ *this }; }
};


struct EventParameterFloat : public EventParameter {
	EventParameterFloat() = default;
	EventParameterFloat(const std::string name) : EventParameter(name) {}
	EventParameterFloat(const char* name) : EventParameter(name) {}
	EventParameterFloat(const std::string& name, float value) : EventParameter(name), value(value) {}

	/// <summary> Value associated with the parameter. </summary>
	float value;

	/// <summary> Convert the float value to string. </summary>
	std::string ToString() const override {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	/// <summary> Get the underlying type which is Float. </summary>
	eEventParameterType Type() const override { return eEventParameterType::FLOAT; }

	/// <summary> Virtual copy constructor. </summary>
	virtual EventParameter* Clone() const { return new EventParameterFloat{ *this }; }
};


struct EventParameterInt : public EventParameter {
	EventParameterInt() = default;
	EventParameterInt(const std::string& name) : EventParameter(name) {}
	EventParameterInt(const char* name) : EventParameter(name) {}
	EventParameterInt(std::string name, int value) : EventParameter(name), value(value) {}

	/// <summary> Value associated with the parameter. </summary>
	int value;

	/// <summary> Convert the int value to string. </summary>
	std::string ToString() const override {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	/// <summary> Get the underlying type which is Int. </summary>
	eEventParameterType Type() const override { return eEventParameterType::INT; }

	/// <summary> Virtual copy constructor. </summary>
	virtual EventParameter* Clone() const { return new EventParameterInt{ *this }; }
};


struct EventParameterString : public EventParameter {
	EventParameterString() = default;
	EventParameterString(const std::string& name) : EventParameter(name) {}
	EventParameterString(const char* name) : EventParameter(name) {}
	EventParameterString(std::string name, std::string value) : EventParameter(name), value(value) {}

	/// <summary> Value associated with the parameter. </summary>
	std::string value;

	/// <summary> Convert the string value to quoted string. </summary>
	std::string ToString() const override {
		std::stringstream ss;
		ss << "\"" << value << "\"";
		return ss.str();
	}

	/// <summary> Get the underlying type which is Int. </summary>
	eEventParameterType Type() const override { return eEventParameterType::STRING; }

	/// <summary> Virtual copy constructor. </summary>
	virtual EventParameter* Clone() const { return new EventParameterString{ *this }; }
};


struct EventParameterRaw : public EventParameter {
	/// <summary> Raw binary data associated with parameter. </summary>
	std::vector<uint8_t> data;

	/// <summary> Binary data cannot be converted to string, so it just returns "binary data". </summary>
	std::string ToString() const override {
		return "binary data";
	}

	/// <summary> Get the underlying type which is Raw. </summary>
	eEventParameterType Type() const override { return eEventParameterType::RAW; }

	/// <summary> Virtual copy constructor. </summary>
	virtual EventParameter* Clone() const { return new EventParameterRaw{ *this }; }
};


/// <summary>
/// Describes an event which is to be logged.
/// An event contains a message and can have
/// any number of parameters associated with it.
/// </summary>
class LogEvent {
public:
	LogEvent();

	/// <summary> Create an event with specific message. </summary>
	LogEvent(std::string message) : LogEvent(std::move(message), eEventType::UNSPECIFIED) {}

	/// <summary> Create an event with specific message. </summary>
	LogEvent(const char* message) : LogEvent(std::string(message)) {}
	
	/// <summary> Construct object with message and a list of parameters. </summary>
	/// <param name="message"> The message of the event. </param>
	/// <param name="parameters"> Any number of EventParameters which describe the event's parameters. </param>
	template <class Head, class... Args, std::enable_if_t<!std::is_same_v<Head, eEventType>, int> = 0>
	LogEvent(std::string message, Head&& head, Args&&... parameters)
		: LogEvent(std::move(message), eEventType::UNSPECIFIED, std::forward<Head>(head), std::forward<Args>(parameters)...) {}

	/// <summary> Construct object with message and a list of parameters. </summary>
	/// <param name="message"> The message of the event. </param>
	/// <param name="parameters"> Any number of EventParameters which describe the event's parameters. </param>
	template <class Head, class... Args, std::enable_if_t<!std::is_same_v<Head, eEventType>, int> = 0>
	LogEvent(const char* message, Head&& head, Args&&... parameters) 
		: LogEvent(std::string(message), eEventType::UNSPECIFIED, std::forward<Head>(head), std::forward<Args>(parameters)...) {}

	/// <summary> Construct object with message and a list of parameters and specific event type. </summary>
	/// <param name="message"> The message of the event. </param>
	/// <param name="type"> Type of the event. </param>
	/// <param name="parameters"> Any number of EventParameters which describe the event's parameters. </param>
	template <class... Args>
	LogEvent(std::string message, eEventType type, Args&&... parameters);


	LogEvent(const LogEvent&);
	LogEvent(LogEvent&&) = default;
	~LogEvent() = default;

	/// <summary> Set message of the event. </summary>
	void SetMessage(const std::string& message);
	/// <summary> Get current message. </summary>
	const std::string& GetMessage() const;

	/// <summary> Append a parameter to the end of the parameter list. </summary>
	void PutParameter(const EventParameter& parameter);
	/// <summary> Get number of parameters. </summary>
	size_t GetNumParameters() const;

	/// <summary> Modify indexth parameter. </summary>
	EventParameter& operator[](size_t index);
	/// <summary> Read indexth parameter. </summary>
	const EventParameter& operator[](size_t index) const;
private:
	/// <summary> Helper function for variadic ctor. </summary>
	template <size_t Index, class Head, class... Args>
	void AddVariadicParams(Head&& head, Args&&... args);

	/// <summary> Recursion terminator overload. </summary>
	template <size_t Index>
	void AddVariadicParams();

	std::vector<std::unique_ptr<EventParameter>> parameters;
	std::string message;
	eEventType type;
};



template <class... Args>
LogEvent::LogEvent(std::string message, eEventType type, Args&&... parameters) : message(std::move(message)), type(type) {
	this->parameters.reserve(sizeof...(Args));
	AddVariadicParams<0, Args...>(std::forward<Args>(parameters)...);
}


template <size_t Index, class Head, class... Args>
void LogEvent::AddVariadicParams(Head&& head, Args&&... args) {
	static_assert(std::is_base_of<EventParameter, Head>::value, "Only EventParameters can be given as argument.");
	PutParameter(head);
	AddVariadicParams<Index + 1, Args...>(std::forward<Args>(args)...);
}

template <size_t Index>
void LogEvent::AddVariadicParams() {
	// empty
}



} // namespace inl
