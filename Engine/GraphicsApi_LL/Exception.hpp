#pragma once

#include <exception>
#include <string>

// TODO: add common exception types for the graphics api


namespace inl {
namespace gxapi {


// classes including but not limited to:
//
//	InvalidArgument
//	InvalidState
//	InvalidCall
//	ArgumentNull
//	OutOfMemory
//	FileNotFound
//	ShaderCompilationError
//	OutOfRange
//	etc.

class Exception : public virtual std::exception {
public:
	Exception() = default;
	Exception(const char* message) : m_message(message) {}
	explicit Exception(std::string message) : m_message(message) {}

	const char* what() const override {
		return m_message.c_str();
	}

	const std::string& Message() const {
		return m_message;
	}
private:
	std::string m_message;
};



class InvalidArgument : public Exception {
public:
	using Exception::Exception;
	InvalidArgument(std::string message, std::string argumentName) : Exception(message), m_argumentName(argumentName) {}

	const std::string& Argument() const {
		return m_argumentName;
	}
private:
	std::string m_argumentName;
};



class InvalidState : public Exception {
public:
	using Exception::Exception;
};



class InvalidCall : public Exception {
public:
	using Exception::Exception;
};


class InvalidCast : public Exception {
public:
	using Exception::Exception;
};


class ArgumentNull : public Exception {
public:
	using Exception::Exception;
};



class OutOfMemory : public Exception {
public:
	using Exception::Exception;
	OutOfMemory(std::string message, size_t requiredMemoryHint) : Exception(message), m_requiredMemory(requiredMemoryHint) {}

	size_t MemoryHint() const {
		return m_requiredMemory;
	}
private:
	size_t m_requiredMemory;
};



class FileNotFound : public Exception {
public:
	using Exception::Exception;
	FileNotFound(std::string message, std::string path) : Exception(message), m_path(path) {}

	const std::string& Path() const {
		return m_path;
	}
private:
	std::string m_path;
};



class ShaderCompilationError : public Exception {
public:
	using Exception::Exception;
};



class OutOfRange : public Exception {
public:
	using Exception::Exception;
};



class NotImplementedMethod : public Exception {
public:
	using Exception::Exception;
};



class UnknownError : public Exception {
public:
	using Exception::Exception;
};



} // namespace gxapi
} // namespace inl
