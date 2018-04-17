#include "Material.hpp"
#include <stack>



namespace inl::gxeng {


//------------------------------------------------------------------------------
// MaterialShader
//------------------------------------------------------------------------------

std::string MaterialShader::LoadShaderSource(std::string name) const {
	std::string code = m_shaderManager->LoadShaderSource(name);
	return code;
}

std::vector<MaterialShaderParameter> MaterialShader::GetShaderParameters() const {
	std::vector<MaterialShaderParameter> params;
	eMaterialShaderParamType ret;
	ExtractShaderParameters(GetShaderCode(), "main", ret, params);
	return params;
}

eMaterialShaderParamType MaterialShader::GetShaderOutputType() const {
	std::vector<MaterialShaderParameter> params;
	eMaterialShaderParamType ret;
	ExtractShaderParameters(GetShaderCode(), "main", ret, params);
	return ret;
}


void MaterialShader::SetName(std::string name) {
	if (name.find("__") != name.npos) {
		throw InvalidArgumentException("Name cannot contain double-underscores.");
	}
	m_name = name;
}

const std::string& MaterialShader::GetName() const {
	return m_name;
}



//------------------------------------------------------------------------------
// ShaderEquation
//------------------------------------------------------------------------------

std::string MaterialShaderEquation::GetShaderCode() const {
	return m_source;
}

void MaterialShaderEquation::SetSourceName(const std::string& name) {
	m_source = LoadShaderSource(name);
}

void MaterialShaderEquation::SetSourceCode(const std::string& code) {
	m_source = code;
}



//------------------------------------------------------------------------------
// ShaderGraph
//------------------------------------------------------------------------------


MaterialShaderGraph::MaterialShaderGraph(ShaderManager* shaderManager)
	: MaterialShader(shaderManager)
{}

std::string MaterialShaderGraph::GetShaderCode() const {
	return m_source;
}


void MaterialShaderGraph::AssembleShaderCode() {
	std::vector<ShaderNode> shaderNodes(m_nodes.size());
	std::vector<std::vector<MaterialShaderParameter>> shaderNodeParams(m_nodes.size());
	std::vector<eMaterialShaderParamType> shaderNodeReturns(m_nodes.size());
	std::vector<std::string> functions(m_nodes.size());

	// collect individual shader codes and set number of input params for each node
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		MaterialShader* shader = m_nodes[i].get();
		functions[i] = shader->GetShaderCode();
		ExtractShaderParameters(functions[i], "main", shaderNodeReturns[i], shaderNodeParams[i]);

		for (auto p : shaderNodeParams[i]) {
			if (p.type == eMaterialShaderParamType::UNKNOWN) {
				throw InvalidArgumentException("Parameter of unknown type.");
			}
		}

		shaderNodes[i].SetNumInputs(shaderNodeParams[i].size());
	}

	// link nodes together
	for (const auto& link : m_links) {
		ShaderNode& source = shaderNodes[link.sourceNode];
		ShaderNode& sink = shaderNodes[link.sinkNode];
		if (sink.GetNumInputs() <= link.sinkPort) {
			throw InvalidArgumentException("Invalid link: port does not have that many inputs.");
		}
		bool isLinked = source.GetOutput(0)->Link(sink.GetInput(link.sinkPort));
		if (!isLinked) {
			throw InvalidArgumentException("Invalid link: duplicate input to a single port.");
		}
	}

	// create output port LUT
	std::unordered_map<OutputPortBase*, size_t> outputLut;
	for (size_t i = 0; i < shaderNodes.size(); ++i) {
		outputLut.insert({ shaderNodes[i].GetOutput(0), i });
	}

	// get the sink node
	size_t sinkNodeIdx = -1;
	for (size_t i = 0; i < shaderNodes.size(); ++i) {
		if (shaderNodes[i].GetOutput(0)->begin() == shaderNodes[i].GetOutput(0)->end()) {
			if (sinkNodeIdx != -1) {
				throw InvalidArgumentException("Invalid graph: multiple sink nodes.");
			}
			sinkNodeIdx = i;
		}
	}
	if (sinkNodeIdx == -1) {
		throw InvalidArgumentException("Invalid graph: no sink nodes, contains circle.");
	}

