#pragma once

#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class ClearDepthStencil : virtual public GraphicsNode,
						  virtual public GraphicsTask,
						  virtual public InputPortConfig<Texture2D, float, uint32_t, bool, bool>,
						  virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ClearDepthStencil"; }
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}
	void Reset() override {
		m_dsv = {};
		GetInput<0>().Clear();
	}
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:
	DepthStencilView2D m_dsv;
};


} // namespace inl::gxeng::nodes
