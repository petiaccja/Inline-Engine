#include "NodeFactory.hpp"

#include <new>

namespace exc {

NodeBase* NodeFactory::CreateNode(const std::string& name) {
	auto it = registeredClasses.find(name);
	if (it == registeredClasses.end()) {
		return nullptr;
	}
	else {
		return it->second.Create();
	}
}

const NodeFactory::NodeInfo* NodeFactory::GetNodeInfo(const std::string& name) {
	auto it = registeredClasses.find(name);
	if (it == registeredClasses.end()) {
		return nullptr;
	}
	else {
		return &it->second.info;
	}
}

NodeFactory* NodeFactory::GetInstance() {
	static NodeFactory instance;
	return &instance;
}


auto NodeFactory::EnumerateNodes() -> std::vector<NodeInfo> {
	std::vector<NodeInfo> classes;
	classes.reserve(registeredClasses.size());
	for (auto& record : registeredClasses) {
		classes.push_back(record.second.info);
	}
	return classes;
}


} // namespace exc