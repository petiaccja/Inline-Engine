#include "MaterialShader.hpp"

#include "ShaderManager.hpp"
#include <BaseLibrary/StringUtil.hpp>

#include <regex>
#include <utility>
#include <lemon/list_graph.h>
#include "BaseLibrary/Range.hpp"
#include <lemon/connectivity.h>

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
	if (declarationAndValue.empty()) {
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
	CreatePorts(m_sourceCode);
}


void MaterialShaderEquation2::SetSourceCode(std::string code) {
	m_sourceCode = std::move(code);
	CreatePorts(m_sourceCode);
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
	return m_sourceCode;
}

void MaterialShaderGraph2::SetGraph(std::vector<std::unique_ptr<MaterialShader2>> nodes) {
	using namespace lemon;

	// Algorithm:

	// 1. create dependency graph of nodes
	ListDigraph depGraph;
	ListDigraph::NodeMap<MaterialShader2*> depMap(depGraph);
	CalculateDependencyGraph(nodes, depGraph, depMap);


	// 2. find topological ordering
	ListDigraph::NodeMap<int> sortingMap(depGraph);
	std::unordered_map<const MaterialShader2*, int> indexMap;

	bool isDag = checkedTopologicalSort(depGraph, sortingMap);
	if (!isDag) {
		throw InvalidArgumentException("Provided shader nodes are linked in a circular way.");
	}
	std::vector<ListDigraph::Node> sortedNodes;
	for (ListDigraph::NodeIt node(depGraph); node != lemon::INVALID; ++node) {
		sortedNodes.push_back(node); // Collect nodes into vector for re-ordering.
		indexMap[depMap[node]] = sortingMap[node]; // Make LUT for nodePtr -> topIdx.
	}
	std::sort(sortedNodes.begin(), sortedNodes.end(), [&](auto lhs, auto rhs) {
		return sortingMap[lhs] < sortingMap[rhs];
	});


	// 3. wrap node source codes into namespaces using their topological order as namespace name/id
	std::vector<std::string> sourceCodes;
	int snIndex = 0;
	for (auto graphNode : sortedNodes) {
		std::stringstream ss;
		ss << "namespace " << "subnode" << snIndex++ << " {\n";
		ss << depMap[graphNode]->GetShaderCode();
		ss << "\n}\n\n\n";
		sourceCodes.push_back(ss.str());
	}


	// 4. concat all node codes into a large chunk of code
	std::string concatCode;
	for (auto& src : sourceCodes) {
		concatCode += src;
	}


	// 5. add main function at the end the code
	std::stringstream mainss;
	mainss << "void main(";

	auto[freeInputs, freeOutputs] = GetUnlinkedPorts(nodes);
	std::string paramstr = CreateParameterString(freeInputs, freeOutputs); // Checks duplicate param names & throws.
	mainss << paramstr;

	mainss << ") {\n\n";




	// 6. main calls subnode#::main functions in topological order, all parameters are stored and used for next call
	for (auto graphNode : sortedNodes) {
		auto node = depMap[graphNode];
		std::vector<std::string> arguments(node->GetNumInputs() + node->GetNumOutputs());

		// Process inputs.
		for (auto inputIdx : Range(node->GetNumInputs())) {
			MaterialShaderInput* port = node->GetInput(inputIdx);

			std::stringstream ss;

			// Argument is output of a previous node.
			if (port->GetLink()) {
				std::string sourcePortName = port->GetLink()->name;
				int sourceNodeIndex = indexMap[port->GetLink()->GetParent()];
				ss << "__node" << sourceNodeIndex << "_outparam_" << sourcePortName;
			}
			// Argument is a parameter of the main function.
			else {
				ss << port->GetParent()->GetDisplayName() << "_" << port->name;
			}

			arguments[port->index] = ss.str();
		}

		// Process outputs.
		std::stringstream declaress;
		bool hasReturn = false;
		for (auto outputIdx : Range(node->GetNumOutputs())) {
			MaterialShaderOutput* port = node->GetOutput(outputIdx);

			std::stringstream ss;

			int nodeIdx = indexMap[port->GetParent()];
			ss << "__node" << nodeIdx << "_outparam_" << port->name;
			declaress << port->type << " " << ss.str() << "; ";

			if (port->index < 0) {
				hasReturn = true;
				arguments.pop_back();
			}
			else {
				arguments[port->index] = ss.str();
			}
		}

		// Add call to main.
		int nodeIdx = indexMap[node];
		mainss << "\t" << declaress.str() << "\n\t";
		if (hasReturn) {
			mainss << "__node" << nodeIdx << "_outparam_" << " = ";
		}
		mainss << "subnode" << nodeIdx << "::main(";
		for (auto i : Range(arguments.size())) {
			mainss << (i != 0 ? ", " : "");
			mainss << arguments[i];
		}
		mainss << ");\n\n";
	}

	// 7. forward results of sub-mains to the output of main.
	for (const auto result : freeOutputs) {
		std::stringstream ss;
		const MaterialShader2* node = result->GetParent();
		int nodeIdx = indexMap[node];

		std::string mainParamName = node->GetDisplayName() + "_" + result->name;
		ss << "__node" << nodeIdx << "_outparam_" << result->name;
		std::string outParamName = ss.str();

		mainss << "\t" << mainParamName << " = " << outParamName << ";\n";
	}

	mainss << "}";

	// Note:
	// - main's parameters are the non-connected node inputs and outputs
	// - nodes with the same code should not be duplicated
	// - code duplication might further be helped by handling nested duplication (graph of shader graphs)

	// Create source code and ports.
	m_sourceCode = concatCode + mainss.str();
	CreatePorts(freeInputs, freeOutputs);
}


