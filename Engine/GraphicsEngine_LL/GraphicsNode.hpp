#pragma once


#include "../BaseLibrary/Graph/Node.hpp"
#include "Task.hpp"



namespace inl {
namespace gxeng {


class GraphicsNode : virtual public exc::NodeBase {
public:
	virtual Task GetTask() = 0;
};


} // namespace gxeng
} // namespace inl