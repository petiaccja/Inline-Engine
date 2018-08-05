#pragma once

#include <string>

#undef GetClassName


namespace inl {



class ISerializableOutputPort;



class ISerializableInputPort {
public:
	virtual ~ISerializableInputPort() = default;

	virtual ISerializableOutputPort* GetLink() const = 0;
	virtual std::string ToString() const = 0;
	virtual bool IsSet() const = 0;
};


class ISerializableOutputPort {
public:
	virtual ~ISerializableOutputPort() = default;
};


class ISerializableNode {
public:
	virtual ~ISerializableNode() = default;

	virtual size_t GetNumInputs() const = 0;
	virtual size_t GetNumOutputs() const = 0;

	virtual ISerializableInputPort* GetInput(size_t index) = 0;
	virtual ISerializableOutputPort* GetOutput(size_t index) = 0;

	virtual const ISerializableInputPort* GetInput(size_t index) const = 0;
	virtual const ISerializableOutputPort* GetOutput(size_t index) const = 0;

	virtual const std::string& GetInputName(size_t index) const = 0;
	virtual const std::string& GetOutputName(size_t index) const = 0;

	virtual const std::string& GetDisplayName() const = 0;
	virtual std::string GetClassName() const = 0;
};



} // namespace inl