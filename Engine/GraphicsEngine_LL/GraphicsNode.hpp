#pragma once


#include "../BaseLibrary/Graph/Node.hpp"
#include "Task.hpp"
#include "NodeContext.hpp"



namespace inl::gxeng {

class GraphicsContext;


class GraphicsNode : virtual public exc::NodeBase {
public:
	virtual void Initialize(GraphicsContext& context) = 0;
	virtual void Setup(SetupContext& context) = 0;
	virtual void Execute(RenderContext& context) = 0;
};



} // namespace inlgxeng::gxeng