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
	GetDepthBuffer(MemoryManager* memgr, ResourceViewFactory* resViewFactory, unsigned width, unsigned height);

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override;

	void Resize(unsigned width, unsigned height);
private:
	void Init(unsigned width, unsigned height);
protected:
	DepthStencilView m_dsv;
	MemoryManager* m_memoryManager;
	ResourceViewFactory* m_resourceViewFactory;
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
