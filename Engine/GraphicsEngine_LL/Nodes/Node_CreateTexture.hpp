#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../PipelineTypes.hpp"

#include <cmath>


namespace inl::gxeng::nodes {


/// <summary>
/// Creates a texture with the given parameters.
/// Inputs: width, height, format, texture array element count.
/// Output: a new texture.
/// </summary>
/// <remarks>
/// A general philosophy of the pipeline is that each node that renders to texture
/// should take the render target(s) as its input.
/// This node's sole pupose is to create render target textures for the pipeline.
/// </remarks>
class CreateTexture :
	virtual public GraphicsNode,
	public GraphicsTask,
	public exc::InputPortConfig<unsigned, unsigned, gxapi::eFormat, uint16_t>,
	public exc::OutputPortConfig<gxeng::Texture2D>
{
public:
	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}

	void Setup(SetupContext& context) override {
		unsigned width = GetInput<0>().Get();
		unsigned height = GetInput<1>().Get();
		gxapi::eFormat format = GetInput<2>().Get();
		uint16_t arrayCount = GetInput<3>().Get();

		gxeng::Texture2D texture = context.CreateTexture2D(width, height, format, arrayCount);

		GetOutput<0>().Set(texture);
	}

	void Execute(RenderContext& context) override {}
};


} // namespace inl::gxeng::nodes