	// run backwards DFS from sink node
	// - get topological order
	// - get list of free params
	struct FreeParam {
		size_t node, input;
		std::string name;
	};
	std::vector<size_t> topologicalOrder;
	std::vector<bool> visited(shaderNodes.size(), false);
	std::vector<FreeParam> freeParams;
	auto VisitNode = [&](size_t node, auto& self) {
		if (visited[node]) {
			return;
		}
		visited[node] = true;

		for (int i = 0; i < shaderNodes[node].GetNumInputs(); ++i) {
			auto* input = shaderNodes[node].GetInput(i);
			if (input->GetLink() != nullptr) {
				auto* linkOutput = input->GetLink();
				size_t linkNode = outputLut[linkOutput];
				self(linkNode, self);
			}
			else {
				std::stringstream ss;
				ss << m_nodes[node]->GetName() << "__" << shaderNodeParams[node][i].name;
				static_cast<InputPort<std::string>*>(input)->Set(ss.str());
				freeParams.push_back({ node, (size_t)i, ss.str() });
			}
		}

		std::stringstream ss;
		ss << "main_" << topologicalOrder.size();
		shaderNodes[node].SetFunctionName(ss.str());
		functions[node] = std::regex_replace(functions[node], std::regex("main"), ss.str());
		topologicalOrder.push_back(node);
		shaderNodes[node].SetFunctionReturn(GetParameterString(shaderNodeReturns[node]));
	};

	// attach final input port to sink node, and update according to topological order
	InputPort<std::string> finalCodePort;
	shaderNodes[sinkNodeIdx].GetOutput(0)->Link(&finalCodePort);
	VisitNode(sinkNodeIdx, VisitNode);
	for (auto idx : topologicalOrder) {
		shaderNodes[idx].Update();
	}

	// assemble resulting code
	std::stringstream finalCode;
	// sub-functions
	for (auto node : topologicalOrder) {
		finalCode << functions[node] << "\n";
	}
	finalCode << "\n\n";
	// signature
	std::string returnType = GetParameterString(shaderNodeReturns[*--topologicalOrder.end()]);
	finalCode << returnType << " main(";
	bool firstParam = true;
	for (auto& p : freeParams) {
		if (!firstParam)
			finalCode << ", ";
		finalCode << GetParameterString(shaderNodeParams[p.node][p.input].type) << " ";
		finalCode << p.name;
		firstParam = false;
	}
	finalCode << ") {\n";
	finalCode << "\n";
	// preambles
	for (auto idx : topologicalOrder) {
		finalCode << "    " << shaderNodes[idx].GetPreamble() << "\n";
	}
	finalCode << "\n";
	// return statement
	finalCode << "return " << finalCodePort.Get() << ";\n";
	finalCode << "}\n";


	m_source = finalCode.str();
}

void MaterialShaderGraph::SetGraph(std::vector<std::unique_ptr<MaterialShader>> nodes, std::vector<Link> links) {
	m_nodes = std::move(nodes);
	m_links = std::move(links);

	AssembleShaderCode();
}



//------------------------------------------------------------------------------
// Code manipulation helper statics
//------------------------------------------------------------------------------


