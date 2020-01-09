#pragma once

#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class ClearColor : virtual public GraphicsNode,
				   virtual public GraphicsTask,
				   virtual public InputPortConfig<Texture2D, Vec4>,
				   virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ClearColor"; }
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}
	void Reset() override {
		m_rtv = {};
		GetInput<0>().Clear();
	}
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:
	RenderTargetView2D m_rtv;
};


} // namespace inl::gxeng::nodes
