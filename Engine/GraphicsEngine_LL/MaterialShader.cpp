#include "MaterialShader.hpp"

#include "ShaderManager.hpp"
#include <BaseLibrary/StringUtil.hpp>

#include <regex>
#include <utility>

namespace inl::gxeng {


//------------------------------------------------------------------------------
// Material shader IO ports
//------------------------------------------------------------------------------

MaterialShaderInput::MaterialShaderInput(const MaterialShader2& parent)
	: m_parent(&parent)
{}

MaterialShaderInput::MaterialShaderInput(const MaterialShader2& parent, std::string name, std::string type, int index)
	: m_parent(&parent), name(std::move(name)), type(std::move(type)), index(index)
{}

void MaterialShaderInput::Link(MaterialShaderOutput& source) {
	source.Link(*this);
}

void MaterialShaderInput::Unlink() {
	if (m_link) {
		m_link->Unlink(*this);
	}
}

const MaterialShader2* MaterialShaderInput::GetParent() const {
	return m_parent;
}

const MaterialShaderOutput* MaterialShaderInput::GetLink() const {
	return m_link;
}


MaterialShaderOutput::MaterialShaderOutput(const MaterialShader2& parent)
	: m_parent(&parent)
{}

MaterialShaderOutput::MaterialShaderOutput(const MaterialShader2& parent, std::string name, std::string type, int index)
	: m_parent(&parent), name(std::move(name)), type(std::move(type)), index(index)
{}


void MaterialShaderOutput::Link(MaterialShaderInput& target) {
	if (target.m_link) {
		target.Unlink();
	}
	m_links.push_back(&target);
	target.m_link = this;
}

void MaterialShaderOutput::UnlinkAll() {
	for (auto& target : m_links) {
		target->m_link = nullptr;
	}
	m_links.clear();
}

void MaterialShaderOutput::Unlink(MaterialShaderInput& target) {
	auto it = m_links.begin();
	for (; it != m_links.end(); ++it) {
		if (*it == &target) {
			break;
		}
	}
	(*it)->m_link = nullptr;
	m_links.erase(it);
}

const MaterialShader2* MaterialShaderOutput::GetParent() const {
	return m_parent;
}

const std::vector<MaterialShaderInput*>& MaterialShaderOutput::GetLinks() const {
	return m_links;
}


//------------------------------------------------------------------------------
// Material shader base class
//------------------------------------------------------------------------------

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
	std::regex signatureRegex(R"(\s)" + functionName + R"(\s*\(.*\)\s*\{)");

	std::smatch matches;
	std::regex_search(code, matches, signatureRegex);

	if (matches.empty()) {
		throw InvalidArgumentException("Code piece does not contain function with provided name.");
	}

	auto first = matches[0].first;
	auto last = matches[0].second;

	// Remove spaces from between return type and function name.
	do {
		if (first == code.begin()) {
			throw InvalidArgumentException("HLSL code has syntax error.", "Function signature found, but it has no return type.");
		}
		--first;
	} while (isspace(*first));

	// Go backwards until the beginning of the return type.
	while (first != code.begin() && !isspace(*first)) {
		--first;
	}

	return std::string(first, last-1);
}


FunctionParameter DissectFunctionParameterDeclaration(std::string_view declaration) {
	FunctionParameter desc;

	// Process parameter declaration.
	auto declarationTokens = Tokenize(declaration, " \t\v\r\n", true);
	if (declarationTokens.size() == 3) {
		if (declarationTokens[0] == "in") {
			desc.out = false;
		}
		else if (declarationTokens[0] == "out") {
			desc.out = true;
		}
		else {
			throw InvalidArgumentException("HLSL semantic error.", "Function parameters cannot be uniform or inout, only in and out.");
		}
		desc.type = declarationTokens[1];
		desc.name = declarationTokens[2];
	}
	else if (declarationTokens.size() == 2) {
		desc.out = false;
		desc.type = declarationTokens[0];
		desc.name = declarationTokens[1];
	}
	else {
		throw InvalidArgumentException("HLSL syntax error.", "Funtion parameter signature must look like \"[(in|out)] type name\"");
	}

	return desc;
}

FunctionParameter DissectFunctionParameter(std::string_view parameter) {
	// Syntax of a function parameter:
	// [(in|out|inout)] type name [= value]

	// Cut default parameter, if any.
	auto declarationAndValue = Tokenize(parameter, "=");
	if (!declarationAndValue.empty()) {
		throw InvalidArgumentException("HLSL syntax error.", "Function has empty parameter.");
	}
	assert(declarationAndValue.size() <= 2);

	std::string_view declarationString = declarationAndValue.front();

	// Parameter must not have semantics.
	if (declarationString.find(':') != declarationString.npos) {
		throw InvalidArgumentException("HLSL semantic error.", "Function parameters cannot have semantics.");
	}

	// Get declaration details.
	FunctionParameter declaration = DissectFunctionParameterDeclaration(declarationString);

	return declaration;
}


FunctionSignature MaterialShader2::DissectFunctionSignature(const std::string& signatureString) {
	std::string_view returnType = NextToken(signatureString, " \t\v\r\n").value_or("");
	assert(!returnType.empty());

	size_t paramsFirst = signatureString.find_first_of('(');
	size_t paramsLast = signatureString.find_last_of(')');
	assert(paramsFirst != signatureString.npos && paramsLast != signatureString.npos);
	assert(paramsFirst < paramsLast);

	paramsFirst = paramsFirst + 1; // opening parenthesis not needed

	std::string_view paramString(signatureString.c_str() + paramsFirst, paramsLast - paramsFirst);
	auto parameters = Tokenize(paramString, ",", false);

	FunctionSignature signature;
	signature.returnType = returnType;
	for (auto p : parameters) {
		signature.parameters.push_back(DissectFunctionParameter(p));
	}

	return signature;
}


//------------------------------------------------------------------------------
// Material shader equation
//------------------------------------------------------------------------------

const std::string& MaterialShaderEquation2::GetShaderCode() const {
	return m_sourceCode;
}


void MaterialShaderEquation2::SetSourceFile(const std::string& name) {
	m_sourceCode = m_shaderManager->LoadShaderSource(name);
}


void MaterialShaderEquation2::SetSourceCode(std::string code) {
	m_sourceCode = std::move(code);
}


void MaterialShaderEquation2::CreatePorts(const std::string& code) {
	std::string signatureString = GetFunctionSignature(code, "main");
	FunctionSignature signature = DissectFunctionSignature(signatureString);
	
	std::vector<MaterialShaderInput> inputs;
	std::vector<MaterialShaderOutput> outputs;

	if (signature.returnType != "void") {
		MaterialShaderOutput returnOutput(*this, "", signature.returnType, -1);
		outputs.push_back(returnOutput);
	}

	int index = 0;
	for (auto& parameter : signature.parameters) {
		if (parameter.out) {
			MaterialShaderOutput output(*this, parameter.name, parameter.type, index);
			outputs.push_back(output);
		}
		else {
			MaterialShaderInput input(*this, parameter.name, parameter.type, index);
			inputs.push_back(input);
		}

		++index;
	}

	m_inputs = std::move(inputs);
	m_outputs = std::move(outputs);
}





//------------------------------------------------------------------------------
// Material shader graph
//------------------------------------------------------------------------------


const std::string& MaterialShaderGraph2::GetShaderCode() const {
	throw NotImplementedException();
}

void MaterialShaderGraph2::SetGraph(std::vector<std::unique_ptr<MaterialShader2>> nodes) {
	
}



} // namespace inl::gxeng