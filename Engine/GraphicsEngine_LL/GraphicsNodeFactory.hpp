#pragma once


#include "../BaseLibrary/Graph/NodeFactory.hpp"


namespace inl {
namespace gxeng {


class GraphicsEngine;


class GraphicsNodeFactory : public NodeFactory {
public:
	virtual NodeBase* CreateNode(const std::string& name) override;

private:
	GraphicsEngine* m_engine;
};


}
}

