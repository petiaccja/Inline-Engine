#include "NodeFactory.hpp"

#include <new>

namespace inl {

NodeBase* NodeFactory::CreateNode(const std::string& name) const {
	auto it = registeredClasses.find(name);
	if (it == registeredClasses.end()) {
		throw InvalidArgumentException("Requested node was not found.", name);
	}
	else {
		return it->second.Create();
	}
}

const NodeFactory::NodeInfo* NodeFactory::GetNodeInfo(const std::string& name) const {
	auto it = registeredClasses.find(name);
	if (it == registeredClasses.end()) {
		return nullptr;
	}
	else {
		return &it->second.info;
	}
}


auto NodeFactory::EnumerateNodes() const -> std::vector<NodeInfo> {
	std::vector<NodeInfo> classes;
	classes.reserve(registeredClasses.size());
	for (auto& record : registeredClasses) {
		classes.push_back(record.second.info);
	}
	return classes;
}


std::tuple<std::string, std::string> NodeFactory::GetFullName(std::type_index classType) const {
	auto it = typeLookup.find(classType);
	if (it == typeLookup.end()) {
		throw OutOfRangeException("This type is not registered as a node.");
	}
	return it->second;
}


} // namespace inl
