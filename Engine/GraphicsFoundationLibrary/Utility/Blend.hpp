#pragma once


#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng {


/// <summary>
/// Inputs: Blend Destination, Blend source, BlendMode
/// Output: Blend Destination
/// </summary>
class Blend :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, gxapi::RenderTargetBlendState>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "Blend"; }
	void Update() override {}
	void Notify(InputPortBase* sender) override {}
	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	gxapi::RenderTargetBlendState m_blendMode;

	Binder m_binder;
	BindParameter m_tex0Param;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_renderTargetFormat = gxapi::eFormat::UNKNOWN;

private: // excute context
	RenderTargetView2D m_blendDest;
	TextureView2D m_blendSrc;
};


} // namespace inl::gxeng::nodes
