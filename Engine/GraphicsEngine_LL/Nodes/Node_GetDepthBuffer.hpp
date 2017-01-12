#pragma once

#include "../GraphicsNode.hpp"

#include "../ResourceView.hpp"
#include "../MemoryManager.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {


class GetDepthBuffer :
	virtual public GraphicsNode,
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<DepthStencilView2D>
{
public:
	GetDepthBuffer(MemoryManager* memgr, DSVHeap& dsvHeap, unsigned width, unsigned height);

	virtual void Update() override {}
	virtual void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext&) override {}

	virtual Task GetTask() override;

	void Resize(unsigned width, unsigned height);
private:
	void Init(unsigned width, unsigned height);
protected:
	DepthStencilView2D m_dsv;
	MemoryManager* m_memoryManager;
	DSVHeap& m_dsvHeap;
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
