#pragma once

#include <BaseLibrary/Exception/Exception.hpp>

// TODO: add common exception types for the graphics api


namespace inl::gxapi {



class ShaderCompilationError : public RuntimeException {
public:
	using RuntimeException::RuntimeException;

	ShaderCompilationError() : RuntimeException("Shader compilation failed.") {}
	ShaderCompilationError(nullptr_t, std::string subject) : RuntimeException("Shader compilation failed.", std::move(subject)) {}
};


} // namespace inl::gxapi
