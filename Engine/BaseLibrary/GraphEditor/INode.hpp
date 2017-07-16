#pragma once


#include <string>
#include <typeindex>


namespace inl {



class INode {
public:
	virtual std::string GetName() = 0;
	virtual std::string GetClassName() = 0;

	virtual int GetNumInputs() = 0;
	virtual int GetNumOutputs() = 0;

	virtual bool HasVariableInputs() = 0;
	virtual bool HasVariableOutputs() = 0;
	virtual bool SetNumInputs() = 0;
	virtual bool SetNumOutputs() = 0;

	virtual std::string GetInputName(int idx) = 0;
	virtual std::string GetOutputName(int idx) = 0;
	virtual std::type_index GetInputType(int idx) = 0;
	virtual std::type_index GetOutputType(int idx) = 0;
};



} // namespace inl