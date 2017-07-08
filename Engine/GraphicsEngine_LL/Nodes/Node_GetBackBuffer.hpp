#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../PipelineTypes.hpp"

#include <cmath>


namespace inl::gxeng::nodes {


/// <summary>
/// Return the texture that identifies the current backbuffer.
/// </sumnmary>
class GetBackBuffer :
	virtual public GraphicsNode,
	public GraphicsTask,
	public InputPortConfig<>,
	public OutputPortConfig<Texture2D>
{
public:
	virtual void Update() override {}
	virtual void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {
		m_backBuffer = {};
	}

	void Setup(SetupContext& context) override {
		if (!m_backBuffer.HasObject()) {
			throw InvalidStateException("You forgot to set the backbuffer to this node.");
		}
		GetOutput<0>().Set(m_backBuffer);
	}

	void Execute(RenderContext& context) override {}


	void SetBuffer(Texture2D backBuffer) {
		m_backBuffer = std::move(backBuffer);
	}
	const Texture2D& GetBuffer() const {
		return m_backBuffer;
	}
private:
	Texture2D m_backBuffer;
};


} // namespace inl::gxeng::nodes
