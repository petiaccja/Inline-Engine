#pragma once

#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {

/// <summary>
/// Gets the parameters of a specific texture.
/// Input: the texture in question.
/// Outputs: width, height, format, texture array element count
/// </summary>
class TextureProperties : virtual public GraphicsNode,
						  public GraphicsTask,
						  public InputPortConfig<gxeng::Texture2D>,
						  public OutputPortConfig<unsigned, unsigned, gxapi::eFormat, uint16_t> {
public:
	static const char* Info_GetName() { return "TextureProperties"; }
	void Update() override {}

	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {
		GetInput(0)->Clear();
	}

	void Setup(SetupContext& context) override {
		const auto& texture = GetInput<0>().Get();

		GetOutput<0>().Set((unsigned)texture.GetWidth());
		GetOutput<1>().Set((unsigned)texture.GetHeight());
		GetOutput<2>().Set(texture.GetFormat());
		GetOutput<3>().Set(texture.GetArrayCount());
	}

	void Execute(RenderContext& context) override {}
};


} // namespace inl::gxeng::nodes
