#pragma once

#include "../GraphicsNode.hpp"

#include "../ResourceView.hpp"
#include "../MemoryManager.hpp"
#include "../ResouceViewFactory.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {


class GetDepthBuffer :
	virtual public GraphicsNode,
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<DepthStencilView>
{
public:
	GetDepthBuffer(MemoryManager* memgr, ResourceViewFactory* resViewFactory, unsigned width, unsigned height, size_t bufferCount);

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override;

protected:
	size_t currBuffer;
	std::vector<DepthStencilView> m_dsvs;
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
