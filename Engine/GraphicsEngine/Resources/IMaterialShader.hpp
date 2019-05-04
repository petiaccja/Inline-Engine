#pragma once

#include <BaseLibrary/Graph/SerializableNode.hpp>

#include <memory>
#include <vector>


namespace inl::gxeng {


class IMaterialShaderInput;
class IMaterialShader;


class IMaterialShaderOutput : public ISerializableOutputPort {
public:
	virtual void Link(IMaterialShaderInput* target) = 0;
	virtual void UnlinkAll() = 0;
	virtual void Unlink(IMaterialShaderInput* target) = 0;
	virtual const IMaterialShader* GetParent() const = 0;

	virtual const std::vector<IMaterialShaderInput*>& GetLinks() const = 0;

	virtual const std::string& GetType() const = 0;
	virtual const std::string& GetName() const = 0;
};


class IMaterialShaderInput : public ISerializableInputPort {
public:
	virtual void Link(IMaterialShaderOutput* source) = 0;
	virtual void Unlink() = 0;
	virtual const IMaterialShader* GetParent() const = 0;

	IMaterialShaderOutput* GetLink() const override = 0;

	virtual const std::string& GetType() const = 0;
	virtual const std::string& GetName() const = 0;

	virtual void SetDefaultValue(std::string defaultValue) = 0;
	virtual std::string GetDefaultValue() const = 0;

	using ISerializableInputPort::IsSet;
	using ISerializableInputPort::ToString;
};


class IMaterialShader : public ISerializableNode {
public:
	virtual ~IMaterialShader() = default;

	// Shaders
	virtual const std::string& GetShaderCode() const = 0;
	virtual size_t GetHash() const = 0;

	virtual void SetDisplayName(std::string name) = 0;
	const std::string& GetDisplayName() const override = 0;

	std::string GetClassName() const override = 0;

	// Ports
	size_t GetNumInputs() const override = 0;
	size_t GetNumOutputs() const override = 0;

	IMaterialShaderInput* GetInput(size_t index) override = 0;
	IMaterialShaderOutput* GetOutput(size_t index) override = 0;

	const IMaterialShaderInput* GetInput(size_t index) const override = 0;
	const IMaterialShaderOutput* GetOutput(size_t index) const override = 0;

	const std::string& GetInputName(size_t index) const override = 0;
	const std::string& GetOutputName(size_t index) const override = 0;
};


class IMaterialShaderEquation : virtual public IMaterialShader {
public:
	virtual void SetSourceFile(const std::string& name) = 0;
	virtual void SetSourceCode(std::string code) = 0;
};


class IMaterialShaderGraph : virtual public IMaterialShader {
public:
	virtual void SetGraph(std::vector<std::unique_ptr<IMaterialShader>> nodes) = 0;

	/// <summary> Cannot handle nested graphs. </summary>
	virtual void SetGraph(std::string jsonDescription) = 0;
};


} // namespace inl::gxeng