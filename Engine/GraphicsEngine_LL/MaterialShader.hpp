#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tuple>

#include <BaseLibrary/Graph/Port.hpp>
#include <BaseLibrary/Graph/SerializableNode.hpp>

#ifdef _MSC_VER // disable lemon warnings
#pragma warning(push)
#pragma warning(disable: 4267)
#endif

#include <lemon/euler.h>
#include <lemon/list_graph.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#undef GetClassName


namespace inl::gxeng {


class ShaderManager;
class MaterialShader2;
class MaterialShaderInput;
class MaterialShaderOutput;


struct FunctionParameter {
	bool out;
	std::string type;
	std::string name;
};

struct FunctionSignature {
	std::string returnType;
	std::vector<FunctionParameter> parameters;
};


class MaterialShaderOutput : public ISerializableOutputPort {
public:
	MaterialShaderOutput() = default;
	MaterialShaderOutput(const MaterialShader2& parent);
	MaterialShaderOutput(const MaterialShader2& parent, std::string name, std::string type, int index);

	void Link(MaterialShaderInput* target);
	void UnlinkAll();
	void Unlink(MaterialShaderInput* target);
	const MaterialShader2* GetParent() const;

	const std::vector<MaterialShaderInput*>& GetLinks() const;

	const std::string& GetType() const;
	const std::string& GetName() const;

	const std::string name;
	const std::string type;
	const int index = 0;
private:
	std::vector<MaterialShaderInput*> m_links;
	const MaterialShader2* m_parent = nullptr;
};



class MaterialShaderInput : public ISerializableInputPort {
	friend class MaterialShaderOutput;
public:
	MaterialShaderInput() = default;
	MaterialShaderInput(const MaterialShader2& parent);
	MaterialShaderInput(const MaterialShader2& parent, std::string name, std::string type, int index);

	void Link(MaterialShaderOutput* source);
	void Unlink();
	const MaterialShader2* GetParent() const;

	MaterialShaderOutput* GetLink() const override;

	const std::string& GetType() const;
	const std::string& GetName() const;

	std::string ToString() const override;

	const std::string name; // HLSL variable name of the port.
	const std::string type; // HLSL type of the port.
	const int index = 0; // Index in the HLSL function parameter list.
private:
	MaterialShaderOutput* m_link = nullptr;
	const MaterialShader2* m_parent = nullptr;
};


class MaterialShader2 : public ISerializableNode {
public:
	MaterialShader2(const ShaderManager* shaderManager);

	// Shaders
	virtual const std::string& GetShaderCode() const = 0;
	virtual size_t GetHash() const { return std::hash<std::string>()(GetShaderCode()); }

	void SetDisplayName(std::string name) { m_name = std::move(name); }
	const std::string& GetDisplayName() const override { return m_name; }

	std::string GetClassName() const override { return m_className; }

	// Ports
	size_t GetNumInputs() const override { return m_inputs.size(); }
	size_t GetNumOutputs() const override { return m_outputs.size(); }

	MaterialShaderInput* GetInput(size_t index) override { return &m_inputs[index]; }
	MaterialShaderOutput* GetOutput(size_t index) override { return &m_outputs[index]; }

	const MaterialShaderInput* GetInput(size_t index) const override { return &m_inputs[index]; }
	const MaterialShaderOutput* GetOutput(size_t index) const override { return &m_outputs[index]; }

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

protected:
	// HLSL string processing helpers
	static std::string RemoveComments(std::string code);
	static std::string GetFunctionSignature(const std::string& code, const std::string& functionName);
	static FunctionSignature DissectFunctionSignature(const std::string& signature);

	void SetClassName(std::string className) { m_className = std::move(className); }
protected:
	std::vector<MaterialShaderInput> m_inputs;
	std::vector<MaterialShaderOutput> m_outputs;
	const ShaderManager* m_shaderManager;

private:
	std::string m_name;
	std::string m_className;
};



class MaterialShaderEquation2 : public MaterialShader2 {
public:
	MaterialShaderEquation2(const ShaderManager* shaderManager) : MaterialShader2(shaderManager) {}

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
	MaterialShaderGraph2(const ShaderManager* shaderManager) : MaterialShader2(shaderManager) {}

	const std::string& GetShaderCode() const override;

	void SetGraph(std::vector<std::unique_ptr<MaterialShader2>> nodes);

private:
	// Creates one graph node per shader node, adds arc in graph for any two shader nodes which have their ports linked together.
	static void CalculateDependencyGraph(const std::vector<std::unique_ptr<MaterialShader2>>& nodes,
										 lemon::ListDigraph& depGraph,
										 lemon::ListDigraph::NodeMap<MaterialShader2*>& depMap);

	// Gets the list of unlinked (free) ports of the node graph.
	static auto GetUnlinkedPorts(const std::vector<std::unique_ptr<MaterialShader2>>& nodes)
		->std::tuple<std::vector<MaterialShaderInput*>, std::vector<MaterialShaderOutput*>>;


	// Creates the parameter list string of the main function using the list of unlinked ports.
	// Parameters are renamed by adding the name of the parent node.
	// Throws an exception if two parameters have the same name, because that's invalid in HLSL.
	static std::string CreateParameterString(const std::vector<MaterialShaderInput*>& inputs,
											 const std::vector<MaterialShaderOutput*>& outputs);

	// Creates input and outputs ports.
	void CreatePorts(const std::vector<MaterialShaderInput*>& inputs,
					 const std::vector<MaterialShaderOutput*>& outputs);
private:
	std::vector<std::unique_ptr<MaterialShader2>> m_nodes;
	std::string m_sourceCode;
};


} // namespace inl::gxeng