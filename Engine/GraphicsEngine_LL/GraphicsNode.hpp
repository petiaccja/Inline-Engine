#pragma once


#include "../BaseLibrary/Graph/Node.hpp"
#include "Task.hpp"



namespace inl {
namespace gxeng {

class GraphicsContext;


class GraphicsNode : virtual public exc::NodeBase {
public:
	virtual void InitGraphics(const GraphicsContext& context) = 0;
	virtual Task GetTask() = 0;
};


} // namespace gxeng
} // namespace inl