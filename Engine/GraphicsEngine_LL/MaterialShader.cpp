#include "MaterialShader.hpp"

#include "ShaderManager.hpp"

#include <regex>


namespace inl::gxeng {


MaterialShader2::MaterialShader2(ShaderManager* shaderManager)
	: m_shaderManager(shaderManager)
{}


std::string MaterialShader2::RemoveComments(std::string code) {
	std::regex singleLineRegex("//\n");
	std::regex multilineRegex(R"(/\*.*\*/)");

	code = std::regex_replace(code, singleLineRegex, "\r\n");
	code = std::regex_replace(code, multilineRegex, "");
	return code;
}

std::string MaterialShader2::GetFunctionSignature(const std::string& code, const std::string& functionName) {
	// Search for the signature as "FunctionName ( ??? ) {".
	std::regex signatureRegex(functionName + R"(\s*\(.*\)\s*\{)");

	std::smatch matches;
	std::regex_search(code, matches, signatureRegex);

	if (matches.empty()) {
		throw InvalidArgumentException("Code piece does not contain function with provided name.");
	}


}

FunctionSignature MaterialShader2::DissectFunctionSignature(const std::string& signature) {
	return {};
}



} // namespace inl::gxeng