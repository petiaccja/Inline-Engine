#include "Exception.hpp"
#include <sstream>

namespace inl {


const std::string Exception::emptyString;
const std::vector<StackFrame> Exception::emptyStackTrace;
std::atomic_bool Exception::breakOnce(false);



Exception::Exception() 
	: m_message(std::make_shared<std::string>("Unknown error occured."))
{
	CalculateStackTrace();
	DoBreak();
}

Exception::Exception(std::string message) 
	: m_message(std::make_shared<std::string>(std::move(message)))
{
	CalculateStackTrace();
	DoBreak();
}

Exception::Exception(std::string message, std::string subject) 
	: m_message(std::make_shared<std::string>(std::move(message))), m_subject(std::make_shared<std::string>(std::move(subject)))
{
	m_what = std::make_shared<std::string>(*m_message + ": " + *m_subject);
	CalculateStackTrace();
	DoBreak();
}

Exception::Exception(nullptr_t, std::string subject) 
	: m_message(std::make_shared<std::string>("Unknown error occured.")), m_subject(std::make_shared<std::string>(std::move(subject)))
{
	CalculateStackTrace();
	DoBreak();
}



Exception::Exception(const Exception& rhs) noexcept {
	m_message = rhs.m_message;
	m_subject = rhs.m_subject;
	m_what = rhs.m_what;
	m_stackTrace = rhs.m_stackTrace;
}

Exception::Exception(Exception&& rhs) noexcept {
	m_message = std::move(rhs.m_message);
	m_subject = std::move(rhs.m_subject);
	m_what = std::move(rhs.m_what);
	m_stackTrace = std::move(rhs.m_stackTrace);
}

Exception& Exception::operator=(const Exception& rhs) noexcept {
	m_message = rhs.m_message;
	m_subject = rhs.m_subject;
	m_what = rhs.m_what;
	m_stackTrace = rhs.m_stackTrace;
	return *this;
}

Exception& Exception::operator=(Exception&& rhs) noexcept {
	m_message = std::move(rhs.m_message);
	m_subject = std::move(rhs.m_subject);
	m_what = std::move(rhs.m_what);
	m_stackTrace = std::move(rhs.m_stackTrace);
	return *this;
}


void Exception::BreakOnce() {
	breakOnce.store(true);
}

void Exception::DoBreak() {
	bool shouldBreak = breakOnce.exchange(false);
#ifdef _MSC_VER 
	if (IsDebuggerPresent() && shouldBreak) {
		DebugBreak();
	}
#endif
}


const char* Exception::what() const noexcept {
	return m_what ? m_what->c_str() : (m_message ? m_message->c_str() : "");
}

const std::string& Exception::Message() const noexcept {
	return m_message ? *m_message : emptyString;
}

const std::string& Exception::Subject() const noexcept {
	return m_subject ? *m_subject : emptyString;
}


const std::vector<StackFrame>& Exception::StackTrace() const noexcept {
	return m_stackTrace ? *m_stackTrace : emptyStackTrace;
}

const std::string Exception::StackTraceStr() const {
	std::stringstream trace;
	PrintStackTrace(trace);
	return trace.str();
}

void Exception::PrintStackTrace(std::ostream& os) const {
	if (m_stackTrace) {
		for (int i = 0; i < m_stackTrace->size(); ++i) {
			os << i << (*m_stackTrace)[i].instructionAddress << "!" << (*m_stackTrace)[i].symbol << " " << (*m_stackTrace)[i].sourceFile << ":" << (*m_stackTrace)[i].sourceLine << std::endl;
		}
	}
}

void Exception::CalculateStackTrace() {
	m_stackTrace = std::make_shared<std::vector<StackFrame>>(GetStackTrace());

	// remove first two entries from stack trace:
	// 0: Exception::CalculateStackTrace() <- removed
	// 1: Exception::Exception() <- removed
	// 2: throw Exception() <- this is what we need
	// ...
	if (m_stackTrace->size() > 2) {
		*m_stackTrace = std::vector<StackFrame>(m_stackTrace->begin() + 2, m_stackTrace->end());
	}
}




} // namespace inl