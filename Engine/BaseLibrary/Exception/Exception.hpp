#pragma once

#include "../StackTrace.hpp"

#include <stdexcept>
#include <string>
#include <cstddef>
#include <ostream>
#include <memory>
#include <atomic>


namespace inl {


//------------------------------------------------------------------------------
// Exception base class
//------------------------------------------------------------------------------
class Exception : public std::exception {
public:
	Exception();
	Exception(std::string message);
	Exception(std::string message, std::string subject);
	Exception(nullptr_t, std::string subject);
	virtual ~Exception() {}

	Exception(const Exception& rhs) noexcept;
	Exception(Exception&& rhs) noexcept;
	Exception& operator=(const Exception& rhs) noexcept;
	Exception& operator=(Exception&& rhs) noexcept;

	const char* what() const noexcept override;

	const std::string& Message() const noexcept;
	const std::string& Subject() const noexcept;
	const std::vector<StackFrame>& StackTrace() const noexcept;
	const std::string StackTraceStr() const;
	void PrintStackTrace(std::ostream& os) const;

	static void BreakOnce();
protected:
	void CalculateStackTrace();
	static void DoBreak();

protected:
	std::shared_ptr<std::string> m_message;
	std::shared_ptr<std::string> m_subject;
	std::shared_ptr<std::string> m_what;
	std::shared_ptr<std::vector<StackFrame>> m_stackTrace;
	static const std::string emptyString;
	static const std::vector<StackFrame> emptyStackTrace;

private:
	static std::atomic_bool breakOnce;
};


//------------------------------------------------------------------------------
// Primary types
//------------------------------------------------------------------------------

class RuntimeException : public Exception {
public:
	using Exception::Exception;
};


class LogicException : public Exception {
public:
	using Exception::Exception;
};



//------------------------------------------------------------------------------
// Logic fuckups
//------------------------------------------------------------------------------

class InvalidArgumentException : public LogicException {
public:
	using LogicException::LogicException;

	InvalidArgumentException() : LogicException("An invalid argument was passed.") {}
	InvalidArgumentException(nullptr_t, std::string subject) : LogicException("An invalid argument was passed.", std::move(subject)) {}
};


class InvalidStateException : public LogicException {
public:
	using LogicException::LogicException;

	InvalidStateException() : LogicException("Invalid state for requested operation.") {}
	InvalidStateException(nullptr_t, std::string subject) : LogicException("Invalid state for requested operation.", std::move(subject)) {}
};


class InvalidCallException : public LogicException {
public:
	using LogicException::LogicException;

	InvalidCallException() : LogicException("Call is not valid in current context.") {}
	InvalidCallException(nullptr_t, std::string subject) : LogicException("Call is not valid in current context.", std::move(subject)) {}
};


class InvalidCastException : public LogicException {
public:
	using LogicException::LogicException;

	InvalidCastException() : LogicException("Invalid typecast.") {}
	InvalidCastException(nullptr_t, std::string subject) : LogicException("Invalid typecast.", std::move(subject)) {}
};


class OutOfRangeException : public LogicException {
public:
	using LogicException::LogicException;

	OutOfRangeException() : LogicException("Argument out of range.") {}
	OutOfRangeException(nullptr_t, std::string subject) : LogicException("Argument out of range.", std::move(subject)) {}
};


class OutOfMemoryException : public LogicException {
public:
	using LogicException::LogicException;

	OutOfMemoryException() : LogicException("Insufficient memory.") {}
	OutOfMemoryException(nullptr_t, std::string subject) : LogicException("Insufficient memory.", std::move(subject)) {}
};


class NotImplementedException : public LogicException {
public:
	using LogicException::LogicException;

	NotImplementedException() : LogicException("This method has not been implemented yet, but the signature is there.") {}
	NotImplementedException(nullptr_t, std::string subject) : LogicException("This method has not been implemented yet, but the signature is there.", std::move(subject)) {}
};


class NotSupportedException : public LogicException {
public:
	using LogicException::LogicException;

	NotSupportedException() : LogicException("Requested operation is not supported.") {}
	NotSupportedException(nullptr_t, std::string subject) : LogicException("Requested operation is not supported.", std::move(subject)) {}
};



//------------------------------------------------------------------------------
// IO
//------------------------------------------------------------------------------

class FileNotFoundException : public RuntimeException {
public:
	using RuntimeException::RuntimeException;
};



} // namespace inl