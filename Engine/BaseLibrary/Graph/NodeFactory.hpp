#pragma once

#include "Node.hpp"
#include "../Singleton.hpp"
#include "../StringUtil.hpp"

#include <unordered_map>
#include <functional>
#include <type_traits>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <cassert>


namespace inl {

class NodeFactory {
public:
	/// <summary> Contains description of a Node class, such as
	/// port count, port names, node name and the like. </summary>
	struct NodeInfo {
		size_t numInputPorts;
		size_t numOutputPorts;
		std::vector<std::string> inputNames;
		std::vector<std::string> outputNames;
		std::vector<std::type_index> inputTypes;
		std::vector<std::type_index> outputTypes;
		std::string name;
		std::string description;
		std::string group;
	};
private:
	/// <summary> A helper struct to instantiate a specific node type. </summary>
	struct NodeCreator {
		NodeCreator() = default;
		NodeCreator(const NodeCreator&) = default;
		NodeCreator(NodeCreator&&) = default;
		NodeInfo info;
		NodeBase* Create() const {
			return creator ? creator() : nullptr;
		}
		std::function<NodeBase*()> creator;
	};
	using RegistryMapT = std::unordered_map<std::string, NodeCreator>;
public:
	NodeFactory() = default;
	NodeFactory(const NodeFactory&) = delete;
	virtual ~NodeFactory() = default;

	/// <summary>
	/// Register a node class. Registered classes can be instantiated later.
	/// </summary>
	template <class T>
	bool RegisterNodeClass(const std::string& group = "");

	/// <summary> Instantiate node class by name. </summary>
	virtual NodeBase* CreateNode(const std::string& name) const;

	/// <summary> Get information about a node by name. </summary>
	const NodeInfo* GetNodeInfo(const std::string& name) const;

	/// <summary> Get list of all registered nodes. </summary>
	virtual std::vector<NodeInfo> EnumerateNodes() const;

	/// <summary> Get path and class name for given node type. </summary>
	std::tuple<std::string, std::string> GetFullName(std::type_index classType) const;

	/// <summary> Get singleton instance. </summary>
	static NodeFactory* GetInstance();
private:
	RegistryMapT registeredClasses; // Maps group/name entries to creators.
	std::unordered_map<std::type_index, std::tuple<std::string, std::string>> typeLookup;
};



template <class T>
bool NodeFactory::RegisterNodeClass(const std::string& group) {
	static_assert(std::is_base_of<NodeBase, T>::value, "Registered class does not inherit from NodeBase.");

	// Cut trailing slash from group name.
	std::string strGroup = group;
	while (strGroup.size() > 0 && strGroup[strGroup.size() - 1] == '/') {
		strGroup.pop_back();
	}

	// Split name to name and description.
	std::string nameDesc = T::Info_GetName();
	auto tokens = Tokenize(nameDesc, ":", false);
	if (tokens.size() == 0) {
		throw InvalidArgumentException("Node's name cannot be empty.");
	}

	std::string strName(tokens[0].begin(), tokens[0].end());
	if (strName.find('/') != std::string::npos) {
		assert(false); // Node's name cannot contain slashes
		return false;
	}

	std::string strDesc;
	if (tokens.size() >= 2) {
		strDesc = std::string(tokens[1].begin(), tokens[1].end());
	}

	// check already registered
	auto it = registeredClasses.find(strName);

	if (it != registeredClasses.end()) {
		return false;
	}

	// set up node information for the class
	NodeCreator creator;
	creator.info.name = strName;
	creator.info.description = strDesc;
	creator.info.group = strGroup;
	creator.creator = []() -> NodeBase* {return new T();};

	// insert class to registered classes' map
	registeredClasses.insert({ strGroup + (strGroup.size() > 0 ? "/" : "") + strName, std::move(creator) });
	typeLookup.insert({ typeid(T), { strGroup, strName } });

	return true;
}


using NodeFactory_Singleton = Singleton<NodeFactory>;



#define INL_REGISTER_NODE(ClassName) \
const int __declspec(dllexport) s_registrar_##ClassName = [] { \
	std::cout << ClassName::Info_GetName() << std::endl; \
	return true; \
}();


} // namespace inl
