#pragma once

#include "ShaderManager.hpp"

#include <BaseLibrary/Graph_All.hpp>

#include <regex>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string>


namespace inl::gxeng {


//struct MaterialShaderParameter {
//	enum eType {
//		COLOR = 0,
//		VALUE = 1,
//		MAP_COLOR_2D = 2,
//		MAP_VALUE_2D = 3,
//		UNKNOWN = 1000,
//	};
//
//	eType type;
//	int index;
//};

enum class eMaterialShaderParamType {
	COLOR = 0,
	VALUE = 1,
	MAP_COLOR_2D = 2,
	MAP_VALUE_2D = 3,
	UNKNOWN = 1000,
};



class MaterialShader {
public:
	virtual ~MaterialShader() {};
	virtual std::string GetShaderCode(ShaderManager& shaderManager) const = 0;
};


class MaterialShaderEquation : public MaterialShader {
public:
	std::string GetShaderCode(ShaderManager& shaderManager) const override;

	void SetSourceName(const std::string& name);
	void SetSourceCode(const std::string& code);
private:
	bool m_isCode = true;
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
	std::string GetShaderCode(ShaderManager& shaderManager) const override;

	void SetGraph(std::vector<std::unique_ptr<MaterialShader>> nodes, std::vector<Link> links);
private:
	std::vector<std::unique_ptr<MaterialShader>> m_nodes;
	std::vector<Link> m_links;
};



inline std::string RemoveComments(std::string code) {
	std::stringstream ss(code);

	// remove single line comments by trimming commented-out tails, line-by-line
	std::string noSingleLine;
	std::string singleLine;
	while (std::getline(ss, singleLine)) {
		size_t idx = singleLine.find_first_of("//");
		if (idx != singleLine.npos) {
			singleLine = singleLine.substr(0, idx - 1);
		}
		noSingleLine += singleLine;
	}

	// remove multiline comments by replacing regex template " /*anything*/ " to empty string
	std::stringstream noComment;
	std::regex multilinePattern(R"(/\*.*\*/)");
	std::regex_replace(std::ostreambuf_iterator<char>(noComment), noSingleLine.begin(), noSingleLine.end(), multilinePattern, "");

	std::string ret = noComment.str();
	return ret;
}


inline std::string FindFunctionSignature(std::string code, const std::string& functionName) {
	// regex to match " functionName ( anything ) { " 
	std::regex signaturePattern(functionName + R"(\s*\(.*\)\s*\{)");

	// find matches
	std::smatch matches;
	if (!std::regex_search(code, matches, signaturePattern)) {
		throw std::invalid_argument("No main function found in material shader.");
	}

	// march back towards return type over whitespaces
	intptr_t matchIdx = matches[0].first - code.cbegin();
	matchIdx -= 1;
	while (matchIdx >= 0 && isspace(code[matchIdx])) {
		--matchIdx;
	}

	// march back over return type's characters
	intptr_t returnIdx = matchIdx;
	while (returnIdx >= 0 && !isspace(code[returnIdx])) {
		--returnIdx;
	}
	returnIdx = std::max(intptr_t(0), returnIdx);

	// return type spans across range [returnIdx, matchIdx], it being empty range means failure
	if (matchIdx == returnIdx) {
		throw std::invalid_argument("Main function has no return type.");
	}

	// walk back from match's end until ')'
	intptr_t matchEndIdx = matches[0].second - code.begin();
	while (code[matchEndIdx] != ')') {
		matchEndIdx--;
	}

	return code.substr(returnIdx, matchEndIdx - returnIdx);
}

inline void SplitFunctionSignature(std::string signature, std::string& returnType, std::vector<std::pair<std::string, std::string>>& parameters) {
	// extract part between the parentheses
	size_t opening = signature.find_first_of('(');
	size_t closing = signature.find_first_of(')');
	std::stringstream paramString(signature.substr(opening + 1, closing - opening - 1));

	// split the parameters string to individual parameters, and process them
	std::string param;
	while (getline(paramString, param, ',')) {
		if (param.size() == 0) {
			throw std::invalid_argument("Parameter of main has zero characters.");
		}
		// trim leading and trailing whitespaces
		intptr_t firstChar = 0;
		while (firstChar < param.size() && isspace(param[firstChar])) {
			++firstChar;
		}
		intptr_t lastChar = param.size() - 1;
		while (lastChar >= firstChar && isspace(param[lastChar])) {
			--lastChar;
		}
		if (firstChar >= lastChar) {
			throw std::invalid_argument("Parameter of main has no type specifier or declaration name.");
		}
		param = param.substr(firstChar, lastChar);

		// split param to type and name
		firstChar = 0;
		while (!isspace(param[firstChar])) {
			++firstChar;
		}
		lastChar = param.size() - 1;
		while (!isspace(param[lastChar])) {
			--lastChar;
		}

		parameters.push_back({ param.substr(0, firstChar), param.substr(lastChar+1, param.npos) });
	}

	// get return type
	intptr_t lastChar = 0;
	while (!isspace(signature[lastChar])) {
		++lastChar;
	}
	returnType = signature.substr(0, lastChar);
}

inline std::string GetParameterString(eMaterialShaderParamType type) {
	switch (type) {
		case eMaterialShaderParamType::COLOR: return "float4";
		case eMaterialShaderParamType::MAP_COLOR_2D: return "MapColor2D";
		case eMaterialShaderParamType::MAP_VALUE_2D: return "MapValue2D";
		case eMaterialShaderParamType::UNKNOWN: return "anyád";
		case eMaterialShaderParamType::VALUE: return "float";
	}
}
inline eMaterialShaderParamType GetParameterType(std::string typeString) {
	if (typeString == "float4") {
		return eMaterialShaderParamType::COLOR;
	}
	else if (typeString == "float") {
		return eMaterialShaderParamType::VALUE;
	}
	else if (typeString == "MapColor2D") {
		return eMaterialShaderParamType::MAP_COLOR_2D;
	}
	else if (typeString == "MapValue2D") {
		return eMaterialShaderParamType::MAP_VALUE_2D;
	}
	else {
		return eMaterialShaderParamType::UNKNOWN;
	}
}

inline void ExtractShaderParameters(std::string code, eMaterialShaderParamType& returnType, std::vector<eMaterialShaderParamType>& parameters) {
	code = RemoveComments(code);
	std::string signature = FindFunctionSignature(code, "main");

	std::string returnTypeStr;
	std::vector<std::pair<std::string, std::string>> paramList;
	SplitFunctionSignature(signature, returnTypeStr, paramList);

	returnType = GetParameterType(returnTypeStr);

	// collect results
	std::vector<eMaterialShaderParamType> params;
	for (const auto& p : paramList) {
		eMaterialShaderParamType type = GetParameterType(p.first);
		params.push_back(type);
	}

	parameters = std::move(params);
}




// TEST IMPLEMENTATION
inline std::string MaterialGenPixelShader(std::string shadingFunction) {
	// get material shading function's HLSL code
	eMaterialShaderParamType returnType;
	std::vector<eMaterialShaderParamType> params;
	ExtractShaderParameters(shadingFunction, returnType, params);

	// rename "main" to something else
	std::stringstream renameMain;
	std::regex renameRegex("main");
	std::regex_replace(std::ostreambuf_iterator<char>(renameMain), shadingFunction.begin(), shadingFunction.end(), renameRegex, "mtl_shader");
	shadingFunction = renameMain.str();

	// add constant buffer, textures and samplers according to shader parameters
	std::stringstream constantBuffer;
	size_t constantBufferSize;
	std::stringstream textures;

	constantBuffer << "struct CB { \n";
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i]) {
			case eMaterialShaderParamType::COLOR: {
				constantBuffer << "    float4 param" << i << "; \n";
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				constantBuffer << "    float param" << i << "; \n";
				break;
			}
			case eMaterialShaderParamType::MAP_COLOR_2D: {
				textures << "Texture2D<float4> map" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
			case eMaterialShaderParamType::MAP_VALUE_2D: {
				textures << "Texture2D<float> map" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
		}
	}
	constantBuffer << "} \n";
	constantBuffer << "ConstantBuffer<CB> : register(b0); \n";

	// main function
	std::stringstream PSMain;

	PSMain << "float4 PSMain() : COLOR0 {   \n";
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i]) {
			case eMaterialShaderParamType::COLOR: {
				PSMain << "    float4 input" << i << "; \n";
				PSMain << "    input" << i << " = cb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				PSMain << "    float input" << i << "; \n";
				PSMain << "    input" << i << " = cb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::MAP_COLOR_2D: {
				PSMain << "    MapColor2D input" << i << "; \n";
				PSMain << "    input" << i << ".map = map" << i << "; \n";
				PSMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::MAP_VALUE_2D: {
				PSMain << "    MapValue2D input" << i << "; \n";
				PSMain << "    input" << i << ".map = map" << i << "; \n";
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

	return constantBuffer.str() + "\n" + textures.str() + "\n" + shadingFunction + "\n\n" + PSMain.str();
}




} // namespace inl::gxeng