#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../PipelineTypes.hpp"

#include <mathfu/mathfu_exc.hpp>
#include <cmath>


namespace inl::gxeng::nodes {


namespace impl {
class VectorComponentBase :
	virtual public GraphicsNode,
	public GraphicsTask
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

	void Execute(RenderContext& context) override {}
};
}



template<int ComponentCount>
/// <summary>
/// Gets the separate components in a vector.
/// Input: the vector
/// Outputs: the components of the vector in the order of storage (x, y, z, w)
/// </summary>
class VectorComponents {
public:
	static_assert(ComponentCount > 0 && ComponentCount <= 4, "invalid number of components");
};



template<>
class VectorComponents<1> :
	public impl::VectorComponentBase,
	public exc::InputPortConfig<mathfu::Vector<float, 1>>,
	public exc::OutputPortConfig<float>
{
public:
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
	}
};


template<>
class VectorComponents<2> :
	public impl::VectorComponentBase,
	public exc::InputPortConfig<mathfu::Vector<float, 2>>,
	public exc::OutputPortConfig<float, float>
{
public:
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
		GetOutput<1>().Set(vector[1]);
	}
};


template<>
class VectorComponents<3> :
	public impl::VectorComponentBase,
	public exc::InputPortConfig<mathfu::Vector<float, 3>>,
	public exc::OutputPortConfig<float, float, float>
{
public:
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
	public exc::InputPortConfig<mathfu::Vector<float, 4>>,
	public exc::OutputPortConfig<float, float, float, float>
{
public:
	void Setup(SetupContext& context) override {
		const auto& vector = GetInput<0>().Get();

		GetOutput<0>().Set(vector[0]);
		GetOutput<1>().Set(vector[1]);
		GetOutput<2>().Set(vector[2]);
		GetOutput<3>().Set(vector[3]);
	}
};

} // namespace inl::gxeng::nodes
