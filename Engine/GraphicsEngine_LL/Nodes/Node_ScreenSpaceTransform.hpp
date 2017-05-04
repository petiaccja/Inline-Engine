#pragma once

#include "../GraphicsNode.hpp"

//#include "GraphicsApi_LL/IPipelineState.hpp"
//#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <mathfu/mathfu_exc.hpp>

namespace inl::gxeng::nodes {

/// <summary>
/// Gets the values in pixel coordinates, and creates a transform that can be applied to a full screen quad in NDC to achieve the required transformation
/// Inputs: Screen width, Screen height, Position, Rotation, Size
/// Output: Blend Destination
/// </summary>
class ScreenSpaceTransform :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<unsigned, unsigned, mathfu::Vector2f, float, mathfu::Vector2f>,
	virtual public exc::OutputPortConfig<mathfu::Matrix4x4f>
{
public:
	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {
		GetInput(0)->Clear();
	}

	void Setup(SetupContext& context) override {
		unsigned width = GetInput<0>().Get();
		unsigned height = GetInput<1>().Get();
		mathfu::Vector2f pos = GetInput<2>().Get();
		float rot = GetInput<3>().Get();
		mathfu::Vector2f size = GetInput<4>().Get();

		// Move fsq so that its top left corner is on the origo
		mathfu::Matrix4x4f result = mathfu::Matrix4x4f::FromTranslationVector(mathfu::Vector3f(1.f, -1.f, 0.f));

		const float scaleX = size.x() / width;
		const float scaleY = size.y() / height;
		result = mathfu::Matrix4x4f::FromScaleVector(mathfu::Vector3f(scaleX, scaleY, 1.f)) * result;
		result = mathfu::Matrix4x4f::FromRotationMatrix(mathfu::Matrix4x4f::RotationZ(rot)) * result;

		const float posX = (pos.x() / width)*2.f - 1.f;
		const float posY = (-pos.y() / height)*2.f + 1.f;
		result = mathfu::Matrix4x4f::FromTranslationVector(mathfu::Vector3f(posX, posY, 0.f)) * result;

		GetOutput<0>().Set(result);
	}

	void Execute(RenderContext& context) override {}
};

}
