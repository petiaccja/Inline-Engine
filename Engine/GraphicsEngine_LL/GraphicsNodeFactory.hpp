#pragma once


#include <BaseLibrary/Graph/NodeFactory.hpp>
#include <BaseLibrary/Singleton.hpp>

namespace inl::gxeng {


class GraphicsEngine;


class GraphicsNodeFactory : public NodeFactory {
public:
	NodeBase* CreateNode(const std::string& name) const override;
	std::vector<NodeInfo> EnumerateNodes() const override;

private:
	GraphicsEngine* m_engine = nullptr;
};


using GraphicsNodeFactory_Singleton = Singleton<GraphicsNodeFactory>;


} // namespace inl::gxeng
