#pragma once

#include <string>
#include <vector>
#include <memory>


namespace inl::gxeng {


class ShaderManager;
class MaterialShader2;


struct FunctionParameter {
	bool out;
	std::string type;
	std::string name;
};

struct FunctionSignature {
	std::string returnType;
	std::vector<FunctionParameter> parameters;
};


class MaterialShaderInput {
	friend class MaterialShaderOutput;
public:
	MaterialShaderInput() = default;
	MaterialShaderInput(const MaterialShader2& parent);
	MaterialShaderInput(const MaterialShader2& parent, std::string name, std::string type, int index);

	void Link(MaterialShaderOutput& source);
	void Unlink();
	const MaterialShader2* GetParent() const;

	const MaterialShaderOutput* GetLink() const;

	const std::string name;
	const std::string type;
	const int index = 0;
private:
	MaterialShaderOutput* m_link = nullptr;
	const MaterialShader2* m_parent = nullptr;
};

class MaterialShaderOutput {
public:
	MaterialShaderOutput() = default;
	MaterialShaderOutput(const MaterialShader2& parent);
	MaterialShaderOutput(const MaterialShader2& parent, std::string name, std::string type, int index);

	void Link(MaterialShaderInput& target);
	void UnlinkAll();
	void Unlink(MaterialShaderInput& target);
	const MaterialShader2* GetParent() const;

	const std::vector<MaterialShaderInput*>& GetLinks() const;

	const std::string name;
	const std::string type;
	const int index = 0;
private:
	std::vector<MaterialShaderInput*> m_links;
	const MaterialShader2* m_parent = nullptr;
};



class MaterialShader2 {
public:
	MaterialShader2(ShaderManager* shaderManager);

	// Shaders
	virtual const std::string& GetShaderCode() const = 0;
	virtual size_t GetHash() const = 0;

	void SetDisplayName(std::string name);
	const std::string& GetDisplayName() const;

	// Ports
	size_t GetNumInputs() const { return m_inputs.size(); }
	size_t GetNumOutputs() const { return m_outputs.size(); }

	MaterialShaderInput* GetInput(size_t index) { return &m_inputs[index]; }
	MaterialShaderOutput* GetOutput(size_t index) { return &m_outputs[index]; }

	const MaterialShaderInput* GetInput(size_t index) const { return &m_inputs[index]; }
	const MaterialShaderOutput* GetOutput(size_t index) const { return &m_outputs[index]; }

protected:
	// HLSL string processing helpers
	static std::string RemoveComments(std::string code);
	static std::string GetFunctionSignature(const std::string& code, const std::string& functionName);
	static FunctionSignature DissectFunctionSignature(const std::string& signature);

protected:
	std::vector<MaterialShaderInput> m_inputs;
	std::vector<MaterialShaderOutput> m_outputs;
	ShaderManager const* m_shaderManager;
	std::string m_name;
};



class MaterialShaderEquation2 : public MaterialShader2 {
public:
	MaterialShaderEquation2(ShaderManager* shaderManager) : MaterialShader2(shaderManager) {}

	const std::string& GetShaderCode() const override;

	void SetSourceFile(const std::string& name);
	void SetSourceCode(std::string code);

private:
	void CreatePorts(const std::string& code);

private:
	std::string m_sourceCode;
};



class MaterialShaderGraph2 : public MaterialShader2 {
public:
	MaterialShaderGraph2(ShaderManager* shaderManager) : MaterialShader2(shaderManager) {}

	const std::string& GetShaderCode() const override;

	void SetGraph(std::vector<std::unique_ptr<MaterialShader2>> nodes);
};


} // namespace inl::gxeng