void MaterialShaderGraph2::CalculateDependencyGraph(const std::vector<std::unique_ptr<MaterialShader2>>& nodes,
													lemon::ListDigraph& depGraph,
													lemon::ListDigraph::NodeMap<MaterialShader2*>& depMap)
{
	using namespace lemon;

	struct NodeMap {
		ListDigraph::Node graphNode;
		size_t index;
	};
	std::unordered_map<const MaterialShader2*, NodeMap> nodeMap;

	for (auto i : Range(nodes.size())) {
		auto& node = nodes[i];
		ListDigraph::Node graphNode = depGraph.addNode();
		nodeMap[node.get()] = { graphNode, i };
		depMap[graphNode] = node.get();
	}

	for (auto destIdx : Range(nodes.size())) {
		auto destNode = nodes[destIdx].get();

		for (size_t inputPortIdx = 0; inputPortIdx < destNode->GetNumInputs(); ++inputPortIdx) {
			auto sourcePort = destNode->GetInput(inputPortIdx)->GetLink();
			auto sourceNode = sourcePort != nullptr ? sourcePort->GetParent() : nullptr;
			if (sourceNode != nullptr) {
				depGraph.addArc(nodeMap[sourceNode].graphNode, nodeMap[destNode].graphNode);
			}
		}
	}
}


auto MaterialShaderGraph2::GetUnlinkedPorts(const std::vector<std::unique_ptr<MaterialShader2>>& nodes)
->std::tuple<std::vector<MaterialShaderInput*>, std::vector<MaterialShaderOutput*>>
{
	std::vector<MaterialShaderInput*> inputs;
	std::vector<MaterialShaderOutput*> outputs;

	for (auto& node : nodes) {
		for (auto i : Range(node->GetNumInputs())) {
			if (node->GetInput(i)->GetLink() == nullptr) {
				inputs.push_back(node->GetInput(i));
			}
		}
		for (auto i : Range(node->GetNumOutputs())) {
			if (node->GetOutput(i)->GetLinks().empty()) {
				outputs.push_back(node->GetOutput(i));
			}
		}
	}

	return { inputs, outputs };
}


std::string MaterialShaderGraph2::CreateParameterString(const std::vector<MaterialShaderInput*>& inputs,
														const std::vector<MaterialShaderOutput*>& outputs)
{
	std::stringstream paramss;
	std::unordered_set<std::string> takenNames;

	bool first = true;
	for (auto& input : inputs) {
		paramss << (first ? "" : ", ");
		std::string name = input->GetParent()->GetDisplayName() + "_" + input->name;
		auto it = takenNames.insert(name);
		if (!it.second) {
			throw InvalidArgumentException("List of free ports have two nodes with the same name and same parent node name.");
		}
		paramss << input->type << " " << name;
		first = false;
	}
	for (auto& output : outputs) {
		paramss << (first ? "" : ", ");
		std::string name = output->GetParent()->GetDisplayName() + "_" + output->name;
		auto it = takenNames.insert(name);
		if (!it.second) {
			throw InvalidArgumentException("List of free ports have two nodes with the same name and same parent node name.");
		}
		paramss << "out " << output->type << " " << name;
		first = false;
	}

	return paramss.str();
}


void MaterialShaderGraph2::CreatePorts(const std::vector<MaterialShaderInput*>& inputs,
									   const std::vector<MaterialShaderOutput*>& outputs)
{
	m_inputs.clear();
	m_outputs.clear();

	int idx = 0;
	for (auto input : inputs) {
		std::string name = input->GetParent()->GetDisplayName() + "_" + input->name;

		MaterialShaderInput port{ *this, name, input->type, idx++ };
		m_inputs.push_back(port);
	}
	for (auto output : outputs) {
		std::string name = output->GetParent()->GetDisplayName() + "_" + output->name;

		MaterialShaderOutput port{ *this, name, output->type, idx++ };
		m_outputs.push_back(port);
	}
}


} // namespace inl::gxeng