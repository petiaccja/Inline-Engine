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
	// get material shading function's HLSL code
	std::vector<MaterialShaderParameter> params;
	std::string shadingFunction;

	params = shader.GetShaderParameters();
	shadingFunction = shader.GetShaderCode();

	// rename "main" to something else
	std::stringstream renameMain;
	std::regex renameRegex("main");
	std::regex_replace(std::ostreambuf_iterator<char>(renameMain), shadingFunction.begin(), shadingFunction.end(), renameRegex, "mtl_shader");
	shadingFunction = renameMain.str();

	// structures
	std::string structures =
		"struct PsInput {\n"
		"    float4 ndcPos : SV_POSITION;\n"
		"    float3 worldNormal : NO;\n"
		"    float2 texCoord : TEX_COORD;\n"
		"};\n\n"
		"struct MapColor2D {\n"
		"    Texture2DArray<float4> tex;\n"
		"    SamplerState samp;\n"
		"};\n\n"
		"struct MapValue2D {\n"
		"    Texture2DArray<float> tex;\n"
		"    SamplerState samp;\n"
		"};\n";

	// globals
	std::string globals =
		"static float3 g_lightDir;\n"
		"static float3 g_lightColor;\n"
		"static float3 g_normal;\n"
		"static float3 g_tex0;\n";

	// add constant buffer, textures and samplers according to shader parameters
	std::stringstream lightConstantBuffer;
	std::stringstream mtlConstantBuffer;
	std::stringstream textures;

	lightConstantBuffer << "struct LightConstants {\n"
		"    float3 direction;\n"
		"    float3 color;\n"
		"};\n";
	lightConstantBuffer << "ConstantBuffer<LightConstants> lightCb: register(b100); \n";

	mtlConstantBuffer << "struct MtlConstants { \n";
	int numMtlConstants = 0;
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i].type) {
			case eMaterialShaderParamType::COLOR:
			{
				mtlConstantBuffer << "    float4 param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				mtlConstantBuffer << "    float param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				textures << "Texture2DArray<float4> tex" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				textures << "Texture2DArray<float> tex" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
		}
	}
	mtlConstantBuffer << "};\n";
	mtlConstantBuffer << "ConstantBuffer<MtlConstants> mtlCb: register(b200); \n";
	if (numMtlConstants == 0) {
		mtlConstantBuffer = std::stringstream();
	}

	// main function
	std::stringstream PSMain;

	PSMain << "float4 PSMain(PsInput psInput) : SV_TARGET {\n";
	PSMain << "    g_lightDir = lightCb.direction;\n";
	PSMain << "    g_lightColor = lightCb.color;\n";
	PSMain << "    g_normal = psInput.worldNormal;\n";
	PSMain << "    g_tex0 = float3(psInput.texCoord, 0.0f);\n";
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i].type) {
			case eMaterialShaderParamType::COLOR:
			{
				PSMain << "    float4 input" << i << "; \n";
				PSMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				PSMain << "    float input" << i << "; \n";
				PSMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				PSMain << "    MapColor2D input" << i << "; \n";
				PSMain << "    input" << i << ".tex = tex" << i << "; \n";
				PSMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				PSMain << "    MapValue2D input" << i << "; \n";
				PSMain << "    input" << i << ".tex = tex" << i << "; \n";
				PSMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
		}
	}
	PSMain << "    return mtl_shader(";
	for (intptr_t i = 0; i < (intptr_t)params.size() - 1; ++i) {
		PSMain << "input" << i << ", ";
	}
	if (params.size() > 0) {
		PSMain << "input" << params.size() - 1;
	}
	PSMain << "); \n} \n";

	return
		structures
		+ "\n//-------------------------------------\n\n"
		+ globals
		+ "\n//-------------------------------------\n\n"
		+ lightConstantBuffer.str()
		+ mtlConstantBuffer.str()
		+ "\n//-------------------------------------\n\n"
		+ textures.str()
		+ "\n//-------------------------------------\n\n"
		+ shadingFunction
		+ "\n//-------------------------------------\n\n"
		+ PSMain.str();
}




} // namespace inl::gxeng