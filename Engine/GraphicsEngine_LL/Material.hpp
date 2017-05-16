#pragma once

#include "ShaderManager.hpp"

#include <BaseLibrary/Graph_All.hpp>
#include <mathfu/mathfu_exc.hpp>

#include <regex>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string>

namespace inl::gxeng {


class Image;


enum class eMaterialShaderParamType {
	COLOR = 0,
	VALUE = 1,
	BITMAP_COLOR_2D = 2,
	BITMAP_VALUE_2D = 3,
	UNKNOWN = 1000,
};


struct MaterialShaderParameter {
	std::string name;
	eMaterialShaderParamType type;
};



class MaterialShader {
public:
	MaterialShader(ShaderManager* shaderManager) : m_shaderManager(shaderManager) {}
	virtual ~MaterialShader() {};
	virtual std::string GetShaderCode() const = 0;
	virtual std::vector<MaterialShaderParameter> GetShaderParameters() const;
	virtual eMaterialShaderParamType GetShaderOutputType() const;
	virtual size_t GetHash() const { return std::hash<std::string>()(GetShaderCode()); }

	void SetName(std::string name);
	const std::string& GetName() const;
protected:
	static std::string RemoveComments(std::string code);
	static std::string FindFunctionSignature(std::string code, const std::string& functionName);
	static void SplitFunctionSignature(std::string signature, std::string& returnType, std::vector<std::pair<std::string, std::string>>& parameters);
	static std::string GetParameterString(eMaterialShaderParamType type);
	static eMaterialShaderParamType GetParameterType(std::string typeString);
	static void ExtractShaderParameters(std::string code, const std::string& functionName, eMaterialShaderParamType& returnType, std::vector<MaterialShaderParameter>& parameters);
protected:
	std::string LoadShaderSource(std::string name) const;
private:
	ShaderManager* m_shaderManager;
	std::string m_name;
};


class MaterialShaderEquation : public MaterialShader {
public:
	MaterialShaderEquation(ShaderManager* shaderManager) : MaterialShader(shaderManager) {}

	std::string GetShaderCode() const override;

	void SetSourceName(const std::string& name);
	void SetSourceCode(const std::string& code);
private:
	std::string m_source;
};


class MaterialShaderGraph : public MaterialShader {
public:
	struct Link {
		int sourceNode;
		int sinkNode, sinkPort;
	};
private:
	class ShaderNode : virtual public exc::NodeBase, public exc::OutputPortConfig<std::string> {
	public:
		size_t GetNumInputs() const override;
		exc::InputPortBase* GetInput(size_t index) override;
		const exc::InputPortBase* GetInput(size_t index) const override;

		void Update() override;
		void Notify(exc::InputPortBase* sender) override;

		void SetFunctionName(std::string functionName);
		void SetFunctionReturn(std::string returnType);
		void SetNumInputs(size_t count);
		std::string GetPreamble() const;
	private:
		std::vector<exc::InputPort<std::string>> m_inputs;
		std::string m_functionName;
		std::string m_returnType;
		std::string m_preamble;
	};

public:
	MaterialShaderGraph(ShaderManager* shaderManager);

	std::string GetShaderCode() const override;

	void SetGraph(std::vector<std::unique_ptr<MaterialShader>> nodes, std::vector<Link> links);
protected:
	void AssembleShaderCode();
private:
	std::vector<std::unique_ptr<MaterialShader>> m_nodes;
	std::vector<Link> m_links;
	std::string m_source;
};


class Material {
public:
	class Parameter {
	public:
		Parameter();
		Parameter(eMaterialShaderParamType type);

		Parameter& operator=(Image*);
		Parameter& operator=(mathfu::Vector4f);
		Parameter& operator=(float);

		eMaterialShaderParamType GetType() const;
		operator Image*() const;
		operator mathfu::Vector4f() const;
		operator float() const;
	private:
		eMaterialShaderParamType m_type;
		union Data {
			Data() { memset(this, 0, sizeof(*this)); }
			Data(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); }
			Data& operator=(const Data& rhs) { memcpy(this, &rhs, sizeof(*this)); return *this; }
			Image* image;
			mathfu::Vector4f color;
			float value;
		} m_data;
	};

public:
	void SetShader(MaterialShader* shader);
	MaterialShader* GetShader() const { return m_shader; }
	size_t GetParameterCount() const;

	Parameter& operator[](size_t index);
	const Parameter& operator[](size_t index) const;

	Parameter& operator[](const std::string& name);
	const Parameter& operator[](const std::string& name) const;
private:
	std::vector<Parameter> m_parameters;
	MaterialShader* m_shader = nullptr;
	std::unordered_map<std::string, size_t> m_paramNameMap; // maps parameter names to indices
};



// TEST IMPLEMENTATION
inline std::string MaterialGenPixelShader(const MaterialShader& shader) {
	return "this function is no longer used";
}




} // namespace inl::gxeng