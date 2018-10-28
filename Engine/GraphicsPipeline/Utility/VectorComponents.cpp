#pragma once

#include <GraphicsEngine_LL//GraphicsNode.hpp>
#include <InlineMath.hpp>

namespace inl::gxeng::nodes {


namespace impl {
class VectorComponentBase :
	virtual public GraphicsNode,
	public GraphicsTask
{
public:
	void Update() override {}

	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {
		GetInput(0)->Clear();
	}

	void Execute(RenderContext& context) override {}
};
}



/// <summary>
/// Gets the separate components in a vector.
/// Input: the vector
/// Outputs: the components of the vector in the order of storage (x, y, z, w)
/// </summary>
template<int ComponentCount>
class VectorComponents {
public:
	static_assert(ComponentCount > 0 && ComponentCount <= 4, "invalid number of components");
};



template<>
class VectorComponents<1> :
	public impl::VectorComponentBase,
	public InputPortConfig<Vector<float, 1>>,
	public OutputPortConfig<float>
{
public:
	static const char* Info_GetName() { return "Vector1Components"; }
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
	}
};


template<>
class VectorComponents<2> :
	public impl::VectorComponentBase,
	public InputPortConfig<Vec2>,
	public OutputPortConfig<float, float>
{
public:
	static const char* Info_GetName() { return "Vector2Components"; }
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
		GetOutput<1>().Set(vector[1]);
	}
};


template<>
class VectorComponents<3> :
	public impl::VectorComponentBase,
	public InputPortConfig<Vec3>,
	public OutputPortConfig<float, float, float>
{
public:
	static const char* Info_GetName() { return "Vector3Components"; }
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
		GetOutput<1>().Set(vector[1]);
		GetOutput<2>().Set(vector[2]);
	}
};


template<>
class VectorComponents<4> :
	public impl::VectorComponentBase,
	public InputPortConfig<Vec4>,
	public OutputPortConfig<float, float, float, float>
{
public:
	static const char* Info_GetName() { return "Vector4Components"; }
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
		GetOutput<1>().Set(vector[1]);
		GetOutput<2>().Set(vector[2]);
		GetOutput<3>().Set(vector[3]);
	}
};

} // namespace inl::gxeng::nodes
