#pragma once

#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


/// <summary>
/// Obtain timing information about the graphics engine.
/// Outputs: absolute time since initializing the engine.
/// </summary>
class GetTime : virtual public GraphicsNode,
				public GraphicsTask,
				public InputPortConfig<>,
				public OutputPortConfig<double> {
public:
	static const char* Info_GetName() { return "GetTime"; }
	GetTime() {}

	void Update() override {}
	void Notify(InputPortBase* sender) override {}
	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {}

	void Setup(SetupContext& context) {
		this->GetOutput<0>().Set(m_absoluteTime);
	}

	void Execute(RenderContext& context) {}


	void SetAbsoluteTime(double absoluteTime) {
		m_absoluteTime = absoluteTime;
	}
	double GetAbsoluteTime() const {
		return m_absoluteTime;
	}

private:
	double m_absoluteTime;
};


} // namespace inl::gxeng
