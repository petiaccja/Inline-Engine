#include "Material.hpp"
#include <stack>



namespace inl::gxeng {


//------------------------------------------------------------------------------
// ShaderEquation
//------------------------------------------------------------------------------

std::string MaterialShaderEquation::GetShaderCode(ShaderManager& shaderManager) const {
	if (m_isCode) {
		return m_source;
	}
	else {
		// shaderManager.loadSource(m_source);
	}
}

void MaterialShaderEquation::SetSourceName(const std::string& name) {
	m_isCode = false;
	m_source = name;
}

void MaterialShaderEquation::SetSourceCode(const std::string& code) {
	m_isCode = true;
	m_source = code;
}



//------------------------------------------------------------------------------
// ShaderGraph
//------------------------------------------------------------------------------

std::string MaterialShaderGraph::GetShaderCode(ShaderManager& shaderManager) const {
	std::vector<ShaderNode> shaderNodes(m_nodes.size());
	std::vector<std::vector<eMaterialShaderParamType>> shaderNodeParams(m_nodes.size());
	std::vector<eMaterialShaderParamType> shaderNodeReturns(m_nodes.size());
	std::vector<std::string> functions(m_nodes.size());

	// collect individual shader codes and set number of input params for each node
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		MaterialShader* shader = m_nodes[i].get();
		functions[i] = shader->GetShaderCode(shaderManager);
		ExtractShaderParameters(functions[i], shaderNodeReturns[i], shaderNodeParams[i]);

		for (auto p : shaderNodeParams[i]) {
			if (p == eMaterialShaderParamType::UNKNOWN) { 
				throw std::runtime_error("Parameter of unknown type.");
			}
		}

		shaderNodes[i].SetNumInputs(shaderNodeParams[i].size());
	}

	// link nodes together
	for (const auto& link : m_links) {
		ShaderNode& source = shaderNodes[link.sourceNode];
		ShaderNode& sink = shaderNodes[link.sinkNode];
		if (sink.GetNumInputs() <= link.sinkPort) {
			throw std::runtime_error("Invalid link: port does not have that many inputs.");
		}
		bool isLinked = source.GetOutput(0)->Link(sink.GetInput(link.sinkPort));
		if (!isLinked) {
			throw std::runtime_error("Invalid link: duplicate input to a single port.");
		}
	}

	// create output port LUT
	std::unordered_map<exc::OutputPortBase*, size_t> outputLut;
	for (size_t i = 0; i < shaderNodes.size(); ++i) {
		outputLut.insert({ shaderNodes[i].GetOutput(0), i });
	}

	// get the sink node
	size_t sinkNodeIdx = -1;
	for (size_t i = 0; i < shaderNodes.size(); ++i) {
		if (shaderNodes[i].GetOutput(0)->begin() == shaderNodes[i].GetOutput(0)->end()) {
			if (sinkNodeIdx != -1) {
				throw std::runtime_error("Invalid graph: multiple sink nodes.");
			}
			sinkNodeIdx = i;
		}
	}
	if (sinkNodeIdx == -1) {
		throw std::runtime_error("Invalid graph: no sink nodes, contains circle.");
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
				ss << "param" << freeParams.size();
				static_cast<exc::InputPort<std::string>*>(input)->Set(ss.str());
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
	exc::InputPort<std::string> finalCodePort;
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
		finalCode << GetParameterString(shaderNodeParams[p.node][p.input]) << " ";
		finalCode << p.name;
		firstParam = false;
	}
	finalCode << ") {\n";
	finalCode << "\n";
	// preambles
	for (auto idx : topologicalOrder) {
		finalCode << shaderNodes[idx].GetPreamble() << "\n";
	}
	finalCode << "\n";
	// return statement
	finalCode << "return " << finalCodePort.Get() << ";\n";
	finalCode << "}\n";
	

	return finalCode.str();

}

void MaterialShaderGraph::SetGraph(std::vector<std::unique_ptr<MaterialShader>> nodes, std::vector<Link> links) {
	m_nodes = std::move(nodes);
	m_links = std::move(links);
}


//------------------------------------------------------------------------------
// ShaderNode
//------------------------------------------------------------------------------

size_t MaterialShaderGraph::ShaderNode::GetNumInputs() const {
	return m_inputs.size();
}

exc::InputPortBase* MaterialShaderGraph::ShaderNode::GetInput(size_t index) {
	return &m_inputs[index];
}

const exc::InputPortBase* MaterialShaderGraph::ShaderNode::GetInput(size_t index) const {
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

void MaterialShaderGraph::ShaderNode::Notify(exc::InputPortBase* sender) {
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



} // namespace inl::gxeng