std::string MaterialShader::RemoveComments(std::string code) {
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


std::string MaterialShader::FindFunctionSignature(std::string code, const std::string& functionName) {
	// regex to match " functionName ( anything ) { " 
	std::regex signaturePattern(functionName + R"(\s*\(.*\)\s*\{)");

	// find matches
	std::smatch matches;
	if (!std::regex_search(code, matches, signaturePattern)) {
		throw InvalidArgumentException("No main function found in material shader.");
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
		throw InvalidArgumentException("Main function has no return type.");
	}

	// walk back from match's end until ')'
	intptr_t matchEndIdx = matches[0].second - code.begin();
	while (code[matchEndIdx] != ')') {
		matchEndIdx--;
	}
	++matchEndIdx;

	return code.substr(returnIdx, matchEndIdx - returnIdx);
}


void MaterialShader::SplitFunctionSignature(std::string signature, std::string& returnType, std::vector<std::pair<std::string, std::string>>& parameters) {
	// extract part between the parentheses
	size_t opening = signature.find_first_of('(');
	size_t closing = signature.find_first_of(')');
	std::stringstream paramString(signature.substr(opening + 1, closing - opening - 1));

	// split the parameters string to individual parameters, and process them
	std::string param;
	while (getline(paramString, param, ',')) {
		if (param.size() == 0) {
			throw InvalidArgumentException("Parameter of main has zero characters.");
		}
		// trim leading and trailing whitespaces
		intptr_t firstChar = 0;
		while (firstChar < (intptr_t)param.size() && isspace(param[firstChar])) {
			++firstChar;
		}
		intptr_t lastChar = param.size() - 1;
		while (lastChar >= firstChar && isspace(param[lastChar])) {
			--lastChar;
		}
		if (firstChar >= lastChar) {
			throw InvalidArgumentException("Parameter of main has no type specifier or declaration name.");
		}
		param = param.substr(firstChar, lastChar-firstChar+1);

		// split param to type and name
		firstChar = 0;
		while (!isspace(param[firstChar])) {
			++firstChar;
		}
		lastChar = param.size() - 1;
		while (!isspace(param[lastChar])) {
			--lastChar;
		}

		parameters.push_back({ param.substr(0, firstChar), param.substr(lastChar + 1, param.npos) });
	}

	// get return type
	intptr_t lastChar = 0;
	while (!isspace(signature[lastChar])) {
		++lastChar;
	}
	returnType = signature.substr(0, lastChar);
}


std::string MaterialShader::GetParameterString(eMaterialShaderParamType type) {
	switch (type) {
		case eMaterialShaderParamType::COLOR: return "float4";
		case eMaterialShaderParamType::BITMAP_COLOR_2D: return "MapColor2D";
		case eMaterialShaderParamType::BITMAP_VALUE_2D: return "MapValue2D";
		case eMaterialShaderParamType::UNKNOWN: return "anyad";
		case eMaterialShaderParamType::VALUE: return "float";
		default: return "anyad";
	}
}
eMaterialShaderParamType MaterialShader::GetParameterType(std::string typeString) {
	if (typeString == "float4") {
		return eMaterialShaderParamType::COLOR;
	}
	else if (typeString == "float") {
		return eMaterialShaderParamType::VALUE;
	}
	else if (typeString == "MapColor2D") {
		return eMaterialShaderParamType::BITMAP_COLOR_2D;
	}
	else if (typeString == "MapValue2D") {
		return eMaterialShaderParamType::BITMAP_VALUE_2D;
	}
	else {
		return eMaterialShaderParamType::UNKNOWN;
	}
}


void MaterialShader::ExtractShaderParameters(std::string code, const std::string& functionName, eMaterialShaderParamType& returnType, std::vector<MaterialShaderParameter>& parameters) {
	code = RemoveComments(code);
	std::string signature = FindFunctionSignature(code, functionName);

	std::string returnTypeStr;
	std::vector<std::pair<std::string, std::string>> paramList;
	SplitFunctionSignature(signature, returnTypeStr, paramList);

	returnType = GetParameterType(returnTypeStr);

	// collect results
	std::vector<MaterialShaderParameter> params;
	for (const auto& p : paramList) {
		MaterialShaderParameter param;
		param.type = GetParameterType(p.first);
		param.name = p.second;
		params.push_back(param);
	}

	parameters = std::move(params);
}



//------------------------------------------------------------------------------
// ShaderNode
//------------------------------------------------------------------------------

size_t MaterialShaderGraph::ShaderNode::GetNumInputs() const {
	return m_inputs.size();
}

InputPortBase* MaterialShaderGraph::ShaderNode::GetInput(size_t index) {
	return &m_inputs[index];
}

const InputPortBase* MaterialShaderGraph::ShaderNode::GetInput(size_t index) const {
	return &m_inputs[index];
}


void MaterialShaderGraph::ShaderNode::Update() {
	std::string resultName = m_functionName + "_result";
	m_preamble = m_returnType + " " + resultName + " = " + m_functionName;
	m_preamble += '(';
	for (const auto& input : m_inputs) {
		m_preamble += input.Get() + ",";
	}
	m_preamble[m_preamble.size() - 1] = ')';
	m_preamble += ';';

	GetOutput<0>().Set(resultName);
}

void MaterialShaderGraph::ShaderNode::Notify(InputPortBase* sender) {
	return; // nothing to do here
}

void MaterialShaderGraph::ShaderNode::SetFunctionName(std::string functionName) {
	m_functionName = functionName;
}

void MaterialShaderGraph::ShaderNode::SetFunctionReturn(std::string returnType) {
	m_returnType = returnType;
}

void MaterialShaderGraph::ShaderNode::SetNumInputs(size_t count) {
	m_inputs.resize(count);
}

std::string MaterialShaderGraph::ShaderNode::GetPreamble() const {
	return m_preamble;
}





//------------------------------------------------------------------------------
// Material
//------------------------------------------------------------------------------

Material::Parameter::Parameter() {
	m_type = eMaterialShaderParamType::UNKNOWN;
}
Material::Parameter::Parameter(eMaterialShaderParamType type) {
	m_type = type;
}


Material::Parameter& Material::Parameter::operator=(Image* image) {
	if (m_type != eMaterialShaderParamType::BITMAP_COLOR_2D && m_type != eMaterialShaderParamType::BITMAP_VALUE_2D) {
		throw InvalidArgumentException("This parameter is not an image.");
	}

	m_data.image = image;
	return *this;
}

Material::Parameter& Material::Parameter::operator=(Vec4 color) {
	if (m_type != eMaterialShaderParamType::COLOR) {
		throw InvalidArgumentException("This parameter is not a color.");
	}

	m_data.color = color;
	return *this;
}

Material::Parameter& Material::Parameter::operator=(float value) {
	if (m_type != eMaterialShaderParamType::VALUE) {
		throw InvalidArgumentException("This parameter is not a value.");
	}

	m_data.value = value;
	return *this;
}


eMaterialShaderParamType Material::Parameter::GetType() const {
	return m_type;
}


Material::Parameter::operator Image*() const {
	if (m_type != eMaterialShaderParamType::BITMAP_COLOR_2D && m_type != eMaterialShaderParamType::BITMAP_VALUE_2D) {
		throw InvalidArgumentException("This parameter is not an image.");
	}
	return m_data.image;
}

Material::Parameter::operator Vec4() const {
	if (m_type != eMaterialShaderParamType::COLOR) {
		throw InvalidArgumentException("This parameter is not a color.");
	}
	return m_data.color;
}

Material::Parameter::operator float() const {
	if (m_type != eMaterialShaderParamType::VALUE) {
		throw InvalidArgumentException("This parameter is not a value.");
	}
	return m_data.value;
}



void Material::SetShader(MaterialShader* shader) {
	m_shader = shader;
	auto params = m_shader->GetShaderParameters();
	m_parameters.clear();
	m_paramNameMap.clear();
	for (auto p : params) {
		m_parameters.push_back(Parameter{ p.type });
		m_paramNameMap.insert({ p.name, m_parameters.size() - 1 });
	}
}

size_t Material::GetParameterCount() const {
	return m_parameters.size();
}

Material::Parameter& Material::operator[](size_t index) {
	assert(index < m_parameters.size());
	return m_parameters[index];
}

const Material::Parameter& Material::operator[](size_t index) const {
	assert(index < m_parameters.size());
	return m_parameters[index];
}

Material::Parameter& Material::operator[](const std::string& name) {
	auto it = m_paramNameMap.find(name);
	assert(it != m_paramNameMap.end());
	return (*this)[it->second];
}

const Material::Parameter& Material::operator[](const std::string& name) const {
	auto it = m_paramNameMap.find(name);
	assert(it != m_paramNameMap.end());
	return (*this)[it->second];
}


} // namespace inl::gxeng