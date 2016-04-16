#pragma once

#include "Node.hpp"

#include <unordered_map>
#include <functional>
#include <type_traits>
#include <string>
#include <typeinfo>
#include <typeindex>


namespace exc {

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
		std::string group;
	};
private:
	/// <summary> A helper struct to instantiate a specific node type. </summary>
	struct NodeCreator {
		NodeCreator() = default;
		NodeCreator(const NodeCreator&) = default;
		NodeCreator(NodeCreator&&) = default;
		NodeInfo info;
		NodeBase* Create() {
			return creator ? creator() : nullptr;
		}
		std::function<NodeBase*()> creator;
	};
	using RegistryMapT = std::unordered_map<std::string, NodeCreator>;
public:
	NodeFactory() = default;
	NodeFactory(const NodeFactory&) = delete;

	/// <summary>
	/// Register a node class. Registered classes can be instantiated later.
	/// </summary>
	template <class T>
	bool RegisterNodeClass(const std::string& group);

	/// <summary> Instantiate node class by name. </summary>
	virtual NodeBase* CreateNode(const std::string& name);

	/// <summary> Get information about a node by name. </summary>
	const NodeInfo* GetNodeInfo(const std::string& name);

	/// <summary> Get list of all registered nodes. </summary>
	std::vector<NodeInfo> EnumerateNodes();

	/// <summary> Get singleton instance. </summary>
	static NodeFactory* GetInstance();
private:
	RegistryMapT registeredClasses; // Maps group/name entries to creators.
};



template <class T>
bool NodeFactory::RegisterNodeClass(const std::string& group) {
	static_assert(std::is_base_of<NodeBase, T>::value, "Registered class does not inherit from NodeBase.");

	// concat name and group as a path
	std::string strGroup = group;
	std::string strName = T::Info_GetName();
	if (strName.find('/') != std::string::npos) {
		return false;
	}
	auto descriptionBegins = strName.find(':');
	if (descriptionBegins != std::string::npos) {
		strName = strName.substr(0, descriptionBegins);
	}
	while (strGroup.size() > 0 && strGroup[strGroup.size() - 1] == '/') {
		strGroup.pop_back();
	}

	// check already registered
	auto it = registeredClasses.find(strName);

	if (it != registeredClasses.end()) {
		return false;
	}

	// set up node information for the class
	NodeCreator creator;
	creator.info.name = T::Info_GetName();
	creator.info.group = strGroup;
	creator.info.numInputPorts = T::Info_GetNumInputs();
	creator.info.numOutputPorts = T::Info_GetNumOutputs();
	creator.info.inputTypes = T::Info_GetInputTypes();
	creator.info.outputTypes = T::Info_GetOutputTypes();
	creator.info.inputNames = T::Info_GetInputNames();
	creator.info.outputNames = T::Info_GetOutputNames();
	creator.creator = []() -> NodeBase* {return new T();};

	// insert class to registered classes' map
	registeredClasses.insert({ strGroup + "/" + strName, std::move(creator) });

	return true;
}





// Statics don't fucking work when you have even static libraries.
// That fucking linker does not include unreferenced symbols, so 
// static member ctors are never called, thus classes are never registered.
// Can be solved by forcing the linker to include all symbols, but
// that's not portable, not efficient, and ugly as fuck. Anyway, use
// /OPT:NOREF on msvc and -Wl --whole-library on gcc.
//
// Switched to explicit registration, however, don't delete this code.


//template <class T, const char* Group>
//class AutoRegisterNode {
//	class Registerer {
//	public:
//		Registerer() {
//			NodeFactory::GetInstance()->RegisterNodeClass<T>(Group);
//		}
//	};
//public:
//	AutoRegisterNode() { &registerer; }
//private:
//	static Registerer registerer;
//};
//
//template <class T, const char* Group>
//typename AutoRegisterNode<T, Group>::Registerer AutoRegisterNode<T, Group>::registerer;



} // namespace exc