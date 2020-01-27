#pragma once


#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gxeng {


class GraphicsException : public Exception {
	using Exception::Exception;
};


class PipelineException : public GraphicsException {
	using GraphicsException::GraphicsException;

	PipelineException() : PipelineException("An error occured within the graphics engine render pipeline.") {}
	PipelineException(nullptr_t, std::string subject) : PipelineException("An error occured within the graphics engine render pipeline.", std::move(subject)) {}
};


class InvalidEntityException : public PipelineException {
	using PipelineException::PipelineException;

	InvalidEntityException() : PipelineException("The entity cannot be rendered correctly.") {}
	InvalidEntityException(nullptr_t, std::string subject) : PipelineException("The entity cannot be rendered correctly.", std::move(subject)) {}
};


} // namespace inl::gxeng