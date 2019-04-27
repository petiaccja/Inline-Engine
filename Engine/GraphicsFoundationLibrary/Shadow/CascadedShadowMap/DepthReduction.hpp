#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class DepthReduction : virtual public GraphicsNode,
					   virtual public GraphicsTask,
					   virtual public InputPortConfig<Texture2D>,
					   virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "DepthReduction"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	DepthReduction();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}
	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	TextureView2D m_depthView;

	uint64_t m_width;
	uint32_t m_height;

	gxeng::RWTextureView2D m_uav;
	gxeng::TextureView2D m_srv;

protected:
	Binder m_binder;
	BindParameter m_depthBindParam;
	BindParameter m_outputBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
