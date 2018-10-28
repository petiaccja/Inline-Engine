#pragma once

#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <InlineMath.hpp>

namespace inl::gxeng::nodes {

/// <summary>
/// Gets the values in pixel coordinates, and creates a transform that can be applied to a full screen quad in NDC to achieve the required transformation
/// Inputs: Screen width, Screen height, Position, Rotation, Size
/// Output: Blend Destination
/// </summary>
class ScreenSpaceTransform :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<unsigned, unsigned, Vec2, float, Vec2>,
	virtual public OutputPortConfig<Mat44>
{
public:
	static const char* Info_GetName() { return "ScreenSpaceTransform"; }
	virtual void Update() override {}

	virtual void Notify(InputPortBase* sender) override {}

	virtual void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {
		GetInput(0)->Clear();
	}

	void Setup(SetupContext& context) override {
		unsigned width = GetInput<0>().Get();
		unsigned height = GetInput<1>().Get();
		Vec2 pos = GetInput<2>().Get();
		float rot = GetInput<3>().Get();
		Vec2 size = GetInput<4>().Get();

		// Move fsq so that its top left corner is on the origo
		Mat44 result = Mat44::Translation(Vec3(1.f, -1.f, 0.f));

		const float scaleX = size.x / width;
		const float scaleY = size.y / height;
		result = Mat44::Scale(Vec3(scaleX, scaleY, 1.f)) * result;
		result = Mat44::RotationZ(rot) * result;

		const float posX = (pos.x / width)*2.f - 1.f;
		const float posY = (-pos.y / height)*2.f + 1.f;
		result = Mat44::Translation(Vec3(posX, posY, 0.f)) * result;

		GetOutput<0>().Set(result);
	}

	void Execute(RenderContext& context) override {}
};

}
