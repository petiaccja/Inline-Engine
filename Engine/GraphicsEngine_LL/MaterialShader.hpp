#pragma once

#include <BaseLibrary/Graph_All.hpp>


namespace inl::gxeng {


class ShaderManager;


struct MaterialShaderParam {
	const std::string type;
	const std::string name;
};


struct FunctionParameter {
	bool out;
	std::string type;
	std::string name;
};

struct FunctionSignature {
	std::string returnType;
	std::vector<FunctionParameter> parameters;
};


class MaterialShaderInput : public InputPort<void>, public MaterialShaderParam 
{};

class MaterialShaderOutput : public OutputPort<void>, public MaterialShaderParam
{};



class MaterialShader2 : protected NodeBase {
public:
	MaterialShader2(ShaderManager* shaderManager);

	// Shaders
	virtual const std::string& GetShaderCode() const = 0;
	virtual size_t GetHash() const = 0;

	using NodeBase::SetDisplayName;
	using NodeBase::GetDisplayName;

	// Ports
	size_t GetNumInputs() const override { return m_inputs.size(); }
	size_t GetNumOutputs() const override { return m_outputs.size(); }

	MaterialShaderInput* GetInput(size_t index) override { return &m_inputs[index]; }
	MaterialShaderOutput* GetOutput(size_t index) override { return &m_outputs[index]; }

	const MaterialShaderInput* GetInput(size_t index) const override { return &m_inputs[index]; }
	const MaterialShaderOutput* GetOutput(size_t index) const override { return &m_outputs[index]; }

protected:
	// HLSL string processing helpers
	static std::string RemoveComments(std::string code);
	static std::string GetFunctionSignature(const std::string& code, const std::string& functionName);
	static FunctionSignature DissectFunctionSignature(const std::string& signature);

private:
	void Update() {};
	void Notify(InputPortBase* sender) {};

protected:
	std::vector<MaterialShaderInput> m_inputs;
	std::vector<MaterialShaderOutput> m_outputs;
	ShaderManager* m_shaderManager;
};



class MaterialShaderEquation2 : public MaterialShader2 {

};



class MaterialShaderGraph2 : public MaterialShader2 {

};


} // namespace inl::gxeng