#include "Exception.hpp"

namespace inl {

Exception::Exception() 
	: m_message("Unknown error occured.")
{
	CalculateStackTrace();
}

Exception::Exception(std::string message) 
	: m_message(std::move(message))
{
	CalculateStackTrace();
}

Exception::Exception(std::string message, std::string subject) 
	: m_message(std::move(message)), m_subject(std::move(subject))
{
	m_what = m_message + ": " + m_subject;
	CalculateStackTrace();
}

Exception::Exception(nullptr_t, std::string subject) 
	: m_message("Unknown error occured."), m_subject(std::move(subject))
{
	CalculateStackTrace();
}



Exception::Exception(const Exception& rhs) {
	m_message = rhs.m_message;
	m_subject = rhs.m_subject;
	m_what = rhs.m_what;
	CalculateStackTrace();
}

Exception::Exception(Exception&& rhs) {
	m_message = std::move(rhs.m_message);
	m_subject = std::move(rhs.m_subject);
	m_what = std::move(rhs.m_what);
	CalculateStackTrace();
}

Exception& Exception::operator=(const Exception& rhs) {
	m_message = rhs.m_message;
	m_subject = rhs.m_subject;
	m_what = rhs.m_what;
	CalculateStackTrace();
}

Exception& Exception::operator=(Exception&& rhs) {
	m_message = std::move(rhs.m_message);
	m_subject = std::move(rhs.m_subject);
	m_what = std::move(rhs.m_what);
	CalculateStackTrace();
}



const char* Exception::what() const {
	return m_what.c_str();
}

const std::string& Exception::Message() const {
	return m_message;
}

const std::string& Exception::Subject() const {
	return m_subject;
}


const std::vector<StackFrame>& Exception::StackTrace() const {
	return m_stackTrace;
}

void Exception::PrintStackTrace(std::ostream& os) const {
	for (int i = 0; i < m_stackTrace.size(); ++i) {
		os << i << m_stackTrace[i].instructionAddress << "!" << m_stackTrace[i].symbol << " " << m_stackTrace[i].sourceFile << ":" << m_stackTrace[i].sourceLine << std::endl;
	}
}

void Exception::CalculateStackTrace() {
	m_stackTrace = GetStackTrace();

	// remove first two entries from stack trace:
	// 0: Exception::CalculateStackTrace() <- removed
	// 1: Exception::Exception() <- removed
	// 2: throw Exception() <- this is what we need
	// ...
	if (m_stackTrace.size() > 2) {
		m_stackTrace = decltype(m_stackTrace)(m_stackTrace.begin() + 2, m_stackTrace.end());
	}
}




} // namespace inl