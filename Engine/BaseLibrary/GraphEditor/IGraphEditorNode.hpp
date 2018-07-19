#pragma once


#include <string>
#include <typeindex>
#include "GraphParser.hpp"


#undef GetClassName // retarded Windows


namespace inl {



class IGraphEditorNode {
public:
	virtual ~IGraphEditorNode() = default;

	virtual std::string GetName() const = 0;
	virtual void SetName(std::string name) = 0;
	virtual std::string GetClassName() const = 0;

	virtual int GetNumInputs() const = 0;
	virtual int GetNumOutputs() const = 0;

	virtual bool HasVariableInputs() const = 0;
	virtual bool HasVariableOutputs() const = 0;
	virtual void SetNumInputs() = 0;
	virtual void SetNumOutputs() = 0;

	virtual std::string GetInputName(int idx) const = 0;
	virtual std::string GetOutputName(int idx) const = 0;

	virtual bool HasPortTypes() const = 0;
	virtual std::type_index GetInputType(int idx) const = 0;
	virtual std::type_index GetOutputType(int idx) const = 0;

	virtual std::string GetInputTypeName(int idx) const = 0;
	virtual std::string GetOutputTypeName(int idx) const = 0;

	virtual void SetMetaData(NodeMetaDescription data) = 0;
	virtual NodeMetaDescription GetMetaData() const = 0;
};



} // namespace